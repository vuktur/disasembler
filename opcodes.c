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

getSeg(char* s) {
    switch (s) {
        case "00" : return "ES";
        case "01" : return "CS";
        case "10" : return "SS";
        case "11" : return "DS";
        default : return "";
    }
}

getReg(char* r, char w) {
    switch (r) {
        case "000" : return w==0 ? "AX" : "AL";
        case "001" : return w==0 ? "CX" : "CL";
        case "010" : return w==0 ? "DX" : "DL";
        case "011" : return w==0 ? "BX" : "BL";
        case "100" : return w==0 ? "SP" : "AH";
        case "101" : return w==0 ? "BP" : "CH";
        case "110" : return w==0 ? "SI" : "DH";
        case "111" : return w==0 ? "DI" : "BH";
    }
}

// MOV Move Register/Memory to/from Register
// 1000 10 d w mo reg r/m
mov_rm_r = "100010dwmmrrrRRR";

// MOV Move Immediate to Register/Memory
// 1100 011 w mo 000 r/m data
mov_i_rm = "11000110mm000RRRDDDDDDDD";
mov_i_rm_w = "11000111mm000RRRDDDDDDDDDDDDDDDD";

// MOV Move Immediate to Register
// 1011 w reg data
mov_i_r = "10110rrrDDDDDDDD";
mov_i_r_w = "10111rrrDDDDDDDDDDDDDDDD";

// MOV Move Memory to Accumulator
// 1010 000 w addr (lo-hi)
mov_m_a = "1010000waaaaaaaaaaaaaaaa";

// MOV Move Accumulator to Memory
// 1010 001 w addr (lo-hi)
mov_a_m = "1010001waaaaaaaaaaaaaaaa";

// MOV Move Register/Memory to Segment Register
// 1000 1110 mo 0 reg r/m
mov_rm_s = "10001110mm0SSRRR";

// MOV Move Segment Register to Register/Memory
// 1000 1100 mo 0 reg r/m
mov_s_rm = "10001100mm0SSRRR";

// PUSH Push Register/Memory
// 1111 1111 mo 110 r/m
push_rm = "11111111mm110RRR";

// PUSH Push Register
// 0101 0 reg
push_r = "01010rrr";

// PUSH Push Segment Register
// 000 reg 110
push_s = "000SS110";

// POP Pop Register/Memory
// 1000 1111 mo 000 r/m
pop_rm = "10001111mm000RRR";

// POP Pop Register
// 0101 1 reg
pop_r = "01011rrr";

// POP Pop Segment Register
// 000 reg 111
pop_s = "000SS111";

// XCHG Exchange Register/Memory with Register
// 1000 011 w mo reg r/m
xchg_rm_r = "1000011wmmrrrRRR";

// XCHG Exchange Register with Accumulator
// 1001 0 reg
xchg_r_a = "10010rrr";

// IN Input from Fixed Port
// 1110 010 w port
in_fixp = "1110010wpppppppp";

// IN Input from Variable Port
// 1110 110 w
in_varp = "1110110w";

// OUT Output to Fixed Port
// 1110 011 w port
out_fixp = "1110011wpppppppp";

// OUT Output to Variable Port
// 1110 111 w
out_varp = "1110111w";

// XLAT Translate Byte to AL
// 1101 0111
xlat = "11010111";

// LEA Load EA to Register
// 1000 1101 mo reg r/m
lea = "10001101mmrrrRRR";

// LDS Load Pointer to DS
// 1100 0101 mo reg r/m
lds = "11000101mmrrrRRR";

// LES Load Pointer to ES
// 1100 0100 mo reg r/m
les = "11000100mmrrrRRR";

// LAHF Load AH with Flags
// 1001 1111
lahf = "10011111";

// SAHF Store AH into Flags
// 1001 1110
sahf = "10011110";

// PUSHF Push Flags
// 1001 1100
pushf = "10011100";

// POPF Pop Flags
// 1001 1101
popf = "10011101";

// ADD Add Reg/Memory with Register to Either
// 0000 00 d w mo reg r/m
add_rm_r = "000000dwmmrrrRRR";

// ADD Add Immediate to Register/Memory
// 1000 00 s w mo 000 r/m data
add_i_rm = "10000000mm000RRRDDDDDDDD";
add_i_rm_w = "10000001mm000RRRDDDDDDDDDDDDDDDD";
add_i_rm_sw = "10000011mm000RRRDDDDDDDD";

