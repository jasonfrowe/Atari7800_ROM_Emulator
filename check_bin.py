#!/usr/bin/env python3
# Check .bin file for signature

import sys
import os

bin_file = sys.argv[1] if len(sys.argv) > 1 else "~/Software/Atari7800/AtariTrader/build/output/astrowing.bin"
bin_file = os.path.expanduser(bin_file)

try:
    with open(bin_file, 'rb') as f:
        rom = f.read()
    
    print(f"File: {bin_file}")
    print(f"Total size: {len(rom)} bytes ({len(rom)//1024}K)")
    print()
    
    # Check for signature at expected location for different ROM sizes
    rom_size = len(rom)
    
    # For 48K ROM starting at $4000, signature should be at offset $FF7C - $4000
    if rom_size == 49152:  # 48K
        sig_offset = 0xFF7C - 0x4000
        print(f"48K ROM detected (maps to $4000-$FFFF)")
    elif rom_size == 32768:  # 32K
        sig_offset = 0xFF7C - 0x8000
        print(f"32K ROM detected (maps to $8000-$FFFF)")
    elif rom_size == 16384:  # 16K
        sig_offset = 0xFF7C - 0xC000
        print(f"16K ROM detected (maps to $C000-$FFFF)")
    else:
        print(f"Unknown ROM size: {rom_size} bytes")
        sig_offset = rom_size - (0x10000 - 0xFF7C) - 1
    
    print(f"Expected signature at offset {sig_offset} (0x{sig_offset:04X})")
    print()
    
    if sig_offset >= 0 and sig_offset + 9 <= rom_size:
        signature = rom[sig_offset:sig_offset+9]
        print(f"Data at offset {sig_offset}:")
        print(f"  ASCII: {signature.decode('ascii', errors='replace')}")
        print(f"  Hex:   {' '.join(f'{b:02X}' for b in signature)}")
        print()
        
        if signature == b'ATARI7800':
            print("✓ Signature FOUND in .bin file!")
            print("  This ROM should work correctly.")
        else:
            print("✗ Signature MISSING from .bin file!")
            # Search for it
            print("\nSearching entire ROM for 'ATARI7800'...")
            pos = rom.find(b'ATARI7800')
            if pos >= 0:
                if rom_size == 49152:
                    addr = 0x4000 + pos
                elif rom_size == 32768:
                    addr = 0x8000 + pos
                elif rom_size == 16384:
                    addr = 0xC000 + pos
                else:
                    addr = pos
                print(f"  Found at offset {pos} (would map to address ${addr:04X})")
                print(f"  Expected at offset {sig_offset} (address $FF7C)")
            else:
                print("  NOT FOUND anywhere in ROM!")
    else:
        print(f"ERROR: Offset {sig_offset} is out of bounds!")
    
    # Also check reset vectors
    print("\n" + "="*60)
    print("Reset Vector Check:")
    reset_offset = rom_size - 4  # $FFFC-$FFFD are last 4 bytes
    if reset_offset >= 0:
        reset_vec = rom[reset_offset:reset_offset+2]
        reset_addr = reset_vec[0] | (reset_vec[1] << 8)
        print(f"  Reset vector at end of ROM: ${reset_addr:04X}")
        print(f"  Hex: {reset_vec[0]:02X} {reset_vec[1]:02X}")
        
except FileNotFoundError:
    print(f"ERROR: File not found: {bin_file}")
    print("Usage: python3 check_bin.py <path-to-bin-file>")
