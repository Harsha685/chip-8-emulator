# CHIP-8 Emulator
A fully working CHIP-8 emulator written in C using SDL2. Built from scratch as a systems programming learning project. Runs classic CHIP-8 ROMs including Space Invaders, Pong, Tetris, and Breakout.

---

## Building and Running

### Requirements
- GCC
- SDL2 (`sudo apt install libsdl2-dev` on Ubuntu)

### Build
```bash
make
```

### Run
```bash
./chip8 roms/SI.ch8
```

### Clean
```bash
make clean
```
--- 

## File Structure

```
Chip8/
├── chip8.h    → Chip8 struct definition and function declarations
├── chip8.c    → Emulator core: init, ROM loader, CPU cycle and all opcodes
├── main.c     → SDL2 window, input handling, render loop
├── Makefile   → Build configuration
└── roms/      → CHIP-8 ROM files (.ch8)
```


### chip8.h
Defines the entire emulator state as a single struct:

```c
typedef struct {
    uint8_t  memory[4096];   // 4KB RAM
    uint8_t  display[64*32]; // pixel buffer, 1 byte per pixel (on/off)
    uint8_t  keypad[16];     // key state: 1 = pressed, 0 = released
    uint8_t  sp;             // stack pointer
    uint8_t  V[16];          // general purpose registers V0-VF
    uint8_t  dt;             // delay timer, decrements at 60Hz
    uint8_t  st;             // sound timer, beeps while nonzero
    uint16_t PC;             // program counter
    uint16_t I;              // index register, stores memory addresses
    uint16_t stack[16];      // call stack for subroutine return addresses
    int      draw_flag;      // set to 1 when display needs redrawing
} Chip8;
```

### chip8.c
Contains three functions:
- `chip8_init` — zeroes the struct, loads fontset into memory at 0x000, sets PC to 0x200
- `chip8_load_rom` — opens a ROM file in binary mode and reads it into memory starting at 0x200
- `chip8_cycle` — fetches one opcode, decodes it, and executes it

### main.c
Contains the SDL2 setup and main game loop. Responsible for:
- Creating the window and renderer
- Initialising the emulator and loading the ROM from `argv[1]`
- Running the main loop: handle input → run CPU cycles → update timers → render

---
## Memory Layout

```
0x000 - 0x1FF  →  reserved (fontset stored here at 0x000)
0x200 - 0xFFF  →  ROM loaded here, programs start at 0x200
```

Programs always start at `0x200`. This is why `PC` is initialised to `0x200` and ROMs are copied into `memory[0x200]` by `chip8_load_rom`.

The fontset (sprites for hex digits 0–F) is stored at `0x000`. Each character sprite is exactly 5 bytes tall, which is why `FX29` calculates the font address as `V[X] * 5`.

```c
// font sprite for '0'
0xF0, 0x90, 0x90, 0x90, 0xF0

// font sprite for '1'
0x20, 0x60, 0x20, 0x20, 0x70
```

Each byte represents one row of 8 pixels. A set bit means the pixel is ON.

---

## Architecture

The emulator is split into two clear layers:

```
chip8.c  →  the virtual machine
             owns all state (memory, registers, display, timers)
             knows nothing about SDL or rendering

main.c   →  the host
             owns the window and renderer
             reads keypad events and writes to chip8.keypad[]
             reads chip8.display[] and renders it to screen
             decrements chip8.dt and chip8.st every frame
```

This separation means `chip8.c` is completely portable. You could replace SDL with any other graphics library and `chip8.c` would not change at all.

---

## CPU Cycle — Fetch, Decode, Execute

Every call to `chip8_cycle` performs one full instruction cycle.

### Fetch
Two bytes are read from memory at the current PC and combined into a single 16-bit opcode:

```c
uint16_t opcode = (chip8->memory[chip8->PC] << 8) | chip8->memory[chip8->PC + 1];
chip8->PC += 2;
```

The left shift by 8 moves the first byte into the high byte position. The OR combines it with the second byte. PC advances by 2 because every CHIP-8 instruction is exactly 2 bytes wide.

### Decode
The relevant fields are extracted from the opcode using bitwise masking:

```c
uint8_t x  = (opcode & 0x0F00) >> 8;  // register index X
uint8_t y  = (opcode & 0x00F0) >> 4;  // register index Y
uint8_t nn = (opcode & 0x00FF);        // 8-bit immediate value
uint8_t n  = (opcode & 0x000F);        // 4-bit immediate value
```

For example, opcode `0x8AB4`:
```
x  = A  (second nibble)
y  = B  (third nibble)
nn = B4 (last byte)
n  = 4  (last nibble)
```

