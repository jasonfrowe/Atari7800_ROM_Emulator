#!/usr/bin/env python3
# Check ROM data at captured addresses

import re

# Read the ROM data
with open('include/game_rom.h', 'r') as f:
    content = f.read()

# Extract just the data array
data_match = re.search(r'const uint8_t ROM_DATA\[49152\] = \{([^}]+)\}', content, re.DOTALL)
if data_match:
    # Parse the hex values
    hex_values = re.findall(r'0x[0-9A-Fa-f]{2}', data_match.group(1))
    rom = [int(v, 16) for v in hex_values]
    
    # Check the addresses from capture
    addresses = [0x83F8, 0x8BF8, 0x87F8, 0x8FF8, 0x93F8, 0x9BF8, 0x97F8]
    expected = [0x25, 0x42, 0x85, 0x86, 0xFF, 0xFF, 0xFF]
    
    print('Address -> ROM Offset -> Data (Expected)')
    print('=' * 50)
    for addr, exp in zip(addresses, expected):
        offset = addr - 0x4000
        if offset < len(rom):
            actual = rom[offset]
            match = '✓' if actual == exp else 'MISMATCH!'
            print(f'${addr:04X} -> ROM[{offset:5d}] = 0x{actual:02X} (expected 0x{exp:02X}) {match}')
        else:
            print(f'${addr:04X} -> OUT OF RANGE')
    
    print(f'\nROM size: {len(rom)} bytes')
    print(f'Address range: $4000-${0x4000 + len(rom) - 1:04X}')
