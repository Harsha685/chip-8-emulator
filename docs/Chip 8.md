# Chip8:
- Chip 8 is an interpreter from the mid 1970s. Essentially it is a virtual machine.
- developed by Joseph Weisbecker on his 1802 microprocessor
- CHIP-8 was designed to be easy to program for and to use less memory than other programming languages like BASIC(**Beginner's All-purpose Symbolic Instruction Code**)
- During the 1970s and 1980s, CHIP-8 users shared CHIP-8 programs, but also changes and extensions to the CHIP-8 interpreter, like in the VIPER magazine for COSMAC VIP. These extensions included CHIP-10 and Hi-Res CHIP-8, which introduced a higher resolution than the standard 64x32, and CHIP-8C and CHIP-8X, which extended the monochrome display capabilities to support limited color, among other features. These extensions were mostly backwards compatible, as they were based on the original interpreter
- Backwards compatible: The ability of a newer technology (like a chip, software, or console) to successfully use the data or programs created for an earlier version of that technology. i.e. the Existing Code Ran Fine and included all the original opcodes (commands) of CHIP-8

### Specifications of Chip8:
### Memory:
- CHIP-8 was most commonly implemented on 4K systems. These machines had 4096 (0x1000) memory locations, and each memory location could hold 8 bits (1 byte) of memory.
- The CHIP-8 interpreter itself occupies the first 512 bytes of the memory space on these machines (0x200).  For this reason, most programs written for the original system begin at memory location 512 (0x200) and do not access any of the memory below the location 512 (0x200)
- The uppermost 256 bytes (0xF00-0xFFF) are reserved for display refresh, and the 96 bytes below that (0xEA0-0xEFF) were reserved for the call stack, internal use, and other variables.
- **Total Remaining:** **3,232 bytes**, can be used for manipulating the data 
	-  ​**​0x000 - 0x1FF (0 - 511 bytes):​**​ Historically reserved for the CHIP-8 interpreter. Programs should avoid using this region. In modern implementations, this area is often utilized for storing font data, as the interpreter runs natively outside the 4K memory space.
	- ​**​0x200 - 0xFFF (512 - 4095 bytes):​**​ This is the primary program and data space for CHIP-8 applications. Most standard CHIP-8 programs begin execution at 0x200.
	- ​**​0x600 (1536 bytes):​**​ While most programs start at 0x200, programs specifically designed for the ETI 660 computer often begin at 0x600. However, for broader compatibility, starting at 0x200 is generally preferred.
	- ​**​0xEA0 - 0xEFF (3744 - 3839 bytes):​**​ Reserved for the call stack, internal use, and other variables.
	- ​**​0xF00 - 0xFFF (3840 - 4095 bytes):​**​ Reserved for display refresh.
### Registers:
- Chip-8 has 16 general purpose 8-bit registers, usually referred to as Vx, where _x_ is a hexadecimal digit (0 through F). There is also a 16-bit register called I. This register is generally used to store memory addresses, so only the lowest (rightmost) 12 bits are usually used.
- The VF register should not be used by any program, as it is used as a flag by some instructions.
- Chip-8 also has two special purpose 8-bit registers, for the delay and sound timers. When these registers are non-zero, they are automatically decremented at a rate of 60Hz                                                            
### The stack:
- The stack are called. The original RCA 1802 "RCA 1802") version allocated 48 bytes for up to 12 levels of nesting; 20 modern implementations usually have more.
### Timers:
CHIP-8 has two timers. They both count down at 60 hertz until they reach 0.
- **Delay timer**: This timer is intended to be used for timing the events of games. Its value can be set and read. It is active whenever the delay timer register (DT) is non-zero. This timer does nothing more than subtract 1 from the value of DT at a rate of 60Hz. When DT reaches 0, it deactivates.
- **Sound timer**: This timer is used for sound effects. When its value is nonzero, a beeping sound is made. Its value can only be set. The sound timer is active whenever the sound timer register (ST) is non-zero. This timer also decrements at a rate of 60Hz, however, as long as ST's value is greater than zero, the Chip-8 buzzer will sound. When ST reaches zero, the sound timer deactivates. The sound timer is active whenever the sound timer register (ST) is non-zero. This timer also decrements at a rate of 60Hz, however, as long as ST's value is greater than zero, the Chip-8 buzzer will sound. When ST reaches zero, the sound timer deactivates.
### Graphics:
- important to the specification is the 64x32 pixel display that is associated with the Chip8. Each pixel only contains the information as to whether it is on or off.
- Graphics are drawn to the screen solely by drawing sprites, which are 8 pixels wide and may be from 1 to 15 pixels in height. Sprite pixels are XOR'd with corresponding screen pixels. In other words, sprite pixels that are set flip the color of the corresponding screen pixel, while unset sprite pixels do nothing.
- The carry flag (VF) is set to 1 if any screen pixels are flipped from set to unset when a sprite is drawn and set to 0 otherwise. This is used for collision detection.
### opcodes
- The original implementation of the Chip-8 language includes 36 different instructions, including math, graphics, and flow control functions.
- All instructions are 2 bytes long and are stored most-significant-byte first. In memory, the first byte of each instruction should be located at an even addresses. If a program includes sprite data, it should be padded so any instructions following it will be properly situated in RAM.
-  The opcodes are listed below, in hexadecimal and with the following symbols:
	- NNN: address
	- NN: 8-bit constant
	- N: 4-bit constant
	- X and Y: 4-bit register identifier
	- PC : Program Counter
	- I : 12bit register (For memory address) (Similar to void pointer);
	- VN: One of the 16 available variables. N may be 0 to F (hexadecimal);
