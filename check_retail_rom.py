#!/usr/bin/env python3
# Check retail ROM for 7800 signature

import sys

rom_file = sys.argv[1] if len(sys.argv) > 1 else "Choplifter_NTSC.a78"

with open(rom_file, 'rb') as f:
    header = f.read(128)
    rom = f.read()

rom_size = len(rom)
print(f'=== RETAIL ROM ANALYSIS ===')
print(f'File: {rom_file}')
print(f'ROM size: {rom_size} bytes ({rom_size//1024}KB)')
print()

# Calculate offsets for different ROM sizes
if rom_size == 49152:  # 48KB at $4000
    sig_offset = 0xFF7C - 0x4000
    ctrl_offset = 0xFF7B - 0x4000
    base_addr = 0x4000
elif rom_size == 32768:  # 32KB at $8000
    sig_offset = 0xFF7C - 0x8000
    ctrl_offset = 0xFF7B - 0x8000
    base_addr = 0x8000
elif rom_size == 16384:  # 16KB at $C000
    sig_offset = 0xFF7C - 0xC000
    ctrl_offset = 0xFF7B - 0xC000
    base_addr = 0xC000
else:
    print(f'ERROR: Unknown ROM size: {rom_size}')
    sys.exit(1)

print(f'Memory map: ${base_addr:04X}-$FFFF')
print(f'Signature offset in ROM: {sig_offset} (0x{sig_offset:04X})')
print()

# Check control byte at $FF7B
ctrl = rom[ctrl_offset]
print(f'=== CONTROL BYTE at $FF7B ===')
print(f'Offset: {ctrl_offset} (0x{ctrl_offset:04X})')
print(f'Value: 0x{ctrl:02X} (binary: {ctrl:08b})')
print(f'  Bit 0 (TV): {"PAL" if ctrl & 1 else "NTSC"}')
print(f'  Bit 1 (Pokey): {"Yes @$4000" if ctrl & 2 else "No"}')
print(f'  Bit 2 (SuperGame): {"Yes" if ctrl & 4 else "No"}')
print()

# Check signature at $FF7C
sig_bytes = rom[sig_offset:sig_offset+9]
sig_str = ''.join(chr(b) if 32 <= b < 127 else '?' for b in sig_bytes)

print(f'=== SIGNATURE at $FF7C-$FF84 ===')
print(f'Offset: {sig_offset} (0x{sig_offset:04X})')
print(f'ASCII: "{sig_str}"')
print(f'Hex:   {" ".join(f"{b:02X}" for b in sig_bytes)}')
print()

if sig_bytes == b'ATARI7800':
    print('✅ VALID 7800 SIGNATURE FOUND!')
else:
    print('❌ SIGNATURE MISSING OR INCORRECT')
    expected = b'ATARI7800'
    print(f'   Expected: {" ".join(f"{b:02X}" for b in expected)}')
    
print()

# Check reset vectors
reset_offset = 0xFFFC - base_addr
if reset_offset + 2 <= rom_size:
    reset_lo = rom[reset_offset]
    reset_hi = rom[reset_offset + 1]
    reset_vec = (reset_hi << 8) | reset_lo
    print(f'=== RESET VECTOR at $FFFC ===')
    print(f'Value: ${reset_vec:04X}')