// ADD Add Immediate to Accumulator
// 0000 010 w data
add_i_a = "00000100DDDDDDDD";

// ADD Add Immediate to Accumulator
// 0000 010 w data (w)
add_i_a_w = "00000101DDDDDDDDDDDDDDDD";

// ADC Add with Carry Reg./Memory with Register to Either
// 0001 00 d w mo reg r/m
adc_rm_r = "000100dwmmrrrRRR";

// ADC Add with Carry Immediate to Register/Memory
// 1000 00 s w mo 010 r/m data
adc_i_rm = "100000swmm010RRRDDDDDDDD";
adc_i_rm_w = "10000001mm010RRRDDDDDDDDDDDDDDDD";
adc_i_rm_sw = "10000011mm010RRRDDDDDDDD";

// ADC Add with Carry Immediate to Accumulator
// 0001 010 w data
adc_i_a = "00010100DDDDDDDD";

// ADC Add with Carry Immediate to Accumulator
// 0001 010 w data (w)
adc_i_a = "00010101DDDDDDDDDDDDDDDD";

// INC Increment Register/Memory
// 1111 111 w mo 000 r/m
inc_rm = "1111111wmm000RRR";

// INC Increment Register
// 0100 0 reg
inc_r = "01000rrr";

// AAA ASCII Adjust for Add
// 0011 0111
aaa = "00110111";

// BAA Decimal Adjust for Add
// 0010 0111
baa = "00100111";

// SUB Subtract Reg./Memory and Register to Either
// 0010 10 d w mo reg r/m
sub_rm_r = "001010dwmmrrrRRR";

// SUB Subtract Immediate from Register/Memory
// 1000 00 s w mo 101 r/m data (sw)
sub_i_rm = "10000000mm101RRRDDDDDDDD";
sub_i_rm_w = "10000001mm101RRRDDDDDDDDDDDDDDDD";
sub_i_rm_sw = "10000011mm101RRRDDDDDDDD";

// SUB Subtract Immediate from Accumulator
// 0010 110 w data (w)
sub_i_a = "00101100DDDDDDDD";
sub_i_a_w = "00101101DDDDDDDDDDDDDDDD";

// SSB Subtract with Borrow Reg./Memory and Register to Either
// 0001 10 d w mo reg r/m
ssb_rm_r = "000110dwmmrrrRRR";

// SSB Subtract with Borrow Immediate from Register/Memory
// 1000 00 s w mo 011 r/m data (sw)
ssb_i_rm = "10000000mm011RRRDDDDDDDD";
ssb_i_rm_w = "10000001mm011RRRDDDDDDDDDDDDDDDD";
ssb_i_rm_sw = "10000011mm011RRRDDDDDDDD";

// SSB Subtract with Borrow Immediate from Accumulator
// 0001 11 w data (w)
ssb_i_a = "00011100DDDDDDDD";
ssb_i_a_w = "00011101DDDDDDDDDDDDDDDD";

// DEC Decrement Register/memory
// 1111 111 w mo 001 r/m
dec_rm = "1111111wmm001RRR";

// DEC Decrement Register
// 0100 1 reg
dec_r = "01001rrr";

// NEG Change sign (NOT+1)
// 1111 011 w mo 011 r/m
neg = "1111011wmm011RRR";

// CMP Compare Register/Memory and Register
// 0011 10 d w mo reg r/m
cmp_rm_r = "001110dwmmrrrRRR";

// CMP Compare Immediate with Register/Memory
// 1000 00 s w mo 111 r/m data (sw)
cmp_i_rm = "10000000mm111RRRDDDDDDDD";
cmp_i_rm_w = "10000001mm111RRRDDDDDDDDDDDDDDDD";
cmp_i_rm_sw = "10000011mm111RRRDDDDDDDD";

// CMP Compare Immediate with Accumulator
// 0011 110 w data (w)
cmp_i_a = "00111100DDDDDDDD";
cmp_i_a_w = "00111101DDDDDDDDDDDDDDDD";

// AAS ASCII Adjust for Subtract
// 0011 1111
aas = "00111111";

// DAS Decimal Adjust for Subtract
// 0010 1111
das = "00101111";

