#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct HeaderStruct {
short magic;
char flags;
char cpu;
char hdrlen;
int textlen;
int datalen;
int bsslen;
int entrylen;
int totallen;
int symslen;
} Header;

typedef struct InstructionStruct {
char** type;
long data;
} Instruction;

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

// MOV Move Register/Memory to/from Register
// 1000 10 d w mo reg r/m
char* mov_rm_r = "100010dwmmrrrRRR";

// MOV Move Immediate to Register/Memory
// 1100 011 w mo 000 r/m data
char* mov_i_rm = "11000110mm000RRRDDDDDDDD";
char* mov_i_rm_w = "11000111mm000RRRDDDDDDDDDDDDDDDD";

// MOV Move Immediate to Register
// 1011 w reg data
char* mov_i_r = "10110rrrDDDDDDDD";
char* mov_i_r_w = "10111rrrDDDDDDDDDDDDDDDD";

// MOV Move Memory to Accumulator
// 1010 000 w addr (lo-hi)
char* mov_m_a = "1010000waaaaaaaaaaaaaaaa";

// MOV Move Accumulator to Memory
// 1010 001 w addr (lo-hi)
char* mov_a_m = "1010001waaaaaaaaaaaaaaaa";

// MOV Move Register/Memory to Segment Register
// 1000 1110 mo 0 reg r/m
char* mov_rm_s = "10001110mm0SSRRR";

// MOV Move Segment Register to Register/Memory
// 1000 1100 mo 0 reg r/m
char* mov_s_rm = "10001100mm0SSRRR";

// PUSH Push Register/Memory
// 1111 1111 mo 110 r/m
char* push_rm = "11111111mm110RRR";

// PUSH Push Register
// 0101 0 reg
char* push_r = "01010rrr";

// PUSH Push Segment Register
// 000 reg 110
char* push_s = "000SS110"

// POP Pop Register/Memory
// 1000 1111 mo 000 r/m
char* pop_rm = "10001111mm000RRR";

// POP Pop Register
// 0101 1 reg
char* pop_r = "01011rrr";

// POP Pop Segment Register
// 000 reg 111
char* pop_s = "000SS111";

// XCHG Exchange Register/Memory with Register
// 1000 011 w mo reg r/m
char* xchg_rm_r = "1000011wmmrrrRRR";

// XCHG Exchange Register with Accumulator
// 1001 0 reg
char* xchg_r_a = "10010rrr";

// IN Input from Fixed Port
// 1110 010 w port
char* in_fixp = "1110010wpppppppp";

// IN Input from Variable Port
// 1110 110 w
char* in_varp = "1110110w";

// OUT Output to Fixed Port
// 1110 011 w port
char* out_fixp = "1110011wpppppppp";

// OUT Output to Variable Port
// 1110 111 w
char* out_varp = "1110111w";

// XLAT Translate Byte to AL
// 1101 0111
char* xlat = "11010111";

// LEA Load EA to Register
// 1000 1101 mo reg r/m
char* lea = "10001101mmrrrRRR";

// LDS Load Pointer to DS
// 1100 0101 mo reg r/m
char* lds = "11000101mmrrrRRR";

// LES Load Pointer to ES
// 1100 0100 mo reg r/m
char* les = "11000100mmrrrRRR";

// LAHF Load AH with Flags
// 1001 1111
char* lahf = "10011111";

// SAHF Store AH into Flags
// 1001 1110
char* sahf = "10011110";

// PUSHF Push Flags
// 1001 1100
char* pushf = "10011100";

// POPF Pop Flags
// 1001 1101
char* popf = "10011101";

// ADD Add Reg/Memory with Register to Either
// 0000 00 d w mo reg r/m
char* add_rm_r = "000000dwmmrrrRRR";

// ADD Add Immediate to Register/Memory
// 1000 00 s w mo 000 r/m data
char* add_i_rm = "10000000mm000RRRDDDDDDDD";
char* add_i_rm_w = "10000001mm000RRRDDDDDDDDDDDDDDDD";
char* add_i_rm_sw = "10000011mm000RRRDDDDDDDD";

// ADD Add Immediate to Accumulator
// 0000 010 w data
char* add_i_a = "00000100DDDDDDDD";

// ADD Add Immediate to Accumulator
// 0000 010 w data (w)
char* add_i_a_w = "00000101DDDDDDDDDDDDDDDD";

