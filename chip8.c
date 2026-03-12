#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


uint8_t fontset[80] = {
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

// CHIP 8 INITILIZATION

void chip8_init(Chip8 *chip8){
    memset(chip8, 0 , sizeof(Chip8));
    chip8->PC= 0x200;
    chip8->I = 0;
    chip8->sp = 0;
    memcpy(chip8->memory, fontset, sizeof(fontset));
    srand(time(NULL));
}

// CHIP 8 ROM LOADER FUNCTION

void chip8_load_rom(Chip8 *chip8, const char *filename){
    FILE *fptr;
    fptr = fopen(filename,"rb");
    if(fptr == NULL){
        printf("Error: Could not open file.\n");
        exit(EXIT_FAILURE);
    }
    fread(&chip8->memory[0x200], 1, 3584, fptr);
    fclose(fptr);
}
// &chip8->memory[0x200]  →  where to put the bytes (memory starting at 512)
// 1                      →  each element is 1 byte
// 3584                   →  max bytes to read (4096 - 512 = 3584, remaining memory)
// fptr                   →  the file to read from

// CHIP8 CYCLE(FETCH),DECODE,EXECUTE

void chip8_cycle(Chip8 *chip8){
    if (chip8->PC >= 4095) {
        fprintf(stderr, "Error: PC out of bounds (%04X)\n", chip8->PC);
        exit(1);
    }

    uint16_t opcode = (chip8->memory[chip8->PC] << 8) | chip8->memory[chip8->PC+1];
    chip8->PC += 2;

    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t y  = (opcode & 0x00F0) >> 4;
    uint8_t nn = (opcode & 0x00FF);
    uint8_t n  = (opcode & 0x000F);

    switch(opcode & 0xF000){
        case 0x0000:
            if(opcode == 0x00E0){
                memset(chip8->display, 0, sizeof(chip8->display));
                chip8->draw_flag = 1;
            }
            else if(opcode == 0x00EE){
                if (chip8->sp == 0) {
                    fprintf(stderr, "Error: Stack underflow\n");
                    exit(1);
                }
                chip8->sp--;
                chip8->PC = chip8->stack[chip8->sp];
            }
            break;

        case 0x1000:
            chip8->PC = opcode & 0x0FFF;
            break;

        case 0x2000:
            if (chip8->sp >= 16) {
                fprintf(stderr, "Error: Stack overflow\n");
                exit(1);
            }
            chip8->stack[chip8->sp] = chip8->PC;
            chip8->sp++;
            chip8->PC = opcode & 0x0FFF;
            break;

        case 0x3000:
            if(chip8->V[x] == nn)
                chip8->PC += 2;
            break;

        case 0x4000:
            if(chip8->V[x] != nn)
                chip8->PC += 2;
            break;

        case 0x5000:
            if(chip8->V[x] == chip8->V[y])
                chip8->PC += 2;
            break;

        case 0x6000:
            chip8->V[x] = nn;
            break;

        case 0x7000:
            chip8->V[x] += nn;
            break;

        case 0x8000:
            switch(n){

                case 0:
                    chip8->V[x] = chip8->V[y];
                    break;

                case 1:
                    chip8->V[x] |= chip8->V[y];
                    chip8->V[0xF] = 0;
                    break;

                case 2:
                    chip8->V[x] &= chip8->V[y];
                    chip8->V[0xF] = 0;
                    break;

                case 3:
                    chip8->V[x] ^= chip8->V[y];
                    chip8->V[0xF] = 0;
                    break;

                case 4:
                    if(chip8->V[x] + chip8->V[y] > 255)
                        chip8->V[0xF] = 1;
                    else
                        chip8->V[0xF] = 0;
                    chip8->V[x] += chip8->V[y];
                    break;

                case 5:
                    if(chip8->V[x] > chip8->V[y])
                        chip8->V[0xF] = 1;
                    else
                        chip8->V[0xF] = 0;
                    chip8->V[x] = chip8->V[x] - chip8->V[y];
                    break;

                case 6:
                    chip8->V[x] = chip8->V[y];
                    chip8->V[0xF] = chip8->V[x] & 0x1;
                    chip8->V[x] >>= 1;
                    break;
                                
                case 7:
                    chip8->V[0xF] = (chip8->V[y] >= chip8->V[x]) ? 1 : 0;
                    chip8->V[x] = chip8->V[y] - chip8->V[x];
                    break;

                case 0xE:
                    chip8->V[x] = chip8->V[y];
                    chip8->V[0xF] = (chip8->V[x] & 0x80) >> 7;
                    chip8->V[x] <<= 1;
                    break;

                default:
                    fprintf(stderr, "Unknown opcode: %04X at PC: %04X\n", opcode, chip8->PC - 2);
                    break;
            }
            break;

        case 0xA000:
            chip8->I = opcode & 0x0FFF;
            break;
        
        case 0xB000:
            chip8->PC = (opcode & 0x0FFF) + chip8->V[0];
            break;

        case 0xD000:
            {
                uint8_t x_coord = chip8->V[x] % 64;
                uint8_t y_coord = chip8->V[y] % 32;
                chip8->V[0xF] = 0;
                for (int row = 0; row < n; row++) {
                    uint8_t sprite_byte = chip8->memory[chip8->I + row];
                    for (int col = 0; col < 8; col++) {
                        if (sprite_byte & (0x80 >> col)) {
                            int px = (x_coord + col) % 64;
                            int py = (y_coord + row) % 32;
                            int pixel_index = py * 64 + px;
                            if (chip8->display[pixel_index] == 1)
                                chip8->V[0xF] = 1;
                            chip8->display[pixel_index] ^= 1;
                        }
                    }
                }
                chip8->draw_flag = 1;
            }
            break;

        case 0xC000:
            chip8->V[x] = (rand() % 256) & nn;
            break;

        case 0xE000:
            if(nn == 0x9E){
                if(chip8->keypad[chip8->V[x]])
                    chip8->PC += 2;
            }
            else if(nn == 0xA1){
                if(!chip8->keypad[chip8->V[x]])
                    chip8->PC += 2;
            }
            break;

        case 0xF000:
            switch(nn){
                case 0x07:
                    chip8->V[x] = chip8->dt;
                    break;
                case 0x0A:
                    {
                        int key_pressed = 0;
                        for(int i = 0; i < 16; i++){
                            if(chip8->keypad[i]){
                                key_pressed = 1;
                                chip8->V[x] = i;
                                break;
                            }
                        }
                        if(key_pressed == 0)
                            chip8->PC -= 2;
                    }
                    break;
                case 0x15:
                    chip8->dt = chip8->V[x];
                    break;
                case 0x18:
                    chip8->st = chip8->V[x];
                    break;
                case 0x1E:
                    chip8->I += chip8->V[x];
                    break;

                case 0x29:
                    chip8->I = chip8->V[x] * 5;
                    break;
                
                case 0x33:
                    chip8->memory[chip8->I]   = chip8->V[x] / 100;
                    chip8->memory[chip8->I+1] = (chip8->V[x] / 10) % 10;
                    chip8->memory[chip8->I+2] = chip8->V[x] % 10;
                    break;

                case 0x55:
                    for(int i = 0; i <= x; i++)
                        chip8->memory[chip8->I + i] = chip8->V[i];
                    break;

                case 0x65:
                    for(int i = 0; i <= x; i++)
                        chip8->V[i] = chip8->memory[chip8->I + i];
                    break;

                default:
                    break;
        }
        break;
    }
}