### Execute
A switch statement on `opcode & 0xF000` dispatches to the correct handler. The top nibble identifies the opcode group. Some groups (0x8000, 0xF000) use a nested switch on `n` or `nn` to distinguish sub-opcodes.

---

## Opcode Handling

All 36 opcodes are implemented in `chip8_cycle`. Here is how each group is handled:

### 0x0000 — System
```
00E0  →  memset display to 0, set draw_flag
00EE  →  sp--, PC = stack[sp]  (return from subroutine)
```

### 0x1000 — Jump
```
1NNN  →  PC = NNN
```

### 0x2000 — Call Subroutine
```
2NNN  →  stack[sp] = PC   (save return address)
          sp++
          PC = NNN         (jump to subroutine)
```
This is the equivalent of a function call. The current PC is saved onto the stack so `00EE` can return to it later.

### 0x3000–0x5000 — Conditional Skip
```
3XNN  →  if V[x] == nn then PC += 2
4XNN  →  if V[x] != nn then PC += 2
5XY0  →  if V[x] == V[y] then PC += 2
```
Skipping is implemented by adding 2 to PC since each instruction is 2 bytes.

### 0x6000–0x7000 — Set and Add
```
6XNN  →  V[x] = nn
7XNN  →  V[x] += nn
```

### 0x8000 — Arithmetic and Bitwise
Inner switch on `n` (last nibble):
```
8XY0  →  V[x] = V[y]
8XY1  →  V[x] |= V[y]
8XY2  →  V[x] &= V[y]
8XY3  →  V[x] ^= V[y]
8XY4  →  VF = 1 if V[x]+V[y] > 255 else 0;  V[x] += V[y]
8XY5  →  VF = 1 if V[x] >= V[y] else 0;     V[x] -= V[y]
8XY6  →  VF = V[x] & 0x1;  V[x] >>= 1
8XY7  →  VF = 1 if V[y] >= V[x] else 0;     V[x] = V[y] - V[x]
8XYE  →  VF = (V[x] >> 7) & 0x1;  V[x] <<= 1
```

VF (register 15) is used as a flag register. It is always set before the arithmetic operation so that if x happens to be F, the flag is not overwritten by the result.

### 0x9000 — Skip if Not Equal
```
9XY0  →  if V[x] != V[y] then PC += 2
```

### 0xA000 — Set Index Register
```
ANNN  →  I = NNN
```

### 0xB000 — Jump with Offset
```
BNNN  →  PC = NNN + V[0]
```

### 0xC000 — Random
```
CXNN  →  V[x] = (rand() % 256) & nn
```

### 0xD000 — Draw Sprite
```
DXYN  →  draw N-byte sprite from memory[I] at (V[x], V[y])
          VF = 1 if any pixel is turned off (collision)
```

Sprites are drawn using XOR onto the display buffer:
```c
chip8->display[pixel_index] ^= 1;
```

If a pixel is already ON and XOR turns it OFF, a collision occurred and VF is set to 1. This is how games detect bullet-enemy collisions.

### 0xE000 — Key Input Skip
```
EX9E  →  if keypad[V[x]] is pressed then PC += 2
EXA1  →  if keypad[V[x]] is not pressed then PC += 2
```

### 0xF000 — Miscellaneous
Inner switch on `nn` (last byte):
```
FX07  →  V[x] = dt
FX0A  →  blocking key wait: if no key held, PC -= 2 (re-execute this instruction)
FX15  →  dt = V[x]
FX18  →  st = V[x]
FX1E  →  I += V[x]
FX29  →  I = V[x] * 5  (font sprite address, each sprite is 5 bytes)
FX33  →  memory[I]=V[x]/100;  memory[I+1]=(V[x]/10)%10;  memory[I+2]=V[x]%10
FX55  →  for i in 0..x: memory[I+i] = V[i]
FX65  →  for i in 0..x: V[i] = memory[I+i]
```

---

## Pseudocode Implemented

