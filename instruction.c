#include "instruction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *getSegPrintStr(int s)
{
    char *res = malloc(3);
    switch (s) {
    case 0b00:
        strcpy(res, "es");
        break;
    case 0b01:
        strcpy(res, "cs");
        break;
    case 0b10:
        strcpy(res, "ss");
        break;
    case 0b11:
        strcpy(res, "ds");
        break;
    }
    return res;
}

char *getRegPrintStr(int r, int w)
{
    char *res = malloc(3);
    switch (r) {
    case 0b000:
        strcpy(res, w == 1 ? "ax" : "al");
        break;
    case 0b001:
        strcpy(res, w != 0 ? "cx" : "cl");
        break;
    case 0b010:
        strcpy(res, w != 0 ? "dx" : "dl");
        break;
    case 0b011:
        strcpy(res, w != 0 ? "bx" : "bl");
        break;
    case 0b100:
        strcpy(res, w != 0 ? "sp" : "ah");
        break;
    case 0b101:
        strcpy(res, w != 0 ? "bp" : "ch");
        break;
    case 0b110:
        strcpy(res, w != 0 ? "si" : "dh");
        break;
    case 0b111:
        strcpy(res, w != 0 ? "di" : "bh");
        break;
    }
    return res;
}

char *getRegMemPrintStr(int m, int R)
{
    if (m == 0b11)
        return getRegPrintStr(R, 1);

    char *res = malloc(13);

    if (R == 0b110 && m == 0b00) {
        strcpy(res, "[+disp]");
    } else {
        switch (R) {
        case 0b000:
            strcpy(res, "[bx+si+disp]");
            break;
        case 0b001:
            strcpy(res, "[bx+di+disp]");
            break;
        case 0b010:
            strcpy(res, "[bp+si+disp]");
            break;
        case 0b011:
            strcpy(res, "[bp+di+disp]");
            break;
        case 0b100:
            strcpy(res, "[si+disp]");
            break;
        case 0b101:
            strcpy(res, "[di+disp]");
            break;
        case 0b110:
            strcpy(res, "[bp+disp]");
            break;
        case 0b111:
            strcpy(res, "[bx+disp]");
            break;
        }
    }

    return res;
}

