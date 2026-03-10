# chip-8-emulator
A CHIP-8 emulator written in C with SDL2, implementing the full fetch-decode-execute cycle and opcode instruction set

## Build
```bash
make
```

## Run
```bash
./chip8 roms/<romname>.ch8
```

## Controls

| CHIP-8 | Key || CHIP-8 | Key |
|--------|-----|-|--------|-----|
| 0 | X || 8 | S |
| 1 | 1 || 9 | D |
| 2 | 2 || A | Z |
| 3 | 3 || B | C |
| 4 | Q || C | 4 |
| 5 | W || D | R |
| 6 | E || E | F |
| 7 | A || F | V |

**Escape** — pause / unpause

## ROMs

Place `.ch8` files in the `roms/` folder.
Get ROMs at: https://github.com/kripod/chip8-roms