### chip8.c
```
#include "chip8.h" 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <time.h>

// FONTSET — lives at the top of chip8.c 
uint8_t fontset[80] = { 
0xF0,0x90,0x90,0x90,0xF0, // 0 
0x20,0x60,0x20,0x20,0x70, // 1 
0xF0,0x10,0xF0,0x80,0xF0, // 2 
0xF0,0x10,0xF0,0x10,0xF0, // 3 
0x90,0x90,0xF0,0x10,0x10, // 4 
0xF0,0x80,0xF0,0x10,0xF0, // 5 
0xF0,0x80,0xF0,0x90,0xF0, // 6 
0xF0,0x10,0x20,0x40,0x40, // 7 
0xF0,0x90,0xF0,0x90,0xF0, // 8 
0xF0,0x90,0xF0,0x10,0xF0, // 9 
0xF0,0x90,0xF0,0x90,0x90, // A 
0xE0,0x90,0xE0,0x90,0xE0, // B 
0xF0,0x80,0x80,0x80,0xF0, // C 
0xE0,0x90,0x90,0x90,0xE0, // D 
0xF0,0x80,0xF0,0x80,0xF0, // E 
0xF0,0x80,0xF0,0x80,0x80 // F 
};

// ───────────────────────────────────── 
// FUNCTION 1: chip8_init 
// ─────────────────────────────────────
void chip8_init(Chip8 *chip8): 
	zero out the entire struct using memset 
	set chip8->PC = 0x200 
	set chip8->I = 0 
	set chip8->SP = 0 
	copy fontset into chip8->memory starting at index 0x000 
		use memcpy(chip8->memory, fontset, sizeof(fontset)) 
	seed random number generator 
		srand(time(NULL))
		

// ───────────────────────────────────── 
// FUNCTION 2: chip8_load_rom 
// ───────────────────────────────────── 
void chip8_load_rom(Chip8 *chip8, const char *filename): 
	open the file with fopen in binary read mode ("rb") 
	if file is NULL: 
		print error and exit 
	read the file bytes into memory starting at 0x200 
		fread(&chip8->memory[0x200], 1, MAX_SIZE, file) 
	close the file with fclose
	
// ───────────────────────────────────── 
// FUNCTION 3: chip8_cycle 
// ───────────────────────────────────── 
void chip8_cycle(Chip8 *chip8): 
	// FETCH 
	opcode = (chip8->memory[PC] << 8) | chip8->memory[PC + 1] 
	PC += 2 
	
	// extract common variables upfront 
	x = (opcode & 0x0F00) >> 8 
	y = (opcode & 0x00F0) >> 4 
	nn = (opcode & 0x00FF) 
	n = (opcode & 0x000F) 
	
	// DECODE AND EXECUTE 
	switch (opcode & 0xF000): 
	
		case 0x0000: 
			if opcode == 0x00E0: 
				clear chip8->display with memset 
				set draw_flag = 1 
			if opcode == 0x00EE: 
				SP-- 
				PC = stack[SP] 
		
		case 0x1000: 
			PC = opcode & 0x0FFF 
			
		case 0x2000: 
			stack[SP] = PC 
			SP++ 
			PC = opcode & 0x0FFF 
		
		case 0x3000: 
			if V[x] == nn: PC += 2 
			
		case 0x4000: 
			if V[x] != nn: PC += 2 
			
		case 0x5000: 
			if V[x] == V[y]: PC += 2 
		
		case 0x6000: 
			V[x] = nn 
		
		case 0x7000: 
			V[x] += nn 
			
		case 0x8000: 
			switch (opcode & 0x000F): 
				case 0: V[x] = V[y] 
				case 1: V[x] |= V[y] 
				case 2: V[x] &= V[y] 
				case 3: V[x] ^= V[y] 
				case 4: 
					result = V[x] + V[y] 
					V[0xF] = (result > 255) ? 1 : 0 
					V[x] = result & 0xFF 
				case 5: 
					V[0xF] = (V[x] >= V[y]) ? 1 : 0 
					V[x] -= V[y] 
				case 6: 
					V[0xF] = V[x] & 0x1 
					V[x] >>= 1 
				case 7: 
					V[0xF] = (V[y] >= V[x]) ? 1 : 0 
					V[x] = V[y] - V[x] 
				case 0xE: 
					V[0xF] = (V[x] >> 7) & 0x1 
					V[x] <<= 1 
				
				case 0x9000: 
					if V[x] != V[y]: 
						PC += 2 
				
				case 0xA000: 
					I = opcode & 0x0FFF 
				
				case 0xB000: 
					PC = (opcode & 0x0FFF) + V[0] 
				
				case 0xC000: V[x] = (rand() % 256) & nn 
				
				case 0xD000:
					// DRAW SPRITE 
					x_coord = V[x] 
					y_coord = V[y] 
					height = n 
					V[0xF] = 0 
					for row = 0 to height-1: 
						sprite_byte = memory[I + row] 
						for col = 0 to 7: 
							if (sprite_byte & (0x80 >> col)) is set: 
								pixel_index = (y_coord + row) * 64 + (x_coord + col) 
								if display[pixel_index] == 1: 
									V[0xF] = 1 
								display[pixel_index] ^= 1 
					draw_flag = 1 
				
				case 0xE000: 
					if (nn == 0x9E): 
						if keypad[V[x]] == 1: PC += 2 
						if (nn == 0xA1): if keypad[V[x]] == 0: PC += 2 
				
				case 0xF000: 
					switch (nn): 
						case 0x07: V[x] = delay_timer 
						case 0x0A: 
							// blocking key wait 
							if no key is pressed: 
								PC -= 2 // re-execute this instruction next cycle 
						case 0x15: delay_timer = V[x] 
						case 0x18: sound_timer = V[x] 
						case 0x1E: I += V[x] 
						case 0x29: I = V[x] * 5 
						case 0x33: 
							memory[I] = V[x] / 100 
							memory[I+1] = (V[x] / 10) % 10 
							memory[I+2] = V[x] % 10 
						case 0x55: 
							for i = 0 to x: 
								memory[I + i] = V[i] 
						case 0x65: 
							for i = 0 to x: 
								V[i] = memory[I + i]
```

