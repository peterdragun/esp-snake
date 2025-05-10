# esp-snake

Affordable "Gameboy" style console on ESP32 with LED Matrix.

Currently there are two games:
- Snake
- Connect 4

Project is using ESP-IDF framework (tested on 5.5, should work with at least 5.3+)

## HW Requirements

- 5 buttons
- ESP32 (any should work, I was using ESP32-C3 super mini)
- LED Matrix 8x8 (with addreseable LED)
- Soldering iron, wires and solder

> [!IMPORTANT]
> There are at least two sizes of LED Matrix, so please make sure that yours match the dimensions or else the matrix will not fit into the 3D printed case.

## Wiring

| GPIO | Device       |
|------|--------------|
| 0    | Reset Button |
| 1    | Right Button |
| 2    | Down Button  |
| 3    | Up Button    |
| 4    | Left Button  |
| 5    | LED Matrix   |

Pins are configurable using ``idf.py menuconfig``, so if these does not fit for your devkit, you can change them easily.

TODO: wire diagram