const InstructionType instructionTypes[NUM_OF_INSTRUCT_TYPES] = {
    {"mov $R, $r", "100010dw(Rr)"},       // MOV - Move register/memory to/from register
    {"mov $R, $D", "1100011w(R000)(D)"},  // MOV - Move immediate to register/memory
    {"mov $r, $D", "1011wrrr(D)"},        // MOV - Move immediate to register
    {"mov a$w, $a", "1010000w(a)"},       // MOV - Move memory to accumulator
    {"mov $a, a$w", "1010001w(a)"},       // MOV - Move accumulator to memory
    {"mov $R, $s", "100011d0(Rs)"},       // MOV - Move register/memory to segment register
    {"push $R", "11111111(R110)"},        // PUSH - Push register/memory
    {"push $r", "01010rrr"},              // PUSH - Push register
    {"push $s", "000ss110"},              // PUSH - Push segment register
    {"pop $R", "10001111(R000)"},         // POP - Pop to register/memory
    {"pop $r", "01011rrr"},               // POP - Pop to register
    {"pop $s", "000ss111"},               // POP - Pop to segment register
    {"xchg $R, $r", "1000011w(Rr)"},      // XCHG - Exchange register/memory with register
    {"xchg ax, $r", "10010rrr"},          // XCHG - Exchange register with accumulator
    {"in a$w, $p", "1110010w(p)"},        // IN - Input from fixed port
    {"in a$w, dx", "1110110w"},           // IN - Input from variable port
    {"out $p, a$w", "1110011w(p)"},       // OUT - Output to fixed port
    {"out dx, a$w", "1110111w"},          // OUT - Output to variable port
    {"xlat", "11010111"},                 // XLAT - Translate Byte to AL
    {"lea $r, $R", "10001101(Rr)"},       // LEA - Load EA to register
    {"lds $r, $R", "11000101(Rr)"},       // LDS - Load pointer to DS
    {"les $r, $R", "11000100(Rr)"},       // LES - Load pointer to ES
    {"lahf", "10011111"},                 // LAHF - Load AH with flags
    {"sahf", "10011110"},                 // SAHF - Store AH into flags
    {"pushf", "10011100"},                // PUSHF - Push flags
    {"popf", "10011101"},                 // POPF - Pop flags
    {"add $R, $r", "000000dw(Rr)"},       // ADD - Add Reg/memory with register to Either
/**/    {"add $R, $D", "100000WW(R000)(D)"},  // ADD - Add immediate to register/memory
    {"add a$w, $D", "0000010w(D)"},       // ADD - Add immediate to accumulator
    {"adc $R, $r", "000100dw(Rr)"},       // ADC - Add with carry Reg./memory with register to Either
    {"adc $R, $D", "100000WW(R010)(D)"},  // ADC - Add with carry immediate to register/memory
    {"adc a$w, $D", "0001010w(D)"},       // ADC - Add with carry immediate to accumulator
    {"inc $R", "1111111w(R000)"},         // INC - Increment register/memory
    {"inc $r", "01000rrr"},               // INC - Increment register
    {"aaa", "00110111"},                  // AAA - ASCII Adjust for Add
    {"baa", "00100111"},                  // BAA - Decimal Adjust for Add
    {"sub $R, $r", "001010dw(Rr)"},       // SUB - Subtract Reg./memory and register to Either
    {"sub $R, $D", "100000WW(R101)(D)"},  // SUB - Subtract immediate from register/memory
    {"sub a$w, $D", "0010110w(D)"},       // SUB - Subtract immediate from accumulator
    {"sbb $R, $r", "000110dw(Rr)"},       // SBB - Subtract with Borrow Reg./memory and register to Either
    {"sbb $R, $D", "100000WW(R011)(D)"},  // SBB - Subtract with Borrow immediate from register/memory
    {"sbb a$w, $D", "0001110w(D)"},       // SBB - Subtract with Borrow immediate from accumulator
    {"dec $R", "1111111w(R001)"},         // DEC - Decrement register/memory
    {"dec $r", "01001rrr"},               // DEC - Decrement register
    {"neg $R", "1111011w(R011)"},         // NEG - Change sign (NOT+1)
    {"cmp $R, $r", "001110dw(Rr)"},       // CMP - Compare register/memory and register
    {"cmp $R, $D", "100000WW(R111)(D)"},  // CMP - Compare immediate with register/memory
    {"cmp a$w, $D", "0011110w(D)"},       // CMP - Compare immediate with accumulator
    {"aas", "00111111"},                  // AAS - ASCII Adjust for Subtract
    {"das", "00101111"},                  // DAS - Decimal Adjust for Subtract
    {"mul $R", "1111011w(R100)"},         // MUL - Multiply (Unsigned) AX = AL * r/m
    {"imul $R", "1111011w(R101)"},        // IMUL - Integer Multiply (Signed)
    {"aam", "1101010000001010"},          // AAM - ASCII Adjust for Multiply
    {"div $R", "1111011w(R110)"},         // DIV - Divide (Unsigned)
    {"idiv $R", "1111011w(R111)"},        // IDIV - Integer Divide (Signed)
    {"aad", "1101010100001010"},          // AAD - ASCII Adjust for Divide
    {"cbw", "10011000"},                  // CBW - Convert Byte to Word
    {"cwd", "10011001"},                  // CWD - Convert Word to Double Word
    {"not $R", "1111011w(R010)"},         // NOT - Invert
    {"shl $R, $c", "110100cw(R100)"},     // SHL/SAL - Shift Logical/Arithmetic Left
    {"shr $R, $c", "110100cw(R101)"},     // SHR - Shift Logical Right
    {"sar $R, $c", "110100cw(R111)"},     // SAR - Shift Arithmetic Right
    {"rol $R, $c", "110100cw(R000)"},     // ROL - Rotate Left
    {"ror $R, $c", "110100cw(R001)"},     // ROR - Rotate Right
    {"rcl $R, $c", "110100cw(R010)"},     // RCL - Rotate Through Carry Flag Left
    {"rcr $R, $c", "110100cw(R011)"},     // RCR - Rotate Through Carry Right
    {"and $R, $r", "001000dw(Rr)"},       // AND - And Reg./memory and register to Either
    {"and $R, $D", "1000000w(R100)(D)"},  // AND - And immediate to register/memory
    {"and a$w, $D", "0010010w(D)"},       // AND - And immediate to accumulator
    {"test $R, $r", "1000010w(Rr)"},      // TEST - And Function to Flags, No Result register/memory and register
    {"test $R, $D", "1111011w(R000)(D)"}, // TEST - And Function to Flags, No Result immediate Data and register/memory
    {"test a$w, $D", "1010100w(D)"},      // TEST - And Function to Flags, No Result immediate Data and accumulator
    {"or $R, $r", "000010dw(Rr)"},        // OR - Or Reg./memory and register to Either
    {"or $R, $D", "1000000w(R001)(D)"},   // OR - Or immediate to register/memory
    {"or a$w, $D", "0000110w(D)"},        // OR - Or immediate to accumulator
    {"xor $R, $r", "001100dw(Rr)"},       // XOR - Exclusive or Reg./memory and register to Either
    {"xor $R, $D", "1000000w(R110)(D)"},  // XOR - Exclusive or immediate to register/memory
    {"xor a$w, $D", "0011010w(D)"},       // XOR - Exclusive or immediate to accumulator
    {"rep", "1111001z"},                  // REP - Repeat
    {"movs", "1010010w"},                 // MOVS - Move Byte/Word from DS:SI to ES:DI
    {"cmps", "1010011w"},                 // CMPS - Compare Byte/Word
    {"scas", "1010111w"},                 // SCAS - Scan Byte/Word
    {"lods", "1010110w"},                 // LODS - Load Byte/Word to AL/AX
    {"stos", "1010101w"},                 // STOS - Store Byte/Word from AL/AX
    {"call $P", "11101000(Pw)"},          // CALL - Call Direct within segment
    {"call $R", "11111111(R010)"},        // CALL - Call Indirect within segment
    {"call $o", "10011010(o)"},           // CALL - Call Direct Intersegment
    {"call $R", "11111111(R011)"},        // CALL - Call Indirect Intersegment
    {"jmp $P", "11101001(Pw)"},           // JMP - Unconditional Jump Direct within segment
    {"jmp short $P", "11101011(P)"},      // JMP - Unconditional Jump Direct within segment-Short
    {"jmp $R", "11111111(R100)"},         // JMP - Unconditional Jump Indirect within segment
    {"jmp $o", "11101010(o)"},            // JMP - Unconditional Jump Direct Intersegment
    {"jmp $R", "11111111(R101)"},         // JMP - Unconditional Jump Indirect Intersegment
    {"ret", "11000011"},                  // RET - Return from CALL Within segment
    {"ret $D", "11000010(Dw)"},           // RET - Return from CALL Within segment Adding immediate to SP
    {"ret", "11001011"},                  // RET - Return from CALL Intersegment
    {"ret $D", "11001010(Dw)"},           // RET - Return from CALL Intersegment Adding immediate to SP
    {"je $P", "01110100(P)"},             // JE/JZ - Jump on Equal/Zero
    {"jl $P", "01111100(P)"},             // JL/JNGE - Jump on Less
    {"jle $P", "01111110(P)"},            // JLE/JNG - Jump on Less or Equal
    {"jb $P", "01110010(P)"},             // JB/JNAE - Jump on Below
    {"jbe $P", "01110110(P)"},            // JBE/JNA - Jump on Below or Equal
    {"jp $P", "01111010(P)"},             // JP/JPE - Jump on Parity/Parity Even
    {"jo $P", "01110000(P)"},             // JO - Jump on Overflow
    {"js $P", "01111000(P)"},             // JS - Jump on Sign
    {"jne $P", "01110101(P)"},            // JNE/JNZ - Jump on Not Equal/Not Zero
    {"jnl $P", "01111101(P)"},            // JNL/JGE - Jump on Greater or Equal
    {"jg $P", "01111111(P)"},             // JNLE/JG - Jump on Greater
    {"jnb $P", "01110011(P)"},            // JNB/JAE - Jump on Above or Equal
    {"ja $P", "01110111(P)"},             // JNBE/JA - Jump on Above
    {"jnp $P", "01111011(P)"},            // JNP/JPO - Jump on Not Par/Par Odd
    {"jno $P", "01110001(P)"},            // JNO - Jump on Not Overflow
    {"jns $P", "01111001(P)"},            // JNS - Jump on Not Sign
    {"jcxz $P", "11100011(P)"},           // JCXZ - Jump on CX Zero
    {"loop $P", "11100010(P)"},           // LOOP - Loop CX Times
    {"loopz $P", "11100001(P)"},          // LOOPZ/LOOPE - Loop While Zero/Equal
    {"loopnz $P", "11100000(P)"},         // LOOPNZ/LOOPNE - Loop While Not Zero/Equal
    {"int $i", "11001101(i)"},            // INT - Interrupt Type Specified
    {"int 03", "11001100"},               // INT - Interrupt Type 3
    {"into", "11001110"},                 // INTO - Interrupt on Overflow
    {"iret", "11001111"},                 // IRET - Interrupt Return
    {"clc", "11111000"},                  // CLC - Clear Carry
    {"cmc", "11110101"},                  // CMC - Complement Carry
    {"stc", "11111001"},                  // STC - Set Carry
    {"cld", "11111100"},                  // CLD - Clear Direction
    {"std", "11111101"},                  // STD - Set Direction
    {"cli", "11111010"},                  // CLI - Clear Interrupt
    {"sti", "11111011"},                  // STI - Set Interrupt
    {"hlt", "11110100"},                  // HLT - Halt
    {"wait", "10011011"},                 // WAIT - Wait
    {"esc $R", "11011xxx(Rxxx)"},         // ESC - Escape (to External Device)
    {"lock", "11110000"},                 // LOCK - Bus Lock Prefix
};