// MUL Multiply (Unsigned)
// 1111 011 w mo 100 r/m
mul = "1111011wmm100RRR";

// IMUL Integer Multiply (Signed)
// 1111 011 w mo 101 r/m
imul = "1111011wmm101RRR";

// AAM ASCII Adjust for Multiply
// 1101 0100 0000 1010
aam = "1101010000001010";

// DIV Divide (Unsigned)
// 1111 011 w mo 110 r/m
div = "1111011wmm110RRR";

// IDIV Integer Divide (Signed)
// 1111 011 w mo 111 r/m
idiv = "1111011wmm111RRR";

// AAD ASCII Adjust for Divide
// 1101 0101 0000 1010
aad = "1101010100001010";

// CBW Convert Byte to Word
// 1001 1000
cbw = "10011000";

// CWD Convert Word to Double Word
// 1001 1001
cwd = "10011001";

// NOT Invert
// 1111 011 w mo 010 r/m
not = "1111011wmm010RRR";

// SHL/SAL Shift Logical/Arithmetic Left
// 1101 00 v w mo 100 r/m
shl = "110100vwmm100RRR";
sal = shl;

// SHR Shift Logical Right
// 1101 00 v w mo 101 r/m
shr = "110100vwmm101RRR";

// SAR Shift Arithmetic Right
// 1101 00 v w mo 111 r/m
sar = "110100vwmm111RRR";

// ROL Rotate Left
// 1101 00 v w mo 000 r/m
rol = "110100vwmm000RRR";

// ROR Rotate Right
// 1101 00 v w mo 001 r/m
ror = "110100vwmm001RRR";

// RCL Rotate Through Carry Flag Left
// 1101 00 v w mo 010 r/m
rcl = "110100vwmm010RRR";

// RCR Rotate Through Carry Right
// 1101 00 v w mo 011 r/m
rcr = "110100vwmm011RRR";

// AND And Reg./Memory and Register to Either
// 0010 00 d w mo reg r/m
and_rm_r = "001000dwmmrrrRRR";

// AND And Immediate to Register/Memory
// 1000 000 w mo 100 r/m data (w)
and_i_rm = "10000000mm100RRRDDDDDDDD";
and_i_rm_w = "10000001mm100RRRDDDDDDDDDDDDDDDD";

// AND And Immediate to Accumulator
// 0010 010 w data (w)
and_i_a = "00100100DDDDDDDD";
and_i_a_w = "00100101DDDDDDDDDDDDDDDD";

// TEST And Function to Flags, No Result Register/Memory and Register
// 1000 010 w mo reg r/m
test_rm_r = "1000010wmmrrrRRR";

// TEST And Function to Flags, No Result Immediate Data and Register/Memory
// 1111 011 w mo 000 r/m data (w)
test_i_rm = "11110110mm000RRRDDDDDDDD";
test_i_rm_w = "11110111mm000RRRDDDDDDDDDDDDDDDD";

// TEST And Function to Flags, No Result Immediate Data and Accumulator
// 1010 100 w data (w)
test_i_a = "10101000DDDDDDDD";
test_i_a_w = "10101001DDDDDDDDDDDDDDDD";

// OR Or Reg./Memory and Register to Either
// 0000 10 d w mo reg r/m
or_rm_r = "000010dwmmrrrRRR";

// OR Or Immediate to Register/Memory
// 1000 000 w mo 001 r/m data (w)
or_i_r = "10000000mm001RRRDDDDDDDD";
or_i_r = "10000001mm001RRRDDDDDDDDDDDDDDDD";

// OR Or Immediate to Accumulator
// 0000 110 w data (w)
or_i_a = "00001100DDDDDDDD";
or_i_a = "00001101DDDDDDDDDDDDDDDD";

// XOR Exclusive or Reg./Memory and Register to Either
// 0011 00 d w mo reg r/m
xor_rm_r = "001100dwmmrrrRRR";

// XOR Exclusive or Immediate to Register/Memory
// 1000 000 w mo 110 r/m data (w)
xor_i_rm = "10000000mm110RRRDDDDDDDD";
xor_i_rm_w = "10000001mm110RRRDDDDDDDDDDDDDDDD";

