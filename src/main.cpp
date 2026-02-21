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

// --- DIRECT REGISTER MASKS ---
// GPIO9 Control Pins (Teensy 4.1)
// Pin 2 (OE) is GPIO9_04. 
#define BUS_DISABLE()    GPIO9_DR |= (1<<4)
#define BUS_ENABLE()     GPIO9_DR &= ~(1<<4)

// Data Bus Output (GPIO6_16 to GPIO6_23)
#define DATA_BUS_MASK    (0xFF << 16) 

// RAM Buffer (64K to map full address space easily)
uint8_t RAM_ROM[65536];

void setup() {
    // 1. Configure Control Pins
    pinMode(PIN_OE, OUTPUT); BUS_DISABLE();
    pinMode(PIN_DIR, OUTPUT); digitalWrite(PIN_DIR, HIGH);
    
    // 2. Address Pins: Default to INPUT on boot, no configuration needed
    
    // 3. Configure Data Bus (Output) - Direct register write
    // Data pins are GPIO6_16-23, set them as outputs in one operation
    GPIO6_GDIR |= DATA_BUS_MASK;

    // 4. Load ROM into RAM at Offset $4000
    // TESTING: Skip memcpy - read directly from Flash ROM_DATA
    // memset(RAM_ROM, 0xFF, 0x4000); 
    // memcpy(RAM_ROM + 0x4000, ROM_DATA, 49152); // Load 48KB
    
    noInterrupts();
}

// ULTRA-FAST ATOMIC ADDRESS READ (~10ns)
__attribute__((always_inline)) 
inline uint16_t readFull16BitAddress() {
    uint32_t g6 = GPIO6_PSR;
    uint32_t g7 = GPIO7_PSR;
    
    // High Byte (A8-A15): 
    // GPIO6 Bits 24-31. We shift right 16 to move them to Bits 8-15.
    uint16_t high = (g6 >> 16) & 0xFF00;
    
    // Low Byte (A0-A7):
    // A0-A3 (Pins 10,12,11,13) -> GPIO7 Bits 0-3  (No shift)
    // A4-A6 (Pins 6,9,32)      -> GPIO7 Bits 10-12 (Shift Right 6)
    // A7    (Pin 8)            -> GPIO7 Bit 16    (Shift Right 9)
    uint16_t low = (g7 & 0x0F) | ((g7 >> 6) & 0x70) | ((g7 >> 9) & 0x80);
    
    return high | low;
}

void FASTRUN loop() {
    uint16_t addr;
    uint32_t data;
    volatile uint32_t *gpio6_dr = &GPIO6_DR;
    
    while (1) {
        // 1. Atomic Read
        addr = readFull16BitAddress();
        
        // 2. Map Logic: Active for $4000 - $FFFF
        // This means either A15 (0x8000) or A14 (0x4000) is HIGH.
        if (addr & 0xC000) {
            
            // 3. Fetch - Direct from Flash (addr $4000-$FFFF maps to ROM_DATA[0-48K])
            data = ROM_DATA[addr - 0x4000];

            // 4. Drive (One Instruction)
            *gpio6_dr = (*gpio6_dr & ~DATA_BUS_MASK) | (data << 16);
            BUS_ENABLE();
            
        } else {
            BUS_DISABLE();
        }
    }
}