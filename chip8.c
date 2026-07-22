#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LEGACY_MODE 1
const uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
void init_chip8(CHIP8_State *cpu)
{
    memset(cpu, 0, sizeof(CHIP8_State));
    cpu->PC = 0x0200;
    for (uint8_t i = 0; i < 80; i++)
    {
        cpu->memory[0x0050 + i] = fontset[i];
    }
}
bool load_rom(CHIP8_State *cpu, char *filename)
{
    printf("rom: %s\n", filename);
    FILE *file = fopen(filename, "rb");
    if (!file){
        printf("FAIL: no file\n");
        return false;
    }
    fseek(file, 0, SEEK_END);
    long rom_size = ftell(file);
    rewind(file);
    if (rom_size > 4096 - 0x0200)
    {
        printf("FAIL: rom big\n");
        fclose(file);
        return false;
    }
    fread(&cpu->memory[0x200], 1, rom_size, file);
    fclose(file);
    return true;
}
void emulate_cycle(CHIP8_State *cpu)
{
    uint16_t opcode = (cpu->memory[cpu->PC] << 8) | cpu->memory[(cpu->PC) + 1];
    cpu->PC += 2;
    switch (opcode & 0xf000)
    {
    case 0x0000:
        switch (opcode)
        {
        case 0x00e0: // CLS
            memset(cpu->display, 0, sizeof(cpu->display));
            break;
        case 0x00ee: // RET
            cpu->PC = cpu->stack[cpu->SP];
            cpu->SP -= 1;
            break;
        default: // SYS addr
            break;
        }
        break;

    case 0x1000: // JP addr
        cpu->PC = opcode & 0x0fff;
        break;

    case 0x2000: // CALL addr
        cpu->SP += 1;
        cpu->stack[cpu->SP] = cpu->PC;
        cpu->PC = opcode & 0x0fff;
        break;

    case 0x3000: // SE Vx, byte
        if ((cpu->V[(opcode & 0x0f00) >> 8]) == (opcode & 0xff))
            cpu->PC += 2;
        break;

    case 0x4000: // SNE Vx, byte
        if ((cpu->V[(opcode & 0x0f00) >> 8]) != (opcode & 0xff))
            cpu->PC += 2;
        break;

    case 0x5000: // SE Vx, Vy
        if ((cpu->V[(opcode & 0x0f00) >> 8]) == (cpu->V[(opcode & 0x00f0) >> 4]))
            cpu->PC += 2;
        break;

    case 0x6000: // LD Vx, byte
        cpu->V[(opcode & 0x0f00) >> 8] = opcode & 0x00ff;
        break;

    case 0x7000: // ADD Vx, byte
        cpu->V[(opcode & 0x0f00) >> 8] += opcode & 0x00ff;
        break;

    case 0x8000:
    {
        uint16_t x = (opcode & 0x0f00) >> 8;
        uint16_t y = (opcode & 0x00f0) >> 4;
        switch (opcode & 0x000f)
        {
        case 0x0000: // LD Vx, Vy
            cpu->V[x] = cpu->V[y];
            break;
        case 0x0001: // OR Vx, Vy
            cpu->V[x] = cpu->V[x] | cpu->V[y];
            break;
        case 0x0002: // AND Vx, Vy
            cpu->V[x] = cpu->V[x] & cpu->V[y];
            break;
        case 0x0003: // XOR Vx, Vy
            cpu->V[x] = cpu->V[x] ^ cpu->V[y];
            break;
        case 0x0004: // ADD Vx, Vy
            uint16_t add = cpu->V[x] + cpu->V[y];
            cpu->V[x] = add & 0x00ff;
            if (add > 0x00ff)
            {
                cpu->V[15] = 1;
            }
            else
            {
                cpu->V[15] = 0;
            }
            break;
        case 0x0005: // SUB Vx, Vy
        {
            uint8_t flag = (cpu->V[x] >= cpu->V[y]) ? 1 : 0;
            cpu->V[x] = cpu->V[x] - cpu->V[y];
            cpu->V[15] = flag;
            break;
        }
        case 0x0006: // SHR Vx {, Vy}
            cpu->V[15] = cpu->V[x] & 0x01;
            cpu->V[x] = cpu->V[x] >> 1;
            break;
        case 0x0007: // SUBN Vx, Vy
        {
            uint8_t flag = (cpu->V[y] >= cpu->V[x]) ? 1 : 0;
            cpu->V[x] = cpu->V[y] - cpu->V[x];
            cpu->V[15] = flag;
            break;
        }
        case 0x000e: // SHL Vx {, Vy}
            cpu->V[15] = (cpu->V[x] & 0x80) >> 7;
            cpu->V[x] = cpu->V[x] << 1;
            break;
        }
        break;
    }
    case 0x9000: // SNE Vx, Vy
        if (cpu->V[(opcode & 0x0f00) >> 8] != cpu->V[(opcode & 0x00f0) >> 4])
            cpu->PC += 2;
        break;

    case 0xa000: // LD I, addr
        cpu->I = opcode & 0x0fff;
        break;

    case 0xb000: // JP V0, addr
        cpu->PC = (opcode & 0x0fff) + cpu->V[0];
        break;

    case 0xc000: // RND Vx, byte
        cpu->V[(opcode & 0x0f00) >> 8] = (rand() % 256) & (opcode & 0x00ff);
        break;

    case 0xd000: // DRW Vx, Vy, nibble
    {
        uint8_t x = (opcode & 0x0F00) >> 8;
        uint8_t y = (opcode & 0x00F0) >> 4;
        uint8_t n = opcode & 0x000F;
        uint8_t x0 = cpu->V[x] % 64;
        uint8_t y0 = cpu->V[y] % 32;
        cpu->V[15] = 0;
        for (uint8_t row = 0; row < n; row++)
        {
            uint8_t sprite_byte = cpu->memory[cpu->I + row];
            for (uint8_t col = 0; col < 8; col++)
            {
                uint8_t sprite_pixel = sprite_byte & (0x80 >> col);
                if (sprite_pixel != 0)
                {
                    if (x0 + col >= 64 || y0 + row >= 32)
                        continue;
                    uint16_t pos = (y0 + row) * 64 + (x0 + col);
                    if (cpu->display[pos] == 1)
                        cpu->V[15] = 1;
                    cpu->display[pos] ^= 1;
                }
            }
        }
        break;
    }
    case 0xe000:
        switch (opcode & 0x00ff)
        {
        case 0x009e: // SKP Vx
            if (cpu->keypad[cpu->V[(opcode & 0x0f00) >> 8]])
                cpu->PC += 2;
            break;
        case 0x00a1: // SKNP Vx
            if (!(cpu->keypad[cpu->V[(opcode & 0x0f00) >> 8]]))
                cpu->PC += 2;
            break;
        }
        break;

    case 0xf000:
    {
        uint16_t x = (opcode & 0x0f00) >> 8;
        switch (opcode & 0x00ff)
        {
        case 0x0007: // LD Vx, DT
            cpu->V[x] = cpu->delay_timer;
            break;
        case 0x000a: // LD Vx, K
            bool key_press = false;
            for (uint8_t i = 0; i < 16; i++)
            {
                if (cpu->keypad[i])
                {
                    cpu->V[x] = i;
                    key_press = true;
                    break;
                }
            }
            if (!key_press)
                cpu->PC -= 2;
            break;
        case 0x0015: // LD DT, Vx
            cpu->delay_timer = cpu->V[x];
            break;
        case 0x0018: // LD ST, Vx
            cpu->sound_timer = cpu->V[x];
            break;
        case 0x001e: // ADD I, Vx
            cpu->I += cpu->V[x];
            break;
        case 0x0029: // LD F, Vx
            cpu->I = 0x0050 + (cpu->V[x] * 5);
            break;
        case 0x0033: // LD B, Vx
            uint8_t value = cpu->V[x];
            cpu->memory[cpu->I] = value / 100;
            cpu->memory[cpu->I + 1] = (value / 10) % 10;
            cpu->memory[cpu->I + 2] = value % 10;
            break;
        case 0x0055: // LD [I], Vx
            for (uint8_t i = 0; i <= x; i++)
            {
                cpu->memory[cpu->I + i] = cpu->V[i];
            }
            if(LEGACY_MODE) cpu->I += x + 1;
            break;
        case 0x0065: // LD Vx, [I]
            for (uint8_t i = 0; i <= x; i++)
            {
                cpu->V[i] = cpu->memory[cpu->I + i];
            }
            if(LEGACY_MODE) cpu->I += x + 1;
            break;
        }
        break;
    }
    default:
        break;
    }
}