// XOR Exclusive or Immediate to Accumulator
// 0011 010 w data (w)
xor_i_a = "00110100DDDDDDDD";
xor_i_a_w = "00110101DDDDDDDDDDDDDDDD";

// REP Repeat
// 1111 001 z
rep = "1111001z";

// MOVS Move Byte/Word
// 1010 010 w
movs = "1010010w";

// CMPS Compare Byte/Word
// 1010 011 w
cmps = "1010011w";

// SCAS Scan Byte/Word
// 1010 111 w
scas = "1010111w";

// LODS Load Byte/Word to AL/AX
// 1010 110w
lods = "1010110w";

// STOS Store Byte/Word from AL/AX
// 1010 101w
stos = "1010101w";

// CALL Call Direct within Segment
// 1110 1000 disp (lo-hi)
call_near_i = "11101000PPPPPPPPPPPPPPPP";

// CALL Call Indirect within Segment
// 1111 1111 mo 010 r/m
call_near_r = "11111111mm010RRR";

// CALL Call Direct Intersegment
// 1001 1010 (offset (lo-hi) or seg (lo-hi))
call_far_i = "10011010oooooooooooooooo";

// CALL Call Indirect Intersegment
// 1111 1111 mo 011 r/m
call_far_r = "11111111mm011RRR";

// JMP Unconditional Jump Direct within Segment
// 1110 1001 disp (lo-hi)
jmp_near_i_w = "11101001PPPPPPPPPPPPPPPP";

// JMP Unconditional Jump Direct within Segment-Short
// 1110 1011 disp
jmp_near_i = "11101011PPPPPPPP";

// JMP Unconditional Jump Indirect within Segment
// 1111 1111 mo 100 r/m
jmp_near_r = "11111111mm100RRR";

// JMP Unconditional Jump Direct Intersegment
// 1110 1010 offset (lo-hi) seg (lo-hi)
jmp_far_i = "11101010oooooooooooooooo";

// JMP Unconditional Jump Indirect Intersegment
// 1111 1111 mo 101 r/m
jmp_far_r = "11111111mm101RRR";

// RET Return from CALL Within Segment
// 1100 0011
ret_near = "11000011";

// RET Return from CALL Within Segment Adding Immediate to SP
// 1100 0010 data-low data-high
ret_near_i = "11000010DDDDDDDDDDDDDDDD";

// RET Return from CALL Intersegment
// 1100 1011
ret_far = "11001011";

// RET Return from CALL Intersegment Adding Immediate to SP
// 1100 1010 data-low data-high
ret_far_i = "11001010DDDDDDDDDDDDDDDD";

// JE/JZ Jump on Equal/Zero
// 0111 0100 disp
je = "01110100PPPPPPPP";
jz = je;

// JL/JNGE Jump on Less
// 0111 1100 disp
jl = "01111100PPPPPPPP";
jnge = jl;

// JLE/JNG Jump on Less or Equal
// 0111 1110 disp
jle = "01111110PPPPPPPP";
jng = jle;

// JB/JNAE Jump on Below
// 0111 0010 disp
jb = "01110010PPPPPPPP";
jnae = jb;

// JBE/JNA Jump on Below or Equal
// 0111 0110 disp
jbe = "01110110PPPPPPPP";
jna = jbe;

// JP/JPE Jump on Parity/Parity Even
// 0111 1010 disp
jp = "01111010PPPPPPPP";
jpe = jp;

// JO Jump on Overflow
// 0111 0000 disp
jo = "01110000PPPPPPPP";

// JS Jump on Sign
// 0111 1000 disp
js = "01111000PPPPPPPP";

// JNE/JNZ Jump on Not Equal/Not Zero
// 0111 0101 disp
jne = "01110101PPPPPPPP";
jnz = jne;

// JNL/JGE Jump on Greater or Equal
// 0111 1101 disp
jge = "01111101PPPPPPPP";
jnl = jge;

// JNLE/JG Jump on Greater
// 0111 1111 disp
jg = "01111111PPPPPPPP";
jnle = jg;

// JNB/JAE Jump on Above or Equal
// 0111 0011 disp
jae = "01110011PPPPPPPP";
jnb = jae;

// JNBE/JA Jump on Above
// 0111 0111 disp
ja = "01110111PPPPPPPP";
jnbe = ja;