### main.c
```
#include "chip8.h"
#include <SDL2/SDL.h>

#define SCALE 10          // each chip8 pixel = 10x10 on screen
#define WINDOW_W 640      // 64 * 10
#define WINDOW_H 320      // 32 * 10
#define CYCLES_PER_FRAME 10

int main(int argc, char *argv[]):

    // argv[1] is the ROM path passed from terminal
    // like: ./chip8 roms/pong.ch8
    if argc < 2:
        print "usage: ./chip8 <rom path>"
        exit

    // SDL SETUP
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)
    create window  (640 x 320)
    create renderer

    // CHIP8 SETUP
    Chip8 chip8
    chip8_init(&chip8)
    chip8_load_rom(&chip8, argv[1])

    // KEYBOARD MAP
    // maps SDL keycodes to chip8 keypad indexes
    // CHIP8: 0 1 2 3 4 5 6 7 8 9 A B C D E F
    // KB:    X 1 2 3 Q W E A S D Z C 4 R F V

    // MAIN LOOP
    int running = 1
    while running:

        // HANDLE INPUT
        while SDL_PollEvent(&event):
            if event == SDL_QUIT: running = 0
            if event == SDL_KEYDOWN:
                set corresponding keypad[x] = 1
            if event == SDL_KEYUP:
                set corresponding keypad[x] = 0

        // RUN CPU CYCLES
        for i = 0 to CYCLES_PER_FRAME:
            chip8_cycle(&chip8)

        // UPDATE TIMERS (60Hz)
        if chip8.delay_timer > 0: chip8.delay_timer--
        if chip8.sound_timer > 0:
            // play beep sound
            chip8.sound_timer--

        // RENDER IF NEEDED
        if chip8.draw_flag:
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255)  // black background
            SDL_RenderClear(renderer)
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255)  // white pixels
            for each pixel in chip8.display[64*32]:
                if pixel == 1:
                    draw SDL_Rect at (col * SCALE, row * SCALE, SCALE, SCALE)
            SDL_RenderPresent(renderer)
            chip8.draw_flag = 0

    // CLEANUP
    SDL_DestroyRenderer(renderer)
    SDL_DestroyWindow(window)
    SDL_Quit()
    return 0
```

---

## Keyboard Mapping

CHIP-8 has a 16-key hexadecimal keypad mapped to the keyboard as follows:

```
CHIP-8 pad     Keyboard
1  2  3  C     1  2  3  4
4  5  6  D     Q  W  E  R
7  8  9  E     A  S  D  F
A  0  B  F     Z  X  C  V
```

| CHIP-8 | Keyboard || CHIP-8 | Keyboard |
|--------|----------|-|--------|----------|
| 0 | X || 8 | S |
| 1 | 1 || 9 | D |
| 2 | 2 || A | Z |
| 3 | 3 || B | C |
| 4 | Q || C | 4 |
| 5 | W || D | R |
| 6 | E || E | F |
| 7 | A || F | V |

**Escape** — pause / unpause

---

## ROMs

Place `.ch8` ROM files in the `roms/` directory. ROMs to try:

| ROM | File | Notes |
|-----|------|-------|
| Space Invaders | SI.ch8 | Press 1 to start |
| Pong | pong.ch8 | Classic two player |
| IBM Logo | IBM.ch8 | Display test |
| Maze | Maze.ch8 | |

A full ROM pack is available at: https://github.com/kripod/chip8-roms

---

## References

- [Cowgod's CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [Tobias V. Langhoff's CHIP-8 Guide](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)
