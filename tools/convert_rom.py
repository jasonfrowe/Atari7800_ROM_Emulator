#!/usr/bin/env python3
import sys
import os

def convert_a78_to_header(input_file, output_file):
    with open(input_file, 'rb') as f:
        data = f.read()
    
    # 7800 Header is 128 bytes. Everything after is game data.
    rom_data = data[128:]
    rom_size = len(rom_data)
    
    header = data[0:128]
    # Clean up game name
    game_name = header[0x11:0x31].decode('ascii', errors='ignore').replace('\x00', '').strip()
    
    print(f"Game: {game_name}")
    print(f"ROM Size: {rom_size} bytes")

    with open(output_file, 'w') as f:
        name_def = "GAME_ROM_H"
        f.write(f"#ifndef {name_def}\n#define {name_def}\n\n")
        f.write(f"// Game: {game_name}\n")
        f.write(f"const uint32_t ROM_SIZE = {rom_size};\n\n")
        f.write(f"const uint8_t ROM_DATA[{rom_size}] = {{\n")
        
        for i in range(0, rom_size, 16):
            f.write("    ")
            chunk = rom_data[i:i+16]
            f.write(", ".join(f"0x{b:02X}" for b in chunk))
            if i + 16 < rom_size:
                f.write(",")
            f.write("\n")
        
        f.write("};\n\n#endif\n")
    
    print(f"Created {output_file}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python rom_convert.py <game.a78> <include/game_rom.h>")
    else:
        convert_a78_to_header(sys.argv[1], sys.argv[2])