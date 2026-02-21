#!/usr/bin/env python3
# Check original .a78 file for signature

import sys

a78_file = sys.argv[1] if len(sys.argv) > 1 else "~/Software/Atari7800/AtariTrader/build/output/astrowing.a78"

# Expand home directory
import os
a78_file = os.path.expanduser(a78_file)

try:
    with open(a78_file, 'rb') as f:
        data = f.read()
    
    print(f"File: {a78_file}")
    print(f"Total size: {len(data)} bytes")
    print()
    
    # Skip 128-byte header
    rom = data[128:]
    print(f"ROM size (after header): {len(rom)} bytes")
    print()
    
    # Check for signature at expected location
    sig_offset = 0xFF7C - 0x4000  # 48,252
    if sig_offset + 9 <= len(rom):
        signature = rom[sig_offset:sig_offset+9]
        print(f"Data at ROM offset {sig_offset} (address $FF7C):")
        print(f"  ASCII: {signature.decode('ascii', errors='replace')}")
        print(f"  Hex:   {' '.join(f'{b:02X}' for b in signature)}")
        print()
        
        if signature == b'ATARI7800':
            print("✓ Signature FOUND in .a78 file!")
        else:
            print("✗ Signature MISSING from .a78 file!")
            print("  The ROM needs to be rebuilt with the 7800 signature.")
    
except FileNotFoundError:
    print(f"ERROR: File not found: {a78_file}")
    print("Usage: python3 check_a78.py <path-to-a78-file>")
