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
| EAUD              | Output    | 37         | GPIO7_DR bit 19   | PWM Audio Output                      |

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


00 -> GPIO6-03 | GPIO6-02 -> 01
01 -> GPIO6-02 | GPIO6-03 -> 00
02 -> GPIO9-04 | GPIO6-12 -> 24
03 -> GPIO9-05 | GPIO6-13 -> 25
04 -> GPIO9-06 | GPIO6-16 -> 19
05 -> GPIO9-08 | GPIO6-17 -> 18
06 -> GPIO7-10 | GPIO6-18 -> 14
07 -> GPIO7-17 | GPIO6-19 -> 15
08 -> GPIO7-16 | GPIO6-20 -> 40
09 -> GPIO7-11 | GPIO6-21 -> 41
10 -> GPIO7-00 | GPIO6-22 -> 17
11 -> GPIO7-02 | GPIO6-23 -> 16
12 -> GPIO7-01 | GPIO6-24 -> 22
13 -> GPIO7-03 | GPIO6-25 -> 23
14 -> GPIO6-18 | GPIO6-26 -> 20
15 -> GPIO6-19 | GPIO6-27 -> 21
16 -> GPIO6-23 | GPIO6-28 -> 38
17 -> GPIO6-22 | GPIO6-29 -> 39
18 -> GPIO6-17 | GPIO6-30 -> 26
19 -> GPIO6-16 | GPIO6-31 -> 27
20 -> GPIO6-26 | GPIO7-00 -> 10
21 -> GPIO6-27 | GPIO7-01 -> 12
22 -> GPIO6-24 | GPIO7-02 -> 11
23 -> GPIO6-25 | GPIO7-03 -> 13
24 -> GPIO6-12 | GPIO7-10 -> 06
25 -> GPIO6-13 | GPIO7-11 -> 09
26 -> GPIO6-30 | GPIO7-12 -> 32
27 -> GPIO6-31 | GPIO7-16 -> 08
28 -> GPIO8-18 | GPIO7-17 -> 07
29 -> GPIO9-31 | GPIO7-18 -> 36
30 -> GPIO8-23 | GPIO7-19 -> 37
31 -> GPIO8-22 | GPIO7-28 -> 35
32 -> GPIO7-12 | GPIO7-29 -> 34
33 -> GPIO9-07 | GPIO8-12 -> 45
34 -> GPIO7-29 | GPIO8-13 -> 44
35 -> GPIO7-28 | GPIO8-14 -> 43
36 -> GPIO7-18 | GPIO8-15 -> 42
37 -> GPIO7-19 | GPIO8-16 -> 47
38 -> GPIO6-28 | GPIO8-17 -> 46
39 -> GPIO6-29 | GPIO8-18 -> 28
40 -> GPIO6-20 | GPIO8-22 -> 31
41 -> GPIO6-21 | GPIO8-23 -> 30
42 -> GPIO8-15 | GPIO9-04 -> 02
43 -> GPIO8-14 | GPIO9-05 -> 03
44 -> GPIO8-13 | GPIO9-06 -> 04
45 -> GPIO8-12 | GPIO9-07 -> 33
46 -> GPIO8-17 | GPIO9-08 -> 05
47 -> GPIO8-16 | GPIO9-22 -> 51
48 -> GPIO9-24 | GPIO9-24 -> 48
49 -> GPIO9-27 | GPIO9-25 -> 53
50 -> GPIO9-28 | GPIO9-26 -> 52
51 -> GPIO9-22 | GPIO9-27 -> 49
52 -> GPIO9-26 | GPIO9-28 -> 50
53 -> GPIO9-25 | GPIO9-29 -> 54
54 -> GPIO9-29 | GPIO9-31 -> 29