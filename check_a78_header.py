#!/usr/bin/env python3
# Check .a78 header for 7800 detection info

import sys
import os

a78_file = sys.argv[1] if len(sys.argv) > 1 else "~/Software/Atari7800/AtariTrader/build/output/astrowing.a78"
a78_file = os.path.expanduser(a78_file)

with open(a78_file, 'rb') as f:
    header = f.read(128)
    rom = f.read()

print('=== .a78 FILE ANALYSIS ===')
print(f'File: {a78_file}')
print(f'Header size: 128 bytes')
print(f'ROM size: {len(rom)} bytes')
print()

print('=== HEADER FIELDS ===')
print('Bytes 0-15 (magic signature):')
sig = ''.join(chr(b) if 32 <= b < 127 else '.' for b in header[0:16])
print(f'  ASCII: "{sig}"')
print(f'  Hex: {" ".join(f"{b:02X}" for b in header[0:16])}')
print()

print(f'Byte 53 (cartridge type): 0x{header[53]:02X}')
cart_types = {
    0: 'Standard 7800',
    1: 'SuperGame (bank-switched)',
    2: 'SuperGame with RAM',
    3: 'Absolute (F18 Hornet)',
    4: 'Activision',
}
print(f'  Type: {cart_types.get(header[53], "Unknown")}')
print()

print(f'Byte 54 (controller 1): 0x{header[54]:02X}')
print(f'Byte 55 (controller 2): 0x{header[55]:02X}')
print()

print(f'Byte 58 (TV format): 0x{header[58]:02X}')
print(f'  Format: {"PAL" if header[58] == 1 else "NTSC"}')
print()

print(f'Byte 63 (save device): 0x{header[63]:02X}')
print()

print('Bytes 100-116 (cart name):')
name = ''.join(chr(b) if b != 0 else '' for b in header[100:117])
print(f'  "{name}"')
print()

print('=== FULL HEADER DUMP ===')
for i in range(0, 128, 16):
    hex_part = ' '.join(f'{header[i+j]:02X}' for j in range(16))
    asc_part = ''.join(chr(header[i+j]) if 32 <= header[i+j] < 127 else '.' for j in range(16))
    print(f'{i:04X}: {hex_part}  {asc_part}')

print()
print('=== ROM CHECK ===')
# Check if ROM has signature at $FF7C
sig_offset = 0xFF7C - 0x4000
if sig_offset + 9 <= len(rom):
    rom_sig = rom[sig_offset:sig_offset+9]
    sig_str = rom_sig.decode('ascii', errors='replace')
    print(f'Signature at ROM offset {sig_offset} ($FF7C): "{sig_str}"')
    if rom_sig == b'ATARI7800':
        print('  ✓ ROM has signature')
    else:
        print('  ✗ ROM missing signature')
        print(f'    Found: {" ".join(f"{b:02X}" for b in rom_sig)}')
