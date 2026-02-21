#!/usr/bin/env python3
# Check for 7800 signature in ROM

import re

# Read the ROM data
with open('include/game_rom.h', 'r') as f:
    content = f.read()

# Extract the data array
data_match = re.search(r'const uint8_t ROM_DATA\[49152\] = \{([^}]+)\}', content, re.DOTALL)
if data_match:
    hex_values = re.findall(r'0x[0-9A-Fa-f]{2}', data_match.group(1))
    rom = [int(v, 16) for v in hex_values]
    
    # Calculate offset for $FF7C in a 48K ROM at $4000-$FFFF
    sig_addr = 0xFF7C
    ctrl_addr = 0xFF7B
    sig_offset = sig_addr - 0x4000  # = 48,252
    ctrl_offset = ctrl_addr - 0x4000  # = 48,251
    
    print(f"=== 7800 DETECTION CHECK ===")
    print(f"ROM size: {len(rom)} bytes")
    print()
    
    # Check control byte at $FF7B
    if ctrl_offset < len(rom):
        ctrl_byte = rom[ctrl_offset]
        print(f"Control byte at ${ctrl_addr:04X} (offset {ctrl_offset}):")
        print(f"  Value: 0x{ctrl_byte:02X}")
        tv_format = 'PAL' if (ctrl_byte & 1) else 'NTSC'
        pokey = 'Yes' if (ctrl_byte & 2) else 'No'
        print(f"  Bit 0: {tv_format}")
        print(f"  Bit 1: Pokey@$4000 = {pokey}")
        if (ctrl_byte & 0xFC):
            print(f"  Other bits: 0x{ctrl_byte & 0xFC:02X}")
        print()
    
    # Check signature at $FF7C
    print(f"Signature at ${sig_addr:04X} (offset {sig_offset}):")
    
    if sig_offset + 9 <= len(rom):
        signature = ''.join(chr(rom[sig_offset + i]) if 32 <= rom[sig_offset + i] < 127 else '.' 
                           for i in range(9))
        hex_sig = ' '.join(f'{rom[sig_offset + i]:02X}' for i in range(9))
        
        print(f"  ASCII: {signature}")
        print(f"  Hex:   {hex_sig}")
        print()
        
        expected = "ATARI7800"
        if signature == expected:
            print("✓ Signature FOUND!")
            print("\n✅ Both control byte and signature present!")
        else:
            print(f"✗ Signature WRONG - expected '{expected}'")
            print()
            print("Searching for signature elsewhere in ROM...")
            rom_bytes = bytes(rom)
            expected_bytes = expected.encode('ascii')
            pos = rom_bytes.find(expected_bytes)
            if pos >= 0:
                actual_addr = 0x4000 + pos
                print(f"  Found at ROM offset {pos} (address ${actual_addr:04X})")
            else:
                print("  NOT FOUND anywhere in ROM!")
    else:
        print(f"ERROR: Offset {sig_offset} is beyond ROM size!")
