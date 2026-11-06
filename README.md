# Dungeon Crawler on DE1-SoC

A real-time dungeon game inspired by *Soul Knight*, written in bare-metal C for the RISC-V processor on the DE1-SoC FPGA board. There is no operating system: the game drives the VGA, PS/2 and audio hardware directly through memory-mapped I/O registers.

Built for ECE243 (Computer Organization) at the University of Toronto, March 2026, by [maijiOvO](https://github.com/maijiOvO) and [Williamzou-0610](https://github.com/Williamzou-0610).

## Gameplay

Fight through connected rooms, open treasure rooms, and defeat a boss with its own attacks and music.

| Key | Action |
|---|---|
| W / A / S / D | Move |
| J | Attack and interact |

The game runs on real hardware and in [CPUlator](https://cpulator.01xz.net/). Our board setup could not use a PS/2 keyboard and mouse at the same time, so the game is keyboard-only and the weapon automatically aims at the nearest enemy.

## How the hardware is used

| Peripheral | Base address | Approach |
|---|---|---|
| VGA pixel buffer | `0xFF203020` | Two 16-bit RGB565 frame buffers swapped on vsync |
| PS/2 keyboard | `0xFF200100` | Polls the data register and decodes make and break codes into key states |
| Audio codec | `0xFF203040` | Fills the output FIFO with mixed samples, based on its free-space register |

**Double buffering.** Each frame is drawn to the back buffer. The program then requests a swap and polls the status register until vsync completes, which prevents tearing. When the player changes rooms, the saved positions for both buffers are synchronized so that erasing old sprites does not leave artifacts.

**Audio without interrupts.** The game loop is fully polled. `Audio_Update()` runs at several points in each frame, including inside the vsync wait, so the FIFO keeps getting refilled. Sound effects and looping background music are added together and clamped to the 16-bit range to avoid overflow noise.

## Project structure

```text
hardware/     Peripheral drivers: VGA, PS/2, audio
game_logic/   Main loop, player, enemies, weapons, map
sources/      Sprite, map texture and audio data compiled into C arrays
start.s       Sets up the stack and calls main
```

Hardware access is kept in `hardware/`, separate from the game rules in `game_logic/`.

## Build and run

Install a RISC-V GCC toolchain and add `riscv-none-elf-gcc` to your `PATH`. Then run this on Windows:

```bat
build.bat
```

This produces `test.elf` for RV32 (`-march=rv32imf_zicsr_zicbom -mabi=ilp32f`). To run it without a board, open CPUlator, select the RISC-V DE1-SoC system, and load `test.elf`.
