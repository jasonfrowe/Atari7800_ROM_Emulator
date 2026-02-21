#include <Arduino.h>
#include "game_rom.h"

// ============================================================================
// ATARI 7800 ROM EMULATOR FOR 2600+ (48K ROM) - FAST ADDRESSING
// ============================================================================
// WIRING: REWIRED for GPIO6/GPIO7 Alignment (See Chat History)
// LOGIC:  Atomic Address Read (Direct Register) -> Instant Drive
// MAP:    $4000-$FFFF (Active if A15 or A14 is High)
// ============================================================================

// --- SKIP STARTUP DELAYS (300ms total) ---
extern "C" void startup_middle_hook(void);
extern "C" volatile uint32_t systick_millis_count;

void startup_middle_hook(void) {
  // Force millis() to be 300 to skip startup delays
  systick_millis_count = 300;
}

// --- PIN DEFINITIONS (REWIRED) ---
const int PIN_OE   = 2;
const int PIN_RW   = 3;
const int PIN_PHI2 = 4;
const int PIN_HALT = 5;
const int PIN_DIR  = 33;
const int PIN_AUDIO = 37;
// Pins 10,12,11,13,6,9,32,8 are used for A0-A7
// Pins 22,23,20,21,38,39,26,27 are used for A8-A15

// --- DIRECT REGISTER MASKS ---
// GPIO9 Control Pins (Teensy 4.1)
// Pin 2 (OE) is GPIO9_04. 
#define BUS_DISABLE()    GPIO9_DR |= (1<<4)
#define BUS_ENABLE()     GPIO9_DR &= ~(1<<4)

// Data Bus Output (GPIO6_16 to GPIO6_23)
#define DATA_BUS_MASK    (0xFF << 16) 

// --- POKEY EMULATION ---
#include "PokeyWrapper.h"
PokeyWrapper pokey;

// 600MHz / 1.79MHz = ~335 CPU cycles per Atari clock tick
#define CYCLES_PER_TICK 335 
uint32_t lastPokeyCycle = 0;
uint32_t pokeyDebt = 0;

void setup() {
    // 1. Configure Control Pins
    pinMode(PIN_OE, OUTPUT); BUS_DISABLE();
    pinMode(PIN_DIR, OUTPUT); digitalWrite(PIN_DIR, HIGH);
    
    // 2. Configure Data Bus (Output)
    GPIO6_GDIR |= DATA_BUS_MASK;

    // 3. Configure Audio PWM (Pin 37)
    pinMode(PIN_AUDIO, OUTPUT);
    analogWriteFrequency(PIN_AUDIO, 375000); 
    analogWriteResolution(8);

    pokey.begin();
    lastPokeyCycle = ARM_DWT_CYCCNT;
    noInterrupts();
}

// ULTRA-FAST ATOMIC ADDRESS READ (~10ns)
__attribute__((always_inline)) 
inline uint16_t readFull16BitAddress() {
    uint32_t g6 = GPIO6_PSR;
    uint32_t g7 = GPIO7_PSR;
    uint16_t high = (g6 >> 16) & 0xFF00;
    uint16_t low = (g7 & 0x0F) | ((g7 >> 6) & 0x70) | ((g7 >> 9) & 0x80);
    return high | low;
}

void FASTRUN loop() {
    uint16_t addr, addr2;
    uint32_t data;
    bool lastWriteState = false;
    volatile uint32_t *gpio6_dr = &GPIO6_DR;
    volatile uint32_t *gpio6_psr = &GPIO6_PSR;
    volatile uint32_t *gpio9_psr = &GPIO9_PSR;

    while (1) {
        // --- 1. STABLE ADDRESS READ ---
        // We read twice. If they don't match, the bus is transitioning.
        addr = readFull16BitAddress();
        addr2 = readFull16BitAddress();
        if (addr != addr2) continue;
        
        // --- 2. THE ROM PATH (Absolute Priority) ---
        if (addr >= 0x4000) {
            data = ROM_DATA[addr - 0x4000];
            *gpio6_dr = (*gpio6_dr & ~DATA_BUS_MASK) | (data << 16);
            BUS_ENABLE();
            continue; 
        } 
        
        // --- 3. THE IDLE / IO PATH ---
        BUS_DISABLE(); 

        // B. MARIA DMA PROTECTION
        // HALT (Pin 5) is GPIO9_08. If LOW, Maria is doing DMA.
        // We must NOT do any POKEY work or sniffer logic during DMA.
        uint32_t g9 = *gpio9_psr;
        if (!(g9 & (1 << 8))) continue;

        // C. POKEY SNIFFER ($0450)
        // R/W (Pin 3) is GPIO9_05. LOW = Write.
        if ((addr & 0xFFF0) == 0x0450) {
            bool isWrite = !(g9 & (1 << 5)); 
            if (isWrite && !lastWriteState) {
                pokey.writeRegister(addr & 0x0F, (*gpio6_psr >> 16) & 0xFF);
            }
            lastWriteState = isWrite;
        } else {
            lastWriteState = false;
        }

        // D. ELASTIC POKEY (Safe Window)
        uint32_t currentCycle = ARM_DWT_CYCCNT;
        if ((currentCycle - lastPokeyCycle) >= CYCLES_PER_TICK) {
            pokeyDebt += 9; 
            lastPokeyCycle += CYCLES_PER_TICK;
        }

        if (pokeyDebt > 0) {
            pokey.tickStep();
            pokeyDebt--;
            
            // Update PWM registers
            uint32_t sample = pokey.getOutput();
            FLEXPWM2_SM3VAL5 = (sample * 400) >> 8; 
            FLEXPWM2_MCTRL |= FLEXPWM_MCTRL_LDOK(1<<3); 
        }
    }
}