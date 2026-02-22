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
// Pins 10,12,11,13,6,9,32,8 are used for A0-A7
// Pins 22,23,20,21,38,39,26,27 are used for A8-A15

// --- CORRECTED DIRECT REGISTER MASKS (Ground Truth) ---
// GPIO9 Control Pins (Teensy 4.1)
// Pin 2 (OE)   is GPIO9_04. 
// Pin 33 (DIR) is GPIO9_07. (HIGH = Drive Atari, LOW = Listen)

// Safe Transition: Release Teensy -> Switch Buffer to Listen
#define SET_BUS_LISTEN() { \
    GPIO6_GDIR &= ~DATA_BUS_MASK; \
    asm volatile ("dsb" ::: "memory"); \
    GPIO9_DR &= ~(1<<7); \
    asm volatile ("dsb" ::: "memory"); \
}

// Safe Transition: Load Data -> Switch Buffer to Drive -> Enable Teensy Output
#define SET_BUS_DRIVE(data) { \
    GPIO6_DR = (GPIO6_DR & ~DATA_BUS_MASK) | ((uint32_t)(data) << 16); \
    GPIO9_DR |= (1<<7); \
    asm volatile ("dsb" ::: "memory"); \
    GPIO6_GDIR |= DATA_BUS_MASK; \
    asm volatile ("dsb" ::: "memory"); \
}

// Data Bus Output (GPIO6_16 to GPIO6_23)
#define DATA_BUS_MASK    (0xFF << 16) 

void setup() {
    // 1. Configure Hardware Registers
    pinMode(PIN_OE, OUTPUT);
    GPIO9_DR |= (1<<4); // Disable buffer initially (HIGH)
    
    pinMode(PIN_DIR, OUTPUT);
    SET_BUS_LISTEN(); // Start in safe LISTEN mode
    
    // 2. Lock Buffer Enabled
    GPIO9_DR &= ~(1<<4); // OE = LOW (Enabled)
    
    noInterrupts();
}

// ULTRA-FAST ATOMIC ADDRESS READ (~12ns)
__attribute__((always_inline)) 
inline uint16_t readFull16BitAddress() {
    uint32_t g6 = GPIO6_PSR;
    uint32_t g7 = GPIO7_PSR;
    
    // High Byte (A8-A15): GPIO6 Bits 24-31 (Verified Ground Truth)
    uint16_t high = (g6 >> 16) & 0xFF00;
    
    // Low Byte (A0-A7): GPIO7 mappings (Verified Ground Truth)
    // A0-A3 (Bits 0-3), A4-A6 (Bits 10-12), A7 (Bit 16)
    uint16_t low = (g7 & 0x0F) | ((g7 >> 6) & 0x70) | ((g7 >> 9) & 0x80);
    
    return high|low;
}

void FASTRUN loop() {
    uint16_t addr;
    uint8_t data;
    bool isDriving = false; // State-tracking for direction
    
    while (1) {
        // 1. Atomic Read
        addr = readFull16BitAddress();
        
        // 2. ROM PATH ($4000 - $FFFF)
        if (addr >= 0x4000) {
            data = ROM_DATA[addr - 0x4000];
            
            if (!isDriving) {
                SET_BUS_DRIVE(data);
                isDriving = true;
            } else {
                // Update data pins without full transition
                GPIO6_DR = (GPIO6_DR & ~DATA_BUS_MASK) | ((uint32_t)data << 16);
            }
        } 
        // 3. LISTEN HOME ($0000 - $3FFF) - Rule 3
        else {
            if (isDriving) {
                SET_BUS_LISTEN();
                isDriving = false;
            }
        }
    }
}