#ifndef OPCODE_H
#define OPCODE_H

/*
 * --- FIELDS ---
 *
 * d (direction)
 *   0 : from reg into r/m
 *   1 : from r/m into reg
 *
 * w (word)
 *   0 : byte instruction
 *   1 : word instruction
 *
 * s (sign ext)
 *   0 : word of data
 *   1 : sign extended byte
 *
 * m (mod)
 *   00 : disp = 0 (except if r/m = 110, then EA = disph, displ)
 *   01 : disp = displ sign extended to word
 *   10 : disp = disph, displ
 *   11 : r/m is treated as a reg field
 *
 * R (r/m)
 *   000 : EA = (BX) + (SI) + DISP
 *   001 : EA = (BX) + (DI) + DISP
 *   010 : EA = (BP) + (SI) + DISP
 *   011 : EA = (BP) + (DI) + DISP
 *   100 : EA =        (SI) + DISP
 *   101 : EA =        (DI) + DISP
 *   110 : EA = (BP)        + DISP (except if mod = 00, then EA = disph, displ)
 *   111 : EA = (BX)        + DISP
 *
 * v (count)
 *   0 : count = 1
 *   1 : count = CL
 *
 * x (unused)
 *
 * z (string primitives compare with ZF FLAG)
 *
 * r (reg)
 *   000 : AX/AL
 *   001 : CX/CL
 *   010 : DX/DL
 *   011 : BX/BL
 *   100 : SP/AH
 *   101 : BP/CH
 *   110 : SI/DH
 *   111 : DI/BH
 *     w = 1/0
 *
 * S (segment reg)
 *   00 : ES
 *   01 : CS
 *   10 : SS
 *   11 : DS
 *
 * D (data, byte or word depending on w)
 *
 * p (port, byte)
 *
 * i (interrupt type, byte)
 *
 * P (displacement)
 *
 * o (offset or segment, word)
 *
 * --- REGISTERS ---
 *
 *       +--------+--------+
 *    AX |   AH   |   AL   | accumulator
 *       +--------+--------+
 *    BX |   BH   |   BL   | Base
 *       +--------+--------+
 *    CX |   CH   |   CL   | Count
 *       +--------+--------+
 *    DX |   DH   |   DL   | Data
 *       +--------+--------+
 *
 *       +-----------------+
 *  +---<|       SP        | Stack Pointer
 *  |    +-----------------+
 *  |    |       BP        | Base Pointer
 *  |    +-----------------+
 *  |    |       SI        | Source Index
 *  |    +-----------------+
 *  |    |       DI        | Destination Index
 *  |    +-----------------+
 *  |
 *  |    +-----------------+
 *  | +-<|       IP        | Instruction Pointer
 *  | |  +--------+--------+
 *  | |  | FLAGSH | FLAGSL | Status Flags
 *  | |  +--------+--------+
 *  | |   ____ODIT SZ_A_P_C
 *  | |
 *  | |  +-----------------+
 *  | +->|       CS        | Code segment
 *  |    +-----------------+
 *  |    |       DS        | Data segment
 *  |    +-----------------+
 *  +--->|       SS        | Stack segment
 *       +-----------------+
 *       |       ES        | Extra segment
 *       +-----------------+
 *
 * --- FLAGS ---
 *
 * OF : overflow flag (result was overflowing)
 * DF : direction flag 1=up 0=down
 * IF : interrupt flag (interrupts are enabled)
 * TF : trap flag (single step for debug)
 * SF : sign flag (result was negative)
 * ZF : zero flag (result was zero)
 * AF : aux carry flag (carry from low to high 4 bits or borrow from high to low 4 bits)
 * PF : parity flag (parity even)
 * CF : carry flag
 *
 */

#define NUM_OF_INSTRUCT_TYPES 171
#define MAX_INSTRUCT_LEN 4
#define INSTR_FORMAT_LEN 33
#define INSTR_OPCODE_LEN 33

/// @brief An instruction type
typedef struct InstructionType {
    char format[INSTR_FORMAT_LEN];
    char opcode[INSTR_OPCODE_LEN];
} InstructionType;

/// @brief Get the name of the segment register
char *getSeg(int s);

/// @brief Get the name of the register
char *getReg(int r, char w);

/// @brief Get the name of the register or memory position
char *getRegMem(int m, int R);

extern const InstructionType instructionTypes[];

#endif