# Teensy 4.1 to Atari 7800 Cartridge Pinout

## REWIRED for Fast GPIO Register Access (GPIO6/GPIO7 Alignment)

This configuration allows atomic address reading via direct GPIO register access for maximum speed on real Atari 7800 hardware.

## Data Bus (Bi-Directional via SN74LVC245A - U3)

| Atari 7800 Signal | Direction (Teensy View) | Teensy Pin | GPIO Register     |
|-------------------|-------------------------|------------|-------------------|
| D0 (Data 0)       | Bi-Directional          | 19         | GPIO6_DR bit 16   |
| D1 (Data 1)       | Bi-Directional          | 18         | GPIO6_DR bit 17   |
| D2 (Data 2)       | Bi-Directional          | 14         | GPIO6_DR bit 18   |
| D3 (Data 3)       | Bi-Directional          | 15         | GPIO6_DR bit 19   |
| D4 (Data 4)       | Bi-Directional          | 40         | GPIO6_DR bit 20   |
| D5 (Data 5)       | Bi-Directional          | 41         | GPIO6_DR bit 21   |
| D6 (Data 6)       | Bi-Directional          | 17         | GPIO6_DR bit 22   |
| D7 (Data 7)       | Bi-Directional          | 16         | GPIO6_DR bit 23   |

## Address Bus (Input to Teensy) - REWIRED

| Atari 7800 Signal | Direction | Teensy Pin | GPIO Register       |
|-------------------|-----------|------------|---------------------|
| A0                | Input     | 10         | GPIO7_PSR bit 0     |
| A1                | Input     | 12         | GPIO7_PSR bit 1     |
| A2                | Input     | 11         | GPIO7_PSR bit 2     |
| A3                | Input     | 13         | GPIO7_PSR bit 3     |
| A4                | Input     | 6          | GPIO7_PSR bit 10    |
| A5                | Input     | 9          | GPIO7_PSR bit 11    |
| A6                | Input     | 32         | GPIO7_PSR bit 12    |
| A7                | Input     | 8          | GPIO7_PSR bit 16    |
| A8                | Input     | 22         | GPIO6_PSR bit 24    |
| A9                | Input     | 23         | GPIO6_PSR bit 25    |
| A10               | Input     | 20         | GPIO6_PSR bit 26    |
| A11               | Input     | 21         | GPIO6_PSR bit 27    |
| A12               | Input     | 38         | GPIO6_PSR bit 28    |
| A13               | Input     | 39         | GPIO6_PSR bit 29    |
| A14               | Input     | 26         | GPIO6_PSR bit 30    |
| A15               | Input     | 27         | GPIO6_PSR bit 31    |

## Control Signals

| Atari 7800 Signal | Direction | Teensy Pin | GPIO Register     | Description                           |
|-------------------|-----------|------------|-------------------|---------------------------------------|
| PHI2 (Clock)      | Input     | 4          | GPIO9_PSR bit 6   | System clock (~1.79 MHz)              |
| R/W               | Input     | 3          | GPIO9_PSR bit 5   | Read/Write control (HIGH = Read)      |
| HALT              | Input     | 5          | GPIO9_PSR bit 8   | Console→Cart control (LOW = Halt)     |
| Buffer /OE        | Output    | 2          | GPIO9_DR bit 4    | U3 Output Enable (LOW = Enabled)      |
| Buffer DIR        | Output    | 33         | GPIO7_DR bit 12   | U3 Direction (HIGH = Teensy→Atari)    |

## Notes

- **Level Shifting**: SN74LVC245A level shifters handle 5V↔3.3V conversion
  - U3 (bi-directional): Data bus D0-D7
  - U2, U4, U5 (listen-only, DIR+/OE tied to GND): Address and control signals
- **Fast Address Reading**: Wiring aligned to GPIO6/GPIO7 for atomic register reads
  - A0-A7 on GPIO7 allows single `GPIO7_PSR` read
  - A8-A15 on GPIO6 allows single `GPIO6_PSR` read  
  - Full 16-bit address read in ~10ns (2 register reads)
- **U3 Configuration**: 
  - Pull-up: /OE has 10K pull-up to +3.3V
  - DIR HIGH = Teensy (A) → Atari (B)
  - DIR LOW = Atari (B) → Teensy (A)
- **ROM Mapping**: 48K ROM at addresses $4000-$FFFF (active when A15=1 OR A14=1)
- **Boot Optimization**: Uses `startup_middle_hook()` to skip 300ms startup delay
- **Memory**: ROM data read directly from Flash (no RAM copy needed)
