#ifndef INSTRUCTION_H
#define INSTRUCTION_H

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
 * W (sign ext word)
 *   00 : byte of data
 *   01 : word of data
 *   11 : sign extended byte
 *
 * R first 2 bits (mod) :
 *   00 : disp = 0 (except if r/m = 110, then EA = disph, displ)
 *   01 : disp = displ sign extended to word
 *   10 : disp = disph, displ
 *   11 : r/m is treated as a reg field
 *
 * R last 3 bits (r/m) :
 *   000 : EA = (BX) + (SI) + DISP
 *   001 : EA = (BX) + (DI) + DISP
 *   010 : EA = (BP) + (SI) + DISP
 *   011 : EA = (BP) + (DI) + DISP
 *   100 : EA =        (SI) + DISP
 *   101 : EA =        (DI) + DISP
 *   110 : EA = (BP)        + DISP (except if mod = 00, then EA = disph, displ)
 *   111 : EA = (BX)        + DISP
 *
 * c (count)
 *   0 : count = 1
 *   1 : count = CL
 *
 * x (unused)
 *
 * z (string primitives compare with ZF FLAG)
 *   0 : rep while e/z, terminates on ZF = 0
 *   1 : rep while ne/nz, terminates on ZF = 1
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
 * s (segment reg)
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
 * P (displacement, byte or word)
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
 *  | |   ****ODIT SZ*A*P*C
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

#define NUM_OF_INSTRUCT_TYPES 132
#define NUM_OF_PRINTFMT_FIELDS 11
#define INSTR_PRINTFMT_LEN 33
#define INSTR_CODEFMT_LEN 33
#define MAX_INSTRUCT_LEN 15 // 15 bytes max for x86

/**
 * @brief An instruction type
 * @param printFormat char[INSTR_PRINTFMT_LEN]
 * @param codeFormat char[INSTR_CODEFMT_LEN]
 */
typedef struct InstructionTypeStruct {
    char printFormat[INSTR_PRINTFMT_LEN];
    char codeFormat[INSTR_CODEFMT_LEN];
} InstructionType;

/**
 * @brief An instruction
 * @param type struct InstructionType*
 * @param data char*
 * @param length unsigned int
 *
 */
typedef struct InstructionStruct {
    const InstructionType *type;
    char *data;
    unsigned int length;
    short unsigned int partsLengths[MAX_INSTRUCT_LEN];
} Instruction;

/**
 * @brief Get the name of the segment register
 *
 * @param s int
 * @return char*
 */
char *getSegPrintStr(int s);

/**
 * @brief Get the name of the register
 *
 * @param r int
 * @param w int
 * @return char*
 */
char *getRegPrintStr(int r, int w);

/**
 * @brief Get the name of the register or memory position
 *
 * @param m int
 * @param R int
 * @return char*
 */
char *getRegMemPrintStr(int m, int R);

extern const InstructionType instructionTypes[];
extern const char printFormatFields[];

#endif