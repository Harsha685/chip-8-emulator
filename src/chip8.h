#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

typedef struct {
    uint8_t memory[4096]; // 0x200
    uint8_t display[64*32];
    uint8_t  keypad[16];
    uint8_t sp; // STACK POINTER //
    uint8_t V[16];
    uint8_t dt; // DELAY TIMER//
    uint8_t st; // SOUND TIMER//
    uint16_t PC; // PROGRAM COUNTER //
    uint16_t I;
    uint16_t stack[16];
    int      draw_flag;
} Chip8;

void chip8_init(Chip8 *chip8);
void chip8_load_rom(Chip8 *chip8, const char *filename);
void chip8_cycle(Chip8 *chip8);

#endif