#!/usr/bin/env python3
"""
Check if address line swaps could explain boot loop behavior.
Analyzes reset vectors and common boot addresses.
"""

import sys

# Read the ROM file
if len(sys.argv) != 2:
    print("Usage: python3 check_vectors.py <path_to_.a78_file>")
    sys.exit(1)

with open(sys.argv[1], 'rb') as f:
    rom_data = f.read()[128:]  # Skip 128-byte header

ROM_SIZE = len(rom_data)
ROM_START = 0x4000 if ROM_SIZE > 32768 else 0x8000

print(f"ROM Size: {ROM_SIZE} bytes ({ROM_SIZE//1024}KB)")
print(f"ROM maps to: ${ROM_START:04X}-$FFFF")
print()

# Read reset vectors at $FFFC-$FFFD
reset_vector_offset = 0xFFFC - ROM_START
reset_low = rom_data[reset_vector_offset]
reset_high = rom_data[reset_vector_offset + 1]
reset_addr = (reset_high << 8) | reset_low

print(f"=== RESET VECTOR ===")
print(f"At $FFFC-$FFFD: ${reset_high:02X}{reset_low:02X} → ${reset_addr:04X}")
print()

# Check the addresses from the capture
captured_addrs = [0x83F8, 0x8BF8, 0x87F8, 0x8FF8, 0x93F8, 0x9BF8, 0x97F8, 0x9FF8]

print(f"=== CAPTURED ADDRESS ANALYSIS ===")
for addr in captured_addrs:
    if addr >= ROM_START:
        offset = addr - ROM_START
        if offset < ROM_SIZE:
            data = rom_data[offset]
            print(f"${addr:04X}: 0x{data:02X} ({data:3d}) = ", end="")
            # Try to interpret as 6502 instruction
            if data == 0xA9:
                print("LDA #immediate")
            elif data == 0xA2:
                print("LDX #immediate")
            elif data == 0x8D:
                print("STA absolute")
            elif data == 0x4C:
                print("JMP absolute")
            elif data == 0x20:
                print("JSR absolute")
            elif data == 0x60:
                print("RTS")
            elif data == 0xEA:
                print("NOP")
            else:
                print(f"data/unknown opcode")

print()
print("=== ADDRESS LINE SWAP CHECK ===")
print("Testing common swaps (would cause reading wrong addresses):")
print()

def swap_bits(addr, bit1, bit2):
    """Swap two bits in an address"""
    mask1 = 1 << bit1
    mask2 = 1 << bit2
    
    bit1_val = (addr & mask1) >> bit1
    bit2_val = (addr & mask2) >> bit2
    
    # Clear both bits
    addr &= ~(mask1 | mask2)
    
    # Set swapped bits
    addr |= (bit2_val << bit1)
    addr |= (bit1_val << bit2)
    
    return addr

# Test if common address line swaps would explain the pattern
test_swaps = [
    (10, 11, "A10<->A11"),
    (12, 13, "A12<->A13"),
    (11, 12, "A11<->A12"),
    (8, 9, "A8<->A9"),
]

for bit1, bit2, label in test_swaps:
    swapped_reset = swap_bits(reset_addr, bit1, bit2)
    print(f"{label}: Reset would read ${swapped_reset:04X} instead of ${reset_addr:04X}")
    
    if swapped_reset >= ROM_START and swapped_reset < 0x10000:
        offset = swapped_reset - ROM_START
        if offset < ROM_SIZE:
            data = rom_data[offset]
            print(f"  Data at swapped address: 0x{data:02X}")

print()
print("=== BOOT CODE AT RESET VECTOR ===")
if reset_addr >= ROM_START:
    offset = reset_addr - ROM_START
    if offset < ROM_SIZE - 20:
        print(f"First 20 bytes at ${reset_addr:04X}:")
        for i in range(20):
            if i % 8 == 0:
                print(f"\n${reset_addr+i:04X}: ", end="")
            print(f"{rom_data[offset+i]:02X} ", end="")
        print()
