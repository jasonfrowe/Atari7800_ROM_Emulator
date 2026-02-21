#!/usr/bin/env python3
# Add 7800 signature to ROM

import re

# Read the current ROM header
with open('include/game_rom.h', 'r') as f:
    content = f.read()

# Extract the data array
data_match = re.search(r'const uint8_t ROM_DATA\[49152\] = \{([^}]+)\}', content, re.DOTALL)
if data_match:
    hex_values = re.findall(r'0x[0-9A-Fa-f]{2}', data_match.group(1))
    rom = [int(v, 16) for v in hex_values]
    
    # Add signature at $FF7C (offset 48,252)
    signature = b'ATARI7800'
    offset = 0xFF7C - 0x4000
    
    print(f"Patching signature at offset {offset} (address $FF7C)")
    for i, byte in enumerate(signature):
        rom[offset + i] = byte
    
    # Write new ROM
    with open('include/game_rom.h', 'w') as f:
        f.write('#ifndef GAME_ROM_H\n')
        f.write('#define GAME_ROM_H\n\n')
        f.write('// Game: Astro Wing Startfighter\n')
        f.write(f'const uint32_t ROM_SIZE = {len(rom)};\n\n')
        f.write(f'const uint8_t ROM_DATA[{len(rom)}] = {{\n')
        
        for i in range(0, len(rom), 16):
            line = '    ' + ', '.join(f'0x{rom[j]:02X}' for j in range(i, min(i+16, len(rom))))
            if i + 16 < len(rom):
                line += ','
            f.write(line + '\n')
        
        f.write('};\n\n')
        f.write('#endif\n')
    
    print("✓ Signature added!")
    print("  Rebuild and upload the firmware.")