// ADC Add with Carry Reg./Memory with Register to Either
// 0001 00 d w mo reg r/m
char* adc_rm_r = "000100dwmmrrrRRR";

// ADC Add with Carry Immediate to Register/Memory
// 1000 00 s w mo 010 r/m data
char* adc_i_rm = "100000swmm010RRRDDDDDDDD";
char* adc_i_rm_w = "10000001mm010RRRDDDDDDDDDDDDDDDD";
char* adc_i_rm_sw = "10000011mm010RRRDDDDDDDD";

// ADC Add with Carry Immediate to Accumulator
// 0001 010 w data
char* adc_i_a = "00010100DDDDDDDD";

// ADC Add with Carry Immediate to Accumulator
// 0001 010 w data (w)
char* adc_i_a = "00010101DDDDDDDDDDDDDDDD";

// INC Increment Register/Memory
// 1111 111 w mo 000 r/m
char* inc_rm = "1111111wmm000RRR";

// INC Increment Register
// 0100 0 reg
char* inc_r = "01000rrr";

// AAA ASCII Adjust for Add
// 0011 0111
char* aaa = "00110111";

// BAA Decimal Adjust for Add
// 0010 0111
char* baa = "00100111";

// SUB Subtract Reg./Memory and Register to Either
// 0010 10 d w mo reg r/m
char* sub_rm_r = "001010dwmmrrrRRR";

// SUB Subtract Immediate from Register/Memory
// 1000 00 s w mo 101 r/m data (sw)
char* sub_i_rm = "10000000mm101RRRDDDDDDDD";
char* sub_i_rm_w = "10000001mm101RRRDDDDDDDDDDDDDDDD";
char* sub_i_rm_sw = "10000011mm101RRRDDDDDDDD";

// SUB Subtract Immediate from Accumulator
// 0010 110 w data (w)
char* sub_i_a = "00101100DDDDDDDD";
char* sub_i_a_w = "00101101DDDDDDDDDDDDDDDD";

// SSB Subtract with Borrow Reg./Memory and Register to Either
// 0001 10 d w mo reg r/m
char* ssb_rm_r = "000110dwmmrrrRRR";

// SSB Subtract with Borrow Immediate from Register/Memory
// 1000 00 s w mo 011 r/m data (sw)
char* ssb_i_rm = "10000000mm011RRRDDDDDDDD";
char* ssb_i_rm_w = "10000001mm011RRRDDDDDDDDDDDDDDDD";
char* ssb_i_rm_sw = "10000011mm011RRRDDDDDDDD";

// SSB Subtract with Borrow Immediate from Accumulator
// 0001 11 w data (w)
char* ssb_i_a = "00011100DDDDDDDD";
char* ssb_i_a_w = "00011101DDDDDDDDDDDDDDDD";

// DEC Decrement Register/memory
// 1111 111 w mo 001 r/m
char* dec_rm = "1111111wmm001RRR";

// DEC Decrement Register
// 0100 1 reg
char* dec_r = "01001rrr";

// NEG Change sign (NOT+1)
// 1111 011 w mo 011 r/m
char* neg = "1111011wmm011RRR";

// CMP Compare Register/Memory and Register
// 0011 10 d w mo reg r/m
char* cmp_rm_r = "001110dwmmrrrRRR";

// CMP Compare Immediate with Register/Memory
// 1000 00 s w mo 111 r/m data (sw)
char* cmp_i_rm = "10000000mm111RRRDDDDDDDD";
char* cmp_i_rm_w = "10000001mm111RRRDDDDDDDDDDDDDDDD";
char* cmp_i_rm_sw = "10000011mm111RRRDDDDDDDD";

// CMP Compare Immediate with Accumulator
// 0011 110 w data (w)
char* cmp_i_a = "00111100DDDDDDDD";
char* cmp_i_a_w = "00111101DDDDDDDDDDDDDDDD";

// AAS ASCII Adjust for Subtract
// 0011 1111
char* aas = "00111111";

// DAS Decimal Adjust for Subtract
// 0010 1111
char* das = "00101111";

// MUL Multiply (Unsigned)
// 1111 011 w mo 100 r/m
char* mul = "1111011wmm100RRR";

// IMUL Integer Multiply (Signed)
// 1111 011 w mo 101 r/m
char* imul = "1111011wmm101RRR";

// AAM ASCII Adjust for Multiply
// 1101 0100 0000 1010
char* aam = "1101010000001010";

