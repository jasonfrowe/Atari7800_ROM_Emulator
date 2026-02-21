#ifndef ROM_LOADER_H
#define ROM_LOADER_H

#include <Arduino.h>

// ROM Configuration
#define ROM_SIZE_KB 48
#define ROM_SIZE_BYTES (ROM_SIZE_KB * 1024)
#define A78_HEADER_SIZE 128

// Atari 7800 48K ROM Memory Map: $4000-$FFFF
#define ROM_START_ADDR 0x4000
#define ROM_END_ADDR   0xFFFF

// External ROM data array (will be defined in rom_data.cpp)
extern const uint8_t ROM_DATA[ROM_SIZE_BYTES];

// Function to initialize ROM (for future SD card loading, etc.)
void initROM();

// Fast inline function to get ROM byte from address
inline uint8_t getROMByte(uint16_t address) {
    // For 48K cart: Maps $4000-$FFFF (48K = 0xC000 bytes)
    // Address range $4000-$FFFF = 49152 addresses
    // But we only have 48K (49152 bytes) of ROM
    
    if (address >= ROM_START_ADDR) {
        // Calculate offset into ROM array
        uint16_t offset = address - ROM_START_ADDR;
        
        // Make sure we're in range
        if (offset < ROM_SIZE_BYTES) {
            return ROM_DATA[offset];
        }
    }
    
    // For addresses below $4000, return 0xFF (open bus)
    // This is typical behavior for unmapped ROM space
    return 0xFF;
}

#endif // ROM_LOADER_H