- or:
	- _nnn_ or _addr_ - A 12-bit value, the lowest 12 bits of the instruction  
	- _n_ or _nibble_ - A 4-bit value, the lowest 4 bits of the instruction  
	- _x_ - A 4-bit value, the lower 4 bits of the high byte of the instruction  
	- _y_ - A 4-bit value, the upper 4 bits of the low byte of the instruction  
	- _kk_ or _byte_ - An 8-bit value, the lowest 8 bits of the instruction

| Opcode | Type    | C Pseudocode                  | Explanation                                                                                       |
| ------ | ------- | ----------------------------- | ------------------------------------------------------------------------------------------------- |
| 0NNN   | Call    | Calls RCA 1802 routine at NNN | Calls machine code routine (RCA 1802 for COSMAC VIP) at address NNN. Not necessary for most ROMs. |
| 00E0   | Display | disp_clear()                  | Clears the screen.                                                                                |
| 00EE   | Flow    | return;                       | Returns from a subroutine.                                                                        |
| 1NNN   | Flow    | goto NNN;                     | Jumps to address NNN.                                                                             |
| 2NNN   | Flow    | *(0xNNN)()                    | Calls subroutine at NNN.                                                                          |
| 3XNN   | Cond    | if (Vx == NN)                 | Skips next instruction if VX equals NN.                                                           |
| 4XNN   | Cond    | if (Vx != NN)                 | Skips next instruction if VX does not equal NN.                                                   |
| 5XY0   | Cond    | if (Vx == Vy)                 | Skips next instruction if VX equals VY.                                                           |
| 6XNN   | Const   | Vx = NN                       | Sets VX to NN.                                                                                    |
| 7XNN   | Const   | Vx += NN                      | Adds NN to VX (carry flag unchanged).                                                             |
| 8XY0   | Assign  | Vx = Vy                       | Sets VX to value of VY.                                                                           |
| 8XY1   | BitOp   | Vx \|= Vy                     | Bitwise OR between VX and VY.                                                                     |
| 8XY2   | BitOp   | Vx &= Vy                      | Bitwise AND between VX and VY.                                                                    |
| 8XY3   | BitOp   | Vx ^= Vy                      | Bitwise XOR between VX and VY.                                                                    |
| 8XY4   | Math    | Vx += Vy                      | Adds VY to VX. VF = 1 on overflow, else 0.                                                        |
| 8XY5   | Math    | Vx -= Vy                      | Subtracts VY from VX. VF = 1 if VX ≥ VY, else 0.                                                  |
| 8XY6   | BitOp   | Vx >>= 1                      | Shifts VX right by 1. VF = least significant bit before shift.                                    |
| 8XY7   | Math    | Vx = Vy - Vx                  | Sets VX to VY minus VX. VF = 1 if VY ≥ VX, else 0.                                                |
| 8XYE   | BitOp   | Vx <<= 1                      | Shifts VX left by 1. VF = most significant bit before shift.                                      |
| 9XY0   | Cond    | if (Vx != Vy)                 | Skips next instruction if VX does not equal VY.                                                   |
| ANNN   | MEM     | I = NNN                       | Sets I to address NNN.                                                                            |
| BNNN   | Flow    | PC = V0 + NNN                 | Jumps to address NNN plus V0.                                                                     |
| CXNN   | Rand    | Vx = rand() & NN              | Sets VX to random number AND NN.                                                                  |
| DXYN   | Display | draw(Vx, Vy, N)               | Draws 8×N sprite at (VX, VY). VF = 1 if pixel collision occurs.                                   |
| EX9E   | KeyOp   | if (key() == Vx)              | Skips next instruction if key in VX is pressed.                                                   |
| EXA1   | KeyOp   | if (key() != Vx)              | Skips next instruction if key in VX is not pressed.                                               |
| FX07   | Timer   | Vx = get_delay()              | Sets VX to delay timer value.                                                                     |
| FX0A   | KeyOp   | Vx = get_key()                | Waits for key press and stores it in VX (blocking).                                               |
| FX15   | Timer   | delay_timer(Vx)               | Sets delay timer to VX.                                                                           |
| FX18   | Sound   | sound_timer(Vx)               | Sets sound timer to VX.                                                                           |
| FX1E   | MEM     | I += Vx                       | Adds VX to I. VF not affected.                                                                    |
| FX29   | MEM     | I = sprite_addr[Vx]           | Sets I to sprite address for hex character in VX (0-F).                                           |
| FX33   | BCD     | Store BCD of Vx at I          | Stores hundreds at I, tens at I+1, ones at I+2.                                                   |
| FX55   | MEM     | reg_dump(Vx, &I)              | Stores V0 to VX in memory starting at I. I unchanged.                                             |
| FX65   | MEM     | reg_load(Vx, &I)              | Loads V0 to VX from memory starting at I. I unchanged.                                            |
### Keyboard:
- The computers which originally used the Chip-8 Language had a 16-key hexadecimal keypad with the following layout:
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