// DIV Divide (Unsigned)
// 1111 011 w mo 110 r/m
char* div = "1111011wmm110RRR";

// IDIV Integer Divide (Signed)
// 1111 011 w mo 111 r/m
char* idiv = "1111011wmm111RRR";

// AAD ASCII Adjust for Divide
// 1101 0101 0000 1010
char* aad = "1101010100001010";

// CBW Convert Byte to Word
// 1001 1000
char* cbw = "10011000";

// CWD Convert Word to Double Word
// 1001 1001
char* cwd = "10011001";

// NOT Invert
// 1111 011 w mo 010 r/m
char* not = "1111011wmm010RRR";

// SHL/SAL Shift Logical/Arithmetic Left
// 1101 00 v w mo 100 r/m
char* shl = "110100vwmm100RRR";
char* sal = shl;

// SHR Shift Logical Right
// 1101 00 v w mo 101 r/m
char* shr = "110100vwmm101RRR";

// SAR Shift Arithmetic Right
// 1101 00 v w mo 111 r/m
char* sar = "110100vwmm111RRR";

// ROL Rotate Left
// 1101 00 v w mo 000 r/m
char* rol = "110100vwmm000RRR";

// ROR Rotate Right
// 1101 00 v w mo 001 r/m
char* ror = "110100vwmm001RRR";

// RCL Rotate Through Carry Flag Left
// 1101 00 v w mo 010 r/m
char* rcl = "110100vwmm010RRR";

// RCR Rotate Through Carry Right
// 1101 00 v w mo 011 r/m
char* rcr = "110100vwmm011RRR";

// AND And Reg./Memory and Register to Either
// 0010 00 d w mo reg r/m
char* and_rm_r = "001000dwmmrrrRRR";

// AND And Immediate to Register/Memory
// 1000 000 w mo 100 r/m data (w)
char* and_i_rm = "10000000mm100RRRDDDDDDDD";
char* and_i_rm_w = "10000001mm100RRRDDDDDDDDDDDDDDDD";

// AND And Immediate to Accumulator
// 0010 010 w data (w)
char* and_i_a = "00100100DDDDDDDD";
char* and_i_a_w = "00100101DDDDDDDDDDDDDDDD";

// TEST And Function to Flags, No Result Register/Memory and Register
// 1000 010 w mo reg r/m
char* test_rm_r = "1000010wmmrrrRRR";

// TEST And Function to Flags, No Result Immediate Data and Register/Memory
// 1111 011 w mo 000 r/m data (w)
char* test_i_rm = "11110110mm000RRRDDDDDDDD";
char* test_i_rm_w = "11110111mm000RRRDDDDDDDDDDDDDDDD";

// TEST And Function to Flags, No Result Immediate Data and Accumulator
// 1010 100 w data (w)
char* test_i_a = "10101000DDDDDDDD";
char* test_i_a_w = "10101001DDDDDDDDDDDDDDDD";

// OR Or Reg./Memory and Register to Either
// 0000 10 d w mo reg r/m
char* or_rm_r = "000010dwmmrrrRRR";

// OR Or Immediate to Register/Memory
// 1000 000 w mo 001 r/m data (w)
char* or_i_r = "10000000mm001RRRDDDDDDDD";
char* or_i_r = "10000001mm001RRRDDDDDDDDDDDDDDDD";

// OR Or Immediate to Accumulator
// 0000 110 w data (w)
char* or_i_a = "00001100DDDDDDDD";
char* or_i_a = "00001101DDDDDDDDDDDDDDDD";

// XOR Exclusive or Reg./Memory and Register to Either
// 0011 00 d w mo reg r/m
char* xor_rm_r = "001100dwmmrrrRRR";

// XOR Exclusive or Immediate to Register/Memory
// 1000 000 w mo 110 r/m data (w)
char* xor_i_rm = "10000000mm110RRRDDDDDDDD";
char* xor_i_rm_w = "10000001mm110RRRDDDDDDDDDDDDDDDD";

// XOR Exclusive or Immediate to Accumulator
// 0011 010 w data (w)
char* xor_i_a = "00110100DDDDDDDD";
char* xor_i_a_w = "00110101DDDDDDDDDDDDDDDD";

// REP Repeat
// 1111 001 z
char* rep = "1111001z";

// MOVS Move Byte/Word
// 1010 010 w
char* movs = "1010010w";

