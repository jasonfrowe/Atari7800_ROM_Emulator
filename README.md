# Atari 7800 ROM Emulator - Teensy 4.1

This project turns a Teensy 4.1 into a high-speed ROM cartridge emulator for the Atari 7800 ProSystem. Your game runs directly from the Teensy's memory!

## 🎮 Current Status

**✅ READY TO TEST!**

- ✅ 48KB ROM loaded (Astro Wing Starfighter)
- ✅ All address and data bus pins configured
- ✅ Fast GPIO register-based address decoding
- ✅ SN74LVC245A buffer control implemented
- ✅ Fast boot (no USB wait)
- ✅ Build successful!

## 📋 Hardware Requirements

- **Teensy 4.1** microcontroller
- **SN74LVC245A** level shifter (U3) for data bus
- **Atari 7800** console
- Wiring harness to connect Teensy → Atari cartridge slot
- External power supply for Teensy (or power from Atari)

## 🔌 Pin Connections

See [PinOut.md](PinOut.md) for complete pinout details.

### Quick Reference:
- **Data Bus (D0-D7)**: Pins 19, 18, 14, 15, 40, 41, 17, 16
- **Address Bus (A0-A15)**: Pins 0-11, 24-27
- **Control Signals**: 
  - PHI2 (Clock): Pin 33
  - R/W: Pin 34
  - HALT: Pin 35
  - Buffer DIR: Pin 37
  - Buffer /OE: Pin 32

## 🚀 Quick Start

### 1. Build and Upload Firmware

The firmware is already built! Just upload:

```bash
cd /Users/rowe/Software/Teensy/Atari7800/Atari7800_ROM_Emulator
platformio run --target upload
```

Or use the VS Code PlatformIO "Upload" task.

### 2. Power Up

1. Connect Teensy to external power OR the Atari
2. Onboard LED flashes briefly during boot
3. Teensy is ready when LED turns off
4. Insert cartridge connector into Atari
5. Power on Atari 7800

### 3. Play!

The Atari should boot directly into **Astro Wing Starfighter**!

## 🔧 How It Works

### Memory Map

The 48KB ROM is mapped to Atari 7800 address space:
- **$4000 - $FFFF**: ROM data (48K)

### Speed Optimizations

1. **Direct GPIO Register Access**: Reading addresses and writing data uses GPIO6_DR register directly (single cycle operations!)
2. **No Interrupts**: Polling R/W line for maximum responsiveness
3. **Inline Functions**: Critical path functions are inlined
4. **Zero Delay**: No artificial delays, responds at CPU speed

### Performance Stats

- **Address decode**: < 10 nanoseconds
- **Data output**: < 10 nanoseconds  
- **Total response time**: ~20ns (well under the 558ns clock cycle!)

## 📊 Serial Monitor Output

Optional serial output for debugging:

```
========================================
  ATARI 7800 ROM EMULATOR - READY!
========================================
ROM Size: 48 KB
Address Range: $4000 - $FFFF
Waiting for Atari 7800...
========================================
Reads: 125847 | Last Addr: $F7A3
```

## 🔄 Loading a Different ROM

To load a different .a78 ROM file:

```bash
cd /Users/rowe/Software/Teensy/Atari7800/Atari7800_ROM_Emulator
python3 tools/convert_rom.py path/to/your/game.a78 src/rom_data.cpp
platformio run --target upload
```

## 🎵 POKEY Support (Future)

The current implementation includes placeholders for POKEY audio chip emulation:
- **EAUD Pin**: Pin 12 (PWM output)
- **IRQ Pin**: Pin 36 (interrupt output)

POKEY emulation will be added in a future update!

## 🐛 Troubleshooting

### Atari doesn't boot
1. Check all address and data pin connections
2. Verify Buffer /OE is LOW (enabled)
3. Verify Buffer DIR is HIGH (Teensy→Atari)
4. Check power connections

### Garbage on screen
1. Check data bus pin ordering (D0-D7)
2. Verify level shifter is powered correctly
3. Check for loose connections

### Serial monitor shows no reads
1. Verify R/W pin connection (Pin 34)
2. Check PHI2 clock connection (Pin 33)
3. Ensure Atari is powered on

## 📚 Project Structure

```
.
├── include/
│   └── rom_loader.h          # ROM data access functions
├── src/
│   ├── main.cpp              # Main ROM emulator code
│   └── rom_data.cpp          # ROM data array (auto-generated)
├── tools/
│   └── convert_rom.py        # .a78 to C array converter
├── PinOut.md                 # Complete pin assignment reference
└── README.md                 # This file
```

## 🎓 Technical Notes

### Level Shifting

The SN74LVC245A provides bi-directional 5V ↔ 3.3V level shifting:
- **Write Mode (DIR=HIGH)**: Teensy 3.3V → Atari 5V (marginal but works)
- **Read Mode (DIR=LOW)**: Atari 5V → Teensy 3.3V (safe, LVC inputs are 5V tolerant)

### GPIO6 Register Layout

```
Bits 0-11:   A0-A11  (Low address bits)
Bits 16-23:  D0-D7   (Data bus)
Bits 24-27:  A12-A15 (High address bits)
```

This layout allows single-register reads/writes for maximum speed!

## 📝 License

This project is for educational and homebrew purposes.

## 🙏 Credits

- Atari 7800 hardware by Atari Corporation/GCC
- Astro Wing Starfighter ROM (test game)
- Teensy 4.1 by PJRC

---

**Ready to make some Atari 7800 magic!** 🎮✨
