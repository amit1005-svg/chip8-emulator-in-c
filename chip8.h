#ifndef CHIP8_H
#define CHIP8_H
#include <stdint.h>
#include <stdbool.h>

typedef struct{
    uint8_t memory[4096];
    uint8_t V[16];
    uint16_t I;
    uint16_t PC;
    uint16_t stack[16];
    uint8_t SP;
    uint8_t delay_timer;
    uint8_t sound_timer;
    bool display[64 * 32];
    bool keypad[16];
} CHIP8_State;

void init_chip8(CHIP8_State *cpu);
bool load_rom(CHIP8_State *cpu, char *filename);
void emulate_cycle(CHIP8_State *cpu);

#endif