### CPU Cycle and Execution Timing Model of Chip 8:
- At its core, CHIP-8 follows the classical fetch–decode–execute cycle common to most processor architectures. Each cycle consists of three fundamental stages.

1. **Fetch**:During the fetch stage, the interpreter reads the value stored in the Program Counter (PC), which contains the memory address of the next instruction to be executed. Because each CHIP-8 instruction is exactly two bytes in length, the interpreter retrieves two consecutive bytes from memory at the address specified by the PC. These bytes are combined to form a single 16-bit opcode. After the opcode is fetched, the PC is incremented by two to reference the next sequential instruction in memory. This increment assumes normal sequential execution; however, certain instructions executed later in the cycle may override the PC to alter control flow.

2. **Decode**: In the decode stage, the fetched 16-bit opcode is analyzed to determine the operation it represents. Decoding involves extracting specific bit fields from the instruction, such as register identifiers, immediate values, or memory addresses. These fields define both the instruction category and its operands. At this stage, no changes are made to registers, memory, or system state. The purpose of decoding is solely to interpret the structure and meaning of the instruction so that it can be executed correctly in the following phase.

3. **Execute**: The execute phase serves as the functional core of the emulation loop. It translates the decoded opcode into concrete operations performed on the virtual hardware. This phase is typically implemented through a centralized dispatch mechanism, such as a structured conditional system or instruction table, that maps opcode patterns to their corresponding behaviors .Execution may involve a wide range of machine-level operations, including:
	- **Register Manipulation:** Loading immediate values into registers or performing arithmetic and logical operations.
	- **Graphics Rendering:** Drawing sprites to the display buffer using XOR-based pixel operations.
	- **Display Management:** Clearing the frame buffer to reset the visual state.
	- **Memory Interfacing:** Writing register values to memory, loading memory into registers, or updating the index register.
	- **Control Flow Modification:** Altering the Program Counter to implement jumps, subroutine calls, conditional skips, or returns.


