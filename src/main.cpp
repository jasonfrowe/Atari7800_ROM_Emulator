#include <Arduino.h>
#include "game_rom.h"

// ============================================================================
// ATARI 7800 ROM EMULATOR FOR 2600+ (48K ROM) - PURE ADDRESS LOGIC
// ============================================================================

// --- SKIP STARTUP DELAYS ---
extern "C" void startup_middle_hook(void);
extern "C" volatile uint32_t systick_millis_count;

void startup_middle_hook(void) {
  systick_millis_count = 300;
}

// --- PIN DEFINITIONS ---
const int PIN_OE   = 2;
const int PIN_DIR  = 33;
const int PIN_AUDIO = 37;

// --- DIRECTION MACROS ---
#define DATA_BUS_MASK (0xFF << 16) 

#define SET_BUS_LISTEN() { \
    GPIO6_GDIR &= ~DATA_BUS_MASK; \
    asm volatile ("dsb" ::: "memory"); \
    GPIO9_DR &= ~(1<<7); \
    asm volatile ("dsb" ::: "memory"); \
}

#define SET_BUS_DRIVE(data) { \
    GPIO6_DR = (GPIO6_DR & ~DATA_BUS_MASK) | ((uint32_t)(data) << 16); \
    GPIO9_DR |= (1<<7); \
    asm volatile ("dsb" ::: "memory"); \
    GPIO6_GDIR |= DATA_BUS_MASK; \
    asm volatile ("dsb" ::: "memory"); \
}

// --- POKEY EMULATION ---
#include "PokeyWrapper.h"
PokeyWrapper pokey;

#define CYCLES_PER_TICK 335 
uint32_t lastPokeyCycle = 0;
uint32_t pokeyDebt = 0;

void setup() {
    pinMode(PIN_OE, OUTPUT);
    GPIO9_DR |= (1<<4); // Disable buffer initially (HIGH)
    
    pinMode(PIN_DIR, OUTPUT);
    SET_BUS_LISTEN(); // Start in safe LISTEN mode
    
    GPIO9_DR &= ~(1<<4); // OE = LOW (Enabled)
    
    analogWrite(PIN_AUDIO, 1); 
    analogWriteFrequency(PIN_AUDIO, 375000); 
    analogWriteResolution(8);

    pokey.begin();
    lastPokeyCycle = ARM_DWT_CYCCNT;

    noInterrupts();
}

__attribute__((always_inline)) 
inline uint16_t readFull16BitAddress() {
    uint32_t g6 = GPIO6_PSR;
    uint32_t g7 = GPIO7_PSR;
    
    uint16_t high = (g6 >> 16) & 0xFF00;
    uint16_t low = (g7 & 0x0F) | ((g7 >> 6) & 0x70) | ((g7 >> 9) & 0x80);
    
    return high|low;
}

void FASTRUN loop() {
    uint16_t addr;
    uint8_t data;
    bool isDriving = false; 
    
    volatile uint32_t *gpio6_dr = &GPIO6_DR;
    volatile uint32_t *gpio6_psr = &GPIO6_PSR;
    volatile uint32_t *gpio9_psr = &GPIO9_PSR;

    while (1) {
        addr = readFull16BitAddress();
        
        // --- 1. CARTRIDGE SPACE (Drive ROM Data) ---
        if (addr >= 0x4000) {
            data = ROM_DATA[addr - 0x4000];
            
            if (!isDriving) {
                SET_BUS_DRIVE(data);
                isDriving = true;
            } else {
                *gpio6_dr = (*gpio6_dr & ~DATA_BUS_MASK) | ((uint32_t)data << 16);
            }
        } 
        // --- 2. LISTEN SPACE (Idle / Pokey) ---
        else {
            if (isDriving) {
                SET_BUS_LISTEN();
                isDriving = false;
            }

            // --- MARIA DMA PROTECTION ---
            // PIN 5 (HALT) is GPIO9 bit 8. 
            // HIGH = 6502 CPU is running. LOW = MARIA is doing DMA.
            if (*gpio9_psr & (1 << 8)) {
                
                // A. POKEY SPACE: Read Address, Read Data
                if ((addr & 0xFFF0) == 0x0450) {
                    uint8_t busData = (*gpio6_psr >> 16) & 0xFF;
                    pokey.writeRegister(addr & 0x0F, busData);
                }

                // B. POKEY MATH: Process Ticks
                uint32_t currentCycle = ARM_DWT_CYCCNT;
                if ((currentCycle - lastPokeyCycle) >= CYCLES_PER_TICK) {
                    pokeyDebt += 9; 
                    lastPokeyCycle += CYCLES_PER_TICK;
                }

                if (pokeyDebt > 0) {
                    pokey.tickStep();
                    pokeyDebt--;
                }

                // C. PWM REFRESH
                // We only update PWM when the CPU is running. The hardware 
                // PWM will automatically hold the last state during MARIA DMA!
                uint32_t sample = pokey.getOutput();
                uint32_t val = (sample * 400) >> 8;
                FLEXPWM2_SM3VAL5 = val; 
                FLEXPWM2_MCTRL |= FLEXPWM_MCTRL_LDOK(1<<3); 
            }
        }
    }
}