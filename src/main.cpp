#include <Arduino.h>
#include "game_rom.h"

// ============================================================================
// ATARI 7800 ROM EMULATOR (48K) - 816MHz OVERCLOCK SYNC
// ============================================================================

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

// Calibrated for 64kHz @ 816MHz: 816,000,000 / (64,000 * 9 steps) = 1417 cycles
#define CYCLES_PER_STEP 1417 
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
        // --- 1. PURE ROM BRANCH (Stop Illegal Instructions) ---
        // Absolutely NO logic before this. The Atari 7800 needs instant response.
        addr = readFull16BitAddress();
        
        if (addr >= 0x4000) {
            data = ROM_DATA[addr - 0x4000];
            
            if (!isDriving) {
                SET_BUS_DRIVE(data);
                isDriving = true;
            } else {
                *gpio6_dr = (*gpio6_dr & ~DATA_BUS_MASK) | ((uint32_t)data << 16);
            }
        } 
        // --- 2. LISTEN / MATH BRANCH (Safe Zone) ---
        else {
            if (isDriving) {
                SET_BUS_LISTEN();
                isDriving = false;
            }

            // Gated by HALT (Pin 5 / GPIO9 bit 8). HIGH = CPU Active.
            if (*gpio9_psr & (1 << 8)) {
                
                // --- CLOCK RECOVERY ---
                // We track time ONLY in the LISTEN branch.
                // It will catch up on any time missed during Maria DMA.
                uint32_t currentCycle = ARM_DWT_CYCCNT;
                while ((currentCycle - lastPokeyCycle) >= CYCLES_PER_STEP) {
                    pokeyDebt++; 
                    lastPokeyCycle += CYCLES_PER_STEP;
                }
                if (pokeyDebt > 2000) pokeyDebt = 2000;

                // --- POKEY SNIFFER ---
                if ((addr & 0xFFF0) == 0x0450) {
                    // Pin 3 (R/W) is LOW for Write.
                    if (!(*gpio9_psr & (1 << 5))) {
                        uint8_t busData = (*gpio6_psr >> 16) & 0xFF;
                        pokey.writeRegister(addr & 0x0F, busData);
                    }
                }

                // --- DISTRIBUTED MATH (1 step at a time) ---
                if (pokeyDebt > 0) {
                    if (pokey.tickStep()) {
                        uint32_t val = (pokey.getOutput() * 1500) >> 8;
                        FLEXPWM2_SM3VAL5 = val; 
                        FLEXPWM2_MCTRL |= FLEXPWM_MCTRL_LDOK(1<<3); 
                    }
                    pokeyDebt--;
                }
            }
        }
    }
}

