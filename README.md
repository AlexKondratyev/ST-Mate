![alt text](/docs/logo.svg)

![](/docs/st-mate-board.png)

# ST-Mate – Chess Board on STM32F103C8

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)
[![License: CC BY-NC-SA 4.0](https://img.shields.io/badge/License-CC_BY--NC--SA_4.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc-sa/4.0/)

ST-Mate is a port of the legendary chess engine **micro-Max** (author H.G. Muller) to the STM32F103C8 microcontroller (Blue Pill). The project implements a full chess game with the ability to play human vs human and human vs computer.

## Features

- **micro-Max 4.8 Engine** with full set of features (deep search, hash tables, pruning, castling, en passant)
- **Adaptation for STM32F103C8** (20 KB RAM, 64 KB Flash)
- **Game modes**:
  - Human vs Human (PVP)
  - Human vs Computer (player can play as white or black)
- **PGN Generation** with recording games in standard format
- **Board visualization** via UART, Bluetooth or LED matrix
- **Hints**:
  - Highlighting possible moves
  - Hint button
- **Saving settings in Flash memory**

## Contents

- [Build and Flash](#build-and-flash)
- [Usage](#usage)
- [Hardware](#hardware)
- [License](#license)
- [Acknowledgments](#acknowledgments)

## Build and Flash
### Build Instructions

1. Clone the repository:
   ```bash
   git clone https://github.com/AlexKondratyev/ST-Mate.git
   cd STMate

### Requirements
- STM32F103C8 (Blue Pill) or compatible board
- USB-UART adapter (for loading and debugging)
- Development environment:
  - **STM32CubeIDE** + ARM GCC

## Usage
### Control
- Hint: press the HELP button – the engine will show the best move.
- Mode change: switch mode using the rotary switch.
- Reset: press the RESET button or send the REBOOT command.
- The rest can be found in the user_manual.pdf manual.
### Bluetooth Commands
  All available commands are described in user_manual.pdf appendix A.

## Hardware
Microcontroller: STM32F103C8 (72 MHz, 64 KB Flash, 20 KB RAM)
Field sensors: Hall sensor matrix SS49E (64 pcs.)
Display: ARGB LED strip (WS2812)
Communication: Bluetooth module (HC-06)
Power: 5V via USB.

## License
> **Software** is distributed under the **GPLv3** license.  
> **Hardware** (schematics, documentation) is distributed under the **CC BY-NC-SA 4.0** license (non-commercial use with attribution and distribution under the same conditions).

## Acknowledgments
 - H.G. Muller – author of micro-Max
 - STM32 community for useful advice
 - Wife and son for testing