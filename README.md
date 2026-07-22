# CHIP-8 Emulator

A custom-built CHIP-8 virtual machine written in C. This project implements a complete CPU core, memory management, and hardware-accelerated graphics and audio using the SDL3 library.

## Prerequisites
To compile and run this emulator from the source code, you will need:
* A C Compiler (like GCC)
* SDL3 Development Libraries: https://github.com/libsdl-org/SDL/releases

Note: The `include` and `lib` directories for SDL3 are not tracked in this repository. You must download them for your specific operating system and place them in the root directory before compiling.

## Build Instructions
Open your terminal in the root directory of the project and compile the source code using the following command:

Windows (MinGW/GCC):
```powershell
gcc main.c chip8.c -o chip8@emu -I./include -L./lib -lSDL3
```
To use a Chip-8 rom:
```powershell
./chip8@emu "romfile.ch8"
```
## Acknowledgements
* The CPU architecture and memory mapping were built strictly following Cowgod\'s Chip-8 Technical Reference : http://devernay.free.fr/hacks/chip8/C8TECH10.HTM by Thomas P. Greene.