// CMPS Compare Byte/Word
// 1010 011 w
char* cmps = "1010011w";

// SCAS Scan Byte/Word
// 1010 111 w
char* scas = "1010111w";

// LODS Load Byte/Word to AL/AX
// 1010 110w
char* lods = "1010110w";

// STOS Store Byte/Word from AL/AX
// 1010 101w
char* stos = "1010101w";

// CALL Call Direct within Segment
// 1110 1000 disp (lo-hi)
char* call_near = "11101000PPPPPPPPPPPPPPPP";

// CALL Call Indirect within Segment
// 1111 1111 mo 010 r/m
char* aaaaaaaaa = "11111111mm010RRR";

// CALL Call Direct Intersegment
// 1001 1010 offset (lo-hi) seg (lo-hi)
char* aaaaaaaaa = "10011010oooooooooooooooo";

// CALL Call Indirect Intersegment
// 1111 1111 mo 011 r/m
char* aaaaaaaaa = "11111111mm011RRR";

// JMP Unconditional Jump Direct within Segment
// 1110 1001 disp (lo-hi)
char* aaaaaaaaa = "11101001disp(lo-hi)";

// JMP Unconditional Jump Direct within Segment-Short
// 1110 1011 disp
char* aaaaaaaaa = "11101011disp";

// JMP Unconditional Jump Indirect within Segment
// 1111 1111 mo 100 r/m
char* aaaaaaaaa = "11111111mm100RRR";

// JMP Unconditional Jump Direct Intersegment
// 1110 1010 offset (lo-hi) seg (lo-hi)
char* aaaaaaaaa = "11101010offset(lo-hi)seg(lo-hi)";

// JMP Unconditional Jump Indirect Intersegment
// 1111 1111 mo 101 r/m
char* aaaaaaaaa = "11111111mm101RRR";

// RET Return from CALL Within Segment
// 1100 0011
char* ret_near = "11000011";

// RET Return from CALL Within Seg Adding Immed to SP
// 1100 0010 data-low data-high
char* aaaaaaaaa = "11000010data-lowdata-high";

// RET Return from CALL Intersegment
// 1100 1011
char* ret_far = "11001011";

// RET Return from CALL Intersegment Adding Immediate to SP
// 1100 1010 data-low data-high
char* aaaaaaaaa = "11001010data-lowdata-high";

// JE/JZ Jump on Equal/Zero
// 0111 0100 disp
char* aaaaaaaaa = "01110100disp";

// JL/JNGE Jump on Less
// 0111 1100 disp
char* aaaaaaaaa = "01111100disp";

// JLE/JNG Jump on Less or Equal
// 0111 1110 disp
char* aaaaaaaaa = "01111110disp";

// JB/JNAE Jump on Below
// 0111 0010 disp
char* aaaaaaaaa = "01110010disp";

// JBE/JNA Jump on Below or Equal
// 0111 0110 disp
char* aaaaaaaaa = "01110110disp";

// JP/JPE Jump on Parity/Parity Even
// 0111 1010 disp
char* aaaaaaaaa = "01111010disp";

// JO Jump on Overflow
// 0111 0000 disp
char* aaaaaaaaa = "01110000disp";

// JS Jump on Sign
// 0111 1000 disp
char* aaaaaaaaa = "01111000disp";

// JNE/JNZ Jump on Not Equal/Not Zero
// 0111 0101 disp
char* aaaaaaaaa = "01110101disp";

// JNL/JGE Jump on Greater or Equal
// 0111 1101 disp
char* aaaaaaaaa = "01111101disp";

// JNLE/JG Jump on Greater
// 0111 1111 disp
char* aaaaaaaaa = "01111111disp";

// JNB/JAE Jump on Above or Equal
// 0111 0011 disp
char* aaaaaaaaa = "01110011disp";

// JNBE/JA Jump on Above
// 0111 0111 disp
char* aaaaaaaaa = "01110111disp";

// JNP/JPO Jump on Not Par/Par Odd
// 0111 1011 disp
char* aaaaaaaaa = "01111011disp";

// JNO Jump on Not Overflow
// 0111 0001 disp
char* aaaaaaaaa = "01110001disp";

// JNS Jump on Not Sign
// 0111 1001 disp
char* jns = "01111001disp";

// LOOP Loop CX Times
// 1110 0010 disp
char* loop = "11100010disp";

