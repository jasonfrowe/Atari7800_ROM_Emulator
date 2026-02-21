#!/usr/bin/env python3
# Convert .bin to .h with signature added

import sys
import os

bin_file = sys.argv[1] if len(sys.argv) > 1 else "~/Software/Atari7800/AtariTrader/build/output/astrowing.bin"
out_file = sys.argv[2] if len(sys.argv) > 2 else "include/game_rom.h"

bin_file = os.path.expanduser(bin_file)

try:
    with open(bin_file, 'rb') as f:
        data = f.read()
    
    # Check if this is an .a78 file (has 128-byte header)
    if bin_file.endswith('.a78') and len(data) > 128:
        # Check for .a78 magic signature
        if data[0:2] == b'\x04A':  # 0x04 followed by 'A' (start of "ATARI7800")
            print(f"Detected .a78 format - stripping 128-byte header")
            rom = bytearray(data[128:])  # Skip header
        else:
            rom = bytearray(data)
    else:
        rom = bytearray(data)
    
    print(f"Input:  {bin_file}")
    print(f"Output: {out_file}")
    print(f"ROM size: {len(rom)} bytes")
    
    # Determine memory map based on ROM size
    if len(rom) == 49152:  # 48KB at $4000-$FFFF
        base_addr = 0x4000
        print(f"Memory map: ${base_addr:04X}-$FFFF (48KB)")
    elif len(rom) == 32768:  # 32KB at $8000-$FFFF
        base_addr = 0x8000
        print(f"Memory map: ${base_addr:04X}-$FFFF (32KB)")
    elif len(rom) == 16384:  # 16KB at $C000-$FFFF
        base_addr = 0xC000
        print(f"Memory map: ${base_addr:04X}-$FFFF (16KB)")
    else:
        print(f"ERROR: Unsupported ROM size: {len(rom)} bytes")
        sys.exit(1)
    
    # Add 7800 detection data
    # Control byte at $FF7B - MUST BE NON-ZERO for 7800 detection!
    # 0x00 = 2600 mode, any non-zero = 7800 mode
    # Retail carts use 0x01 (seen in Choplifter, Asteroids, etc.)
    ctrl_offset = 0xFF7B - base_addr
    rom[ctrl_offset] = 0x01  # Non-zero = 7800 mode
    print(f"\nAdding control byte 0x01 at offset {ctrl_offset} (address $FF7B - triggers 7800 mode)")
    
    # Signature at $FF7C (NOT used by real hardware, but keep for completeness)
    sig_offset = 0xFF7C - base_addr
    signature = b'ATARI7800'
    
    print(f"Adding signature at offset {sig_offset} (address $FF7C)")
    for i, byte in enumerate(signature):
        rom[sig_offset + i] = byte
    
    print("✓ Control byte and signature added!")
    
    # Write header file
    with open(out_file, 'w') as f:
        f.write('#ifndef GAME_ROM_H\n')
        f.write('#define GAME_ROM_H\n\n')
        f.write('// Game: Astro Wing Starfighter (with 7800 signature)\n')
        f.write(f'const uint32_t ROM_SIZE = {len(rom)};\n\n')
        f.write(f'const uint8_t ROM_DATA[{len(rom)}] = {{\n')
        
        for i in range(0, len(rom), 16):
            line = '    ' + ', '.join(f'0x{rom[j]:02X}' for j in range(i, min(i+16, len(rom))))
            if i + 16 < len(rom):
                line += ','
            f.write(line + '\n')
        
        f.write('};\n\n')
        f.write('#endif\n')
    
    print(f"\n✓ Converted to {out_file}")
    print("  Rebuild and upload the firmware.")
    
except FileNotFoundError:
    print(f"ERROR: File not found: {bin_file}")
    print("Usage: python3 convert_bin_with_sig.py <bin-file> [output.h]")