// JNP/JPO Jump on Not Par/Par Odd
// 0111 1011 disp
jnp = "01111011PPPPPPPP";
jpo = jnp;

// JNO Jump on Not Overflow
// 0111 0001 disp
jno = "01110001PPPPPPPP";

// JNS Jump on Not Sign
// 0111 1001 disp
jns = "01111001PPPPPPPP";

// LOOP Loop CX Times
// 1110 0010 disp
loop = "11100010PPPPPPPP";

// LOOPZ/LOOPE Loop While Zero/Equal
// 1110 0001 disp
loopz = "11100001PPPPPPPP";
loope = loopz;

// LOOPNZ/LOOPNE Loop While Not Zero/Equal
// 1110 0000 disp
loopnz = "11100000PPPPPPPP";
loopne = loopnz;

// JCXZ Jump on CX Zero
// 1110 0011 disp
jcxz = "11100011PPPPPPPP";

// INT Interrupt Type Specified
// 1100 1101 type
int_typs = "11001101iiiiiiii";

// INT Interrupt Type 3
// 1100 1100
int_typ3 = "11001100";

// INTO Interrupt on Overflow
// 1100 1110
into = "11001110";

// IRET Interrupt Return
// 1100 1111
iret = "11001111";

// CLC Clear Carry
// 1111 1000
clc = "11111000";

// CMC Complement Carry
// 1111 0101
cmc = "11110101";

// STC Set Carry
// 1111 1001
stc = "11111001";

// CLD Clear Direction
// 1111 1100
cld = "11111100";

// STD Set Direction
// 1111 1101
std = "11111101";

// CLI Clear Interrupt
// 1111 1010
cli = "11111010";

// STI Set Interrupt
// 1111 1011
sti = "11111011";

// HLT Halt
// 1111 0100
hlt = "11110100";

// WAIT Wait
// 1001 1011
wait = "10011011";

// ESC Escape (to External Device)
// 1101 1 xxx mo xxx r/m
esc = "11011xxxmmxxxRRR";

// LOCK Bus Lock Prefix
// 1111 0000
lock = "11110000";


char** instructionTypes = [mov_rm_r, mov_i_rm, mov_i_rm_w, mov_i_r, mov_i_r_w, mov_m_a, mov_a_m, mov_rm_s, mov_s_rm, push_rm, push_r, push_s, pop_rm, pop_r, pop_s, xchg_rm_r, xchg_r_a, in_fixp, in_varp, out_fixp, out_varp, xlat, lea, lds, les, lahf, sahf, pushf, popf, add_rm_r, add_i_rm, add_i_rm_w, add_i_rm_sw, add_i_a, add_i_a_w, adc_rm_r, adc_i_rm, adc_i_rm_w, adc_i_rm_sw, adc_i_a, adc_i_a, inc_rm, inc_r, aaa, baa, sub_rm_r, sub_i_rm, sub_i_rm_w, sub_i_rm_sw, sub_i_a, sub_i_a_w, ssb_rm_r, ssb_i_rm, ssb_i_rm_w, ssb_i_rm_sw, ssb_i_a, ssb_i_a_w, dec_rm, dec_r, neg, cmp_rm_r, cmp_i_rm, cmp_i_rm_w, cmp_i_rm_sw, cmp_i_a, cmp_i_a_w, aas, das, mul, imul, aam, div, idiv, aad, cbw, cwd, not, shl, sal, shr, sar, rol, ror, rcl, rcr, and_rm_r, and_i_rm, and_i_rm_w, and_i_a, and_i_a_w, test_rm_r, test_i_rm, test_i_rm_w, test_i_a, test_i_a_w, or_rm_r, or_i_r, or_i_r, scas, lods, stos, call_near_i, call_near_r, call_far_i, call_far_r, jmp_near_i_w, jmp_near_i, jmp_near_r, jmp_far_i, jmp_far_r, ret_near, ret_near_i, ret_far, ret_far_i, je, jz, jl, jnge, jle, jng, jb, jnae, jbe, jna, jp, jpe, jo, js, jne, jnz, jge, jnl, jg, jnle, jae, jnb, ja, jnbe, jnp, jpo, jno, jns, loop, loopz, loope, loopnz, loopne, jcxz, int_typs, int_typ3, into, iret, clc, cmc, stc, cld, std, cli, sti, hlt, wait, esc, lock];
