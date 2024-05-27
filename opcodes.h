/*
 * --- FIELDS ---
 *
 * d (direction)
 *   0 : from reg
 *   1 : to reg
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
 *    AX |   AH   |   AL   | Accumulator
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
 *  | +->|       CS        | Code Segment
 *  |    +-----------------+
 *  |    |       DS        | Data Segment
 *  |    +-----------------+
 *  +--->|       SS        | Stack Segment
 *       +-----------------+
 *       |       ES        | Extra Segment
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

char* getSeg(char* s);

char* getReg(char* r, char w);

typedef struct InstructionType_struct {
    char* format;
    char* opcode;
} InstructionType;

// MOV Move Register/Memory to/from Register
// 1000 10 d w mo reg r/m
char* mov_rm_r;

// MOV Move Immediate to Register/Memory
// 1100 011 w mo 000 r/m data
char* mov_i_rm;
char* mov_i_rm_w;

// MOV Move Immediate to Register
// 1011 w reg data
char* mov_i_r;
char* mov_i_r_w;

// MOV Move Memory to Accumulator
// 1010 000 w addr (lo-hi)
char* mov_m_a;

// MOV Move Accumulator to Memory
// 1010 001 w addr (lo-hi)
char* mov_a_m;

// MOV Move Register/Memory to Segment Register
// 1000 1110 mo 0 reg r/m
char* mov_rm_s;

// MOV Move Segment Register to Register/Memory
// 1000 1100 mo 0 reg r/m
char* mov_s_rm;

// PUSH Push Register/Memory
// 1111 1111 mo 110 r/m
char* push_rm;

// PUSH Push Register
// 0101 0 reg
char* push_r;

// PUSH Push Segment Register
// 000 reg 110
char* push_s;

// POP Pop Register/Memory
// 1000 1111 mo 000 r/m
char* pop_rm;

// POP Pop Register
// 0101 1 reg
char* pop_r;

// POP Pop Segment Register
// 000 reg 111
char* pop_s;

// XCHG Exchange Register/Memory with Register
// 1000 011 w mo reg r/m
char* xchg_rm_r;

// XCHG Exchange Register with Accumulator
// 1001 0 reg
char* xchg_r_a;

// IN Input from Fixed Port
// 1110 010 w port
char* in_fixp;

// IN Input from Variable Port
// 1110 110 w
char* in_varp;

// OUT Output to Fixed Port
// 1110 011 w port
char* out_fixp;

// OUT Output to Variable Port
// 1110 111 w
char* out_varp;

// XLAT Translate Byte to AL
// 1101 0111
char* xlat;

// LEA Load EA to Register
// 1000 1101 mo reg r/m
char* lea;

// LDS Load Pointer to DS
// 1100 0101 mo reg r/m
char* lds;

// LES Load Pointer to ES
// 1100 0100 mo reg r/m
char* les;

// LAHF Load AH with Flags
// 1001 1111
char* lahf;

// SAHF Store AH into Flags
// 1001 1110
char* sahf;

// PUSHF Push Flags
// 1001 1100
char* pushf;

// POPF Pop Flags
// 1001 1101
char* popf;

// ADD Add Reg/Memory with Register to Either
// 0000 00 d w mo reg r/m
char* add_rm_r;

// ADD Add Immediate to Register/Memory
// 1000 00 s w mo 000 r/m data
char* add_i_rm;
char* add_i_rm_w;
char* add_i_rm_sw;

// ADD Add Immediate to Accumulator
// 0000 010 w data
char* add_i_a;

// ADD Add Immediate to Accumulator
// 0000 010 w data (w)
char* add_i_a_w;

// ADC Add with Carry Reg./Memory with Register to Either
// 0001 00 d w mo reg r/m
char* adc_rm_r;

// ADC Add with Carry Immediate to Register/Memory
// 1000 00 s w mo 010 r/m data
char* adc_i_rm;
char* adc_i_rm_w;
char* adc_i_rm_sw;

// ADC Add with Carry Immediate to Accumulator
// 0001 010 w data
char* adc_i_a;

// ADC Add with Carry Immediate to Accumulator
// 0001 010 w data (w)
char* adc_i_a;

// INC Increment Register/Memory
// 1111 111 w mo 000 r/m
char* inc_rm;

// INC Increment Register
// 0100 0 reg
char* inc_r;

// AAA ASCII Adjust for Add
// 0011 0111
char* aaa;

// BAA Decimal Adjust for Add
// 0010 0111
char* baa;

// SUB Subtract Reg./Memory and Register to Either
// 0010 10 d w mo reg r/m
char* sub_rm_r;

// SUB Subtract Immediate from Register/Memory
// 1000 00 s w mo 101 r/m data (sw)
char* sub_i_rm;
char* sub_i_rm_w;
char* sub_i_rm_sw;

// SUB Subtract Immediate from Accumulator
// 0010 110 w data (w)
char* sub_i_a;
char* sub_i_a_w;

// SSB Subtract with Borrow Reg./Memory and Register to Either
// 0001 10 d w mo reg r/m
char* ssb_rm_r;

// SSB Subtract with Borrow Immediate from Register/Memory
// 1000 00 s w mo 011 r/m data (sw)
char* ssb_i_rm;
char* ssb_i_rm_w;
char* ssb_i_rm_sw;

// SSB Subtract with Borrow Immediate from Accumulator
// 0001 11 w data (w)
char* ssb_i_a;
char* ssb_i_a_w;

// DEC Decrement Register/memory
// 1111 111 w mo 001 r/m
char* dec_rm;

// DEC Decrement Register
// 0100 1 reg
char* dec_r;

// NEG Change sign (NOT+1)
// 1111 011 w mo 011 r/m
char* neg;

// CMP Compare Register/Memory and Register
// 0011 10 d w mo reg r/m
char* cmp_rm_r;

// CMP Compare Immediate with Register/Memory
// 1000 00 s w mo 111 r/m data (sw)
char* cmp_i_rm;
char* cmp_i_rm_w;
char* cmp_i_rm_sw;

// CMP Compare Immediate with Accumulator
// 0011 110 w data (w)
char* cmp_i_a;
char* cmp_i_a_w;

// AAS ASCII Adjust for Subtract
// 0011 1111
char* aas;

// DAS Decimal Adjust for Subtract
// 0010 1111
char* das;

// MUL Multiply (Unsigned)
// 1111 011 w mo 100 r/m
char* mul;

// IMUL Integer Multiply (Signed)
// 1111 011 w mo 101 r/m
char* imul;

// AAM ASCII Adjust for Multiply
// 1101 0100 0000 1010
char* aam;

// DIV Divide (Unsigned)
// 1111 011 w mo 110 r/m
char* div;

// IDIV Integer Divide (Signed)
// 1111 011 w mo 111 r/m
char* idiv;

// AAD ASCII Adjust for Divide
// 1101 0101 0000 1010
char* aad;

// CBW Convert Byte to Word
// 1001 1000
char* cbw;

// CWD Convert Word to Double Word
// 1001 1001
char* cwd;

// NOT Invert
// 1111 011 w mo 010 r/m
char* not;

// SHL/SAL Shift Logical/Arithmetic Left
// 1101 00 v w mo 100 r/m
char* shl;
char* sal;

// SHR Shift Logical Right
// 1101 00 v w mo 101 r/m
char* shr;

// SAR Shift Arithmetic Right
// 1101 00 v w mo 111 r/m
char* sar;

// ROL Rotate Left
// 1101 00 v w mo 000 r/m
char* rol;

// ROR Rotate Right
// 1101 00 v w mo 001 r/m
char* ror;

// RCL Rotate Through Carry Flag Left
// 1101 00 v w mo 010 r/m
char* rcl;

// RCR Rotate Through Carry Right
// 1101 00 v w mo 011 r/m
char* rcr;

// AND And Reg./Memory and Register to Either
// 0010 00 d w mo reg r/m
char* and_rm_r;

// AND And Immediate to Register/Memory
// 1000 000 w mo 100 r/m data (w)
char* and_i_rm;
char* and_i_rm_w;

// AND And Immediate to Accumulator
// 0010 010 w data (w)
char* and_i_a;
char* and_i_a_w;

// TEST And Function to Flags, No Result Register/Memory and Register
// 1000 010 w mo reg r/m
char* test_rm_r;

// TEST And Function to Flags, No Result Immediate Data and Register/Memory
// 1111 011 w mo 000 r/m data (w)
char* test_i_rm;
char* test_i_rm_w;

// TEST And Function to Flags, No Result Immediate Data and Accumulator
// 1010 100 w data (w)
char* test_i_a;
char* test_i_a_w;

// OR Or Reg./Memory and Register to Either
// 0000 10 d w mo reg r/m
char* or_rm_r;

// OR Or Immediate to Register/Memory
// 1000 000 w mo 001 r/m data (w)
char* or_i_r;
char* or_i_r;

// OR Or Immediate to Accumulator
// 0000 110 w data (w)
char* or_i_a;
char* or_i_a;

// XOR Exclusive or Reg./Memory and Register to Either
// 0011 00 d w mo reg r/m
char* xor_rm_r;

// XOR Exclusive or Immediate to Register/Memory
// 1000 000 w mo 110 r/m data (w)
char* xor_i_rm;
char* xor_i_rm_w;

// XOR Exclusive or Immediate to Accumulator
// 0011 010 w data (w)
char* xor_i_a;
char* xor_i_a_w;

// REP Repeat
// 1111 001 z
char* rep;

// MOVS Move Byte/Word
// 1010 010 w
char* movs;

// CMPS Compare Byte/Word
// 1010 011 w
char* cmps;

// SCAS Scan Byte/Word
// 1010 111 w
char* scas;

// LODS Load Byte/Word to AL/AX
// 1010 110w
char* lods;

// STOS Store Byte/Word from AL/AX
// 1010 101w
char* stos;

// CALL Call Direct within Segment
// 1110 1000 disp (lo-hi)
char* call_near_i;

// CALL Call Indirect within Segment
// 1111 1111 mo 010 r/m
char* call_near_r;

// CALL Call Direct Intersegment
// 1001 1010 (offset (lo-hi) or seg (lo-hi))
char* call_far_i;

// CALL Call Indirect Intersegment
// 1111 1111 mo 011 r/m
char* call_far_r;

// JMP Unconditional Jump Direct within Segment
// 1110 1001 disp (lo-hi)
char* jmp_near_i_w;

// JMP Unconditional Jump Direct within Segment-Short
// 1110 1011 disp
char* jmp_near_i;

// JMP Unconditional Jump Indirect within Segment
// 1111 1111 mo 100 r/m
char* jmp_near_r;

// JMP Unconditional Jump Direct Intersegment
// 1110 1010 offset (lo-hi) seg (lo-hi)
char* jmp_far_i;

// JMP Unconditional Jump Indirect Intersegment
// 1111 1111 mo 101 r/m
char* jmp_far_r;

// RET Return from CALL Within Segment
// 1100 0011
char* ret_near;

// RET Return from CALL Within Segment Adding Immediate to SP
// 1100 0010 data-low data-high
char* ret_near_i;

// RET Return from CALL Intersegment
// 1100 1011
char* ret_far;

// RET Return from CALL Intersegment Adding Immediate to SP
// 1100 1010 data-low data-high
char* ret_far_i;

// JE/JZ Jump on Equal/Zero
// 0111 0100 disp
char* je;
char* jz;

// JL/JNGE Jump on Less
// 0111 1100 disp
char* jl;
char* jnge;

// JLE/JNG Jump on Less or Equal
// 0111 1110 disp
char* jle;
char* jng;

// JB/JNAE Jump on Below
// 0111 0010 disp
char* jb;
char* jnae;

// JBE/JNA Jump on Below or Equal
// 0111 0110 disp
char* jbe;
char* jna;

// JP/JPE Jump on Parity/Parity Even
// 0111 1010 disp
char* jp;
char* jpe;

// JO Jump on Overflow
// 0111 0000 disp
char* jo;

// JS Jump on Sign
// 0111 1000 disp
char* js;

// JNE/JNZ Jump on Not Equal/Not Zero
// 0111 0101 disp
char* jne;
char* jnz;

// JNL/JGE Jump on Greater or Equal
// 0111 1101 disp
char* jge;
char* jnl;

// JNLE/JG Jump on Greater
// 0111 1111 disp
char* jg;
char* jnle;

// JNB/JAE Jump on Above or Equal
// 0111 0011 disp
char* jae;
char* jnb;

// JNBE/JA Jump on Above
// 0111 0111 disp
char* ja;
char* jnbe;

// JNP/JPO Jump on Not Par/Par Odd
// 0111 1011 disp
char* jnp;
char* jpo;

// JNO Jump on Not Overflow
// 0111 0001 disp
char* jno;

// JNS Jump on Not Sign
// 0111 1001 disp
char* jns;

// LOOP Loop CX Times
// 1110 0010 disp
char* loop;

// LOOPZ/LOOPE Loop While Zero/Equal
// 1110 0001 disp
char* loopz;
char* loope;

// LOOPNZ/LOOPNE Loop While Not Zero/Equal
// 1110 0000 disp
char* loopnz;
char* loopne;

// JCXZ Jump on CX Zero
// 1110 0011 disp
char* jcxz;

// INT Interrupt Type Specified
// 1100 1101 type
char* int_typs;

// INT Interrupt Type 3
// 1100 1100
char* int_typ3;

// INTO Interrupt on Overflow
// 1100 1110
char* into;

// IRET Interrupt Return
// 1100 1111
char* iret;

// CLC Clear Carry
// 1111 1000
char* clc;

// CMC Complement Carry
// 1111 0101
char* cmc;

// STC Set Carry
// 1111 1001
char* stc;

// CLD Clear Direction
// 1111 1100
char* cld;

// STD Set Direction
// 1111 1101
char* std;

// CLI Clear Interrupt
// 1111 1010
char* cli;

// STI Set Interrupt
// 1111 1011
char* sti;

// HLT Halt
// 1111 0100
char* hlt;

// WAIT Wait
// 1001 1011
char* wait;

// ESC Escape (to External Device)
// 1101 1 xxx mo xxx r/m
char* esc;

// LOCK Bus Lock Prefix
// 1111 0000
char* lock;


char** instructionTypes;