// LOOPZ/LOOPE Loop While Zero/Equal
// 1110 0001 disp
char* loopz = "11100001disp";
char* loope = loopz;

// LOOPNZ/LOOPNE Loop While Not Zero/Equal
// 1110 0000 disp
char* loopnz = "11100000disp";
char* loopne = loopnz;

// JCXZ Jump on CX Zero
// 1110 0011 disp
char* jcxz = "11100011disp";

// INT Interrupt Type Specified
// 1100 1101 type
char* int_type = "11001101iiiiiiii";

// INT Interrupt Type 3
// 1100 1100
char* int_type3 = "11001100";

// INTO Interrupt on Overflow
// 1100 1110
char* into = "11001110";

// IRET Interrupt Return
// 1100 1111
char* iret = "11001111";

// CLC Clear Carry
// 1111 1000
char* clc = "11111000";

// CMC Complement Carry
// 1111 0101
char* cmc = "11110101";

// STC Set Carry
// 1111 1001
char* stc = "11111001";

// CLD Clear Direction
// 1111 1100
char* cld = "11111100";

// STD Set Direction
// 1111 1101
char* std = "11111101";

// CLI Clear Interrupt
// 1111 1010
char* cli = "11111010";

// STI Set Interrupt
// 1111 1011
char* sti = "11111011";

// HLT Halt
// 1111 0100
char* hlt = "11110100";

// WAIT Wait
// 1001 1011
char* wait = "10011011";

// ESC Escape (to External Device)
// 1101 1 xxx mo xxx r/m
char* esc = "11011xxxmmxxxRRR";

// LOCK Bus Lock Prefix
// 1111 0000
char* lock = "11110000";



int readHeader(FILE* file, Header* hdr)
{
fseek(file, 0, SEEK_SET);

hdr->magic = getc(file)<<8 | getc(file);
hdr->flags = getc(file);
hdr->cpu = getc(file);
hdr->hdrlen = getc(file);

if (hdr->magic != 0x0103) {
printf("bad magic number\n");
return 1;
}

char* buf = malloc(hdr->hdrlen);
fseek(file, 0, SEEK_SET);

for (int i = 0; i<hdr->hdrlen; ++i)
*(buf+i) = getc(file);

if (hdr->hdrlen >= 32) {
hdr->textlen = *((int*)buf+2);
hdr->datalen = *((int*)buf+3);
hdr->bsslen = *((int*)buf+4);
hdr->entrylen = *((int*)buf+5);
hdr->totallen = *((int*)buf+6);
hdr->symslen = *((int*)buf+7);
}

/*long test = ftell(file);
printf("%d\n", test);*/

free(buf);
return 0;
}

int printHeader(Header* hdr)
{
printf("magic:%04x\n", hdr->magic & 0xffff);
printf("flags:%02x\n", hdr->flags & 0xff);
printf("cpu:%02x\n", hdr->cpu & 0xff);
printf("hdrlen:%dB\n", hdr->hdrlen);

if (hdr->hdrlen >= 32) {
printf("textlen:%dB\n", hdr->textlen);
printf("datalen:%dB\n", hdr->datalen);
printf("bsslen:%dB\n", hdr->bsslen);
printf("entrylen:%dB\n", hdr->entrylen);
printf("totallen:%dB\n", hdr->totallen);
printf("symslen:%dB\n", hdr->symslen);
}

return 0;
}

int readText(Header* hdr)
{
char* text = malloc(hdr->textlen);
for (int i = 0;* i<hdr->textlen; ++i)
*(text+i) = getc(file);

char* buf = malloc(4);

for (int i = 0; i<textlen; ++i) {
//printf("%02x ", *(text+i) & 0xff);
printf("%08b\n", *(text+i) & 0xff);
}

for (int i = 0; i<textlen; ++i) {
//printf("%02x ", *(text+i) & 0xff);
printf("%08b\n", *(text+i) & 0xff);
}
printf("\n");*/
return 0;
}

int main(int argc, char** argv)
{
FILE* file;
int status;
//char* buffer;
//long filelen;

file = fopen(argv[1], "rb");

/*
fseek(file, 0, SEEK_END);
ftell(file);
fseek(file, 0, SEEK_SET);
*/

Header* hdr = malloc(sizeof(Header));
status = readHeader(file, hdr);
printHeader(hdr);

status = readText(hdr);

return 0;
}
