#include "opcode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *getSeg(int s)
{
    char *res = malloc(3);
    switch (s) {
    case 0b00:
        strcpy(res, "ES");
        break;
    case 0b01:
        strcpy(res, "CS");
        break;
    case 0b10:
        strcpy(res, "ss");
        break;
    case 0b11:
        strcpy(res, "DS");
        break;
    }
    return res;
}

char *getReg(int r, char w)
{
    char *res = malloc(3);
    switch (r) {
    case 0b000:
        strcpy(res, w ? "AX" : "AL");
        break;
    case 0b001:
        strcpy(res, w ? "CX" : "CL");
        break;
    case 0b010:
        strcpy(res, w ? "DX" : "DL");
        break;
    case 0b011:
        strcpy(res, w ? "BX" : "BL");
        break;
    case 0b100:
        strcpy(res, w ? "SP" : "AH");
        break;
    case 0b101:
        strcpy(res, w ? "BP" : "CH");
        break;
    case 0b110:
        strcpy(res, w ? "SI" : "DH");
        break;
    case 0b111:
        strcpy(res, w ? "DI" : "BH");
        break;
    }
    return res;
}

char *getRegMem(int m, int R)
{
    if (m == 0x11)
        return getReg(R, 1);

    char *res = malloc(13);
    switch (R) {
    case 0b000:
        strcpy(res, m ? "[BX+SI]+disp" : "[BX+SI]");
        break;
    case 0b001:
        strcpy(res, m ? "[BX+DI]+disp" : "[BX+DI]");
        break;
    case 0b010:
        strcpy(res, m ? "[BP+SI]+disp" : "[BP+SI]");
        break;
    case 0b011:
        strcpy(res, m ? "[BP+DI]+disp" : "[BP+DI]");
        break;
    case 0b100:
        strcpy(res, m ? "[SI]+disp" : "[SI]");
        break;
    case 0b101:
        strcpy(res, m ? "[DI]+disp" : "[DI]");
        break;
    case 0b110:
        strcpy(res, m ? "[BP]+disp" : "disp");
        break;
    case 0b111:
        strcpy(res, m ? "[BX]+disp" : "[BX]");
        break;
    }
    return res;
}

const InstructionType instructionTypes[] = {
    {"mov %s, %s", "100010dw(Rr)"},            // MOV - Move register/memory to/from register
    {"mov %s, %x", "1100011w(R000)(D)"},       // MOV - Move immediate to register/memory
    {"mov %s, %x", "1011wrrr(D)"},             // MOV - Move immediate to register
    {"mov AL, [%x]", "10100000(a)"},           // MOV - Move memory to accumulator
    {"mov AX, [%x]", "10100001(a)"},           // MOV - Move memory to accumulator
    {"mov [%x], AL", "10100010(a)"},           // MOV - Move accumulator to memory
    {"mov [%x], AX", "10100011(a)"},           // MOV - Move accumulator to memory
    {"mov %s, %s", "10001110(Rs)"},            // MOV - Move register/memory to segment register
    {"mov %s, %s", "10001100(Rs)"},            // MOV - Move segment register to register/memory
    {"push %s", "11111111(R110)"},             // PUSH - Push register/memory
    {"push %s", "01010rrr"},                   // PUSH - Push register
    {"push %s", "000ss110"},                   // PUSH - Push segment register
    {"pop %s", "10001111(R000)"},              // POP - Pop register/memory
    {"pop %s", "01011rrr"},                    // POP - Pop register
    {"pop %s", "000ss111"},                    // POP - Pop segment register
    {"xchg %s, %s", "1000011w(Rr)"},           // XCHG - Exchange register/memory with register
    {"xchg AX, %s", "10010rrr"},               // XCHG - Exchange register with accumulator
    {"in AL, %x", "11100100(p)"},              // IN - Input from fixed port
    {"in AX, %x", "11100101(p)"},              // IN - Input from fixed port
    {"in AL, DX", "11101100"},                 // IN - Input from variable port
    {"in AX, DX", "11101101"},                 // IN - Input from variable port
    {"out %02x, AL", "11100110(p)"},           // OUT - Output to fixed port
    {"out %02x, AX", "11100111(p)"},           // OUT - Output to fixed port
    {"out DX, AL", "11101110"},                // OUT - Output to variable port
    {"out DX, AX", "11101111"},                // OUT - Output to variable port
    {"xlat", "11010111"},                      // XLAT - Translate Byte to AL
    {"lea %s", "10001101(Rr)"},                // LEA - Load EA to register
    {"lds %s", "11000101(Rr)"},                // LDS - Load Pointer to DS
    {"les %s", "11000100(Rr)"},                // LES - Load Pointer to ES
    {"lahf", "10011111"},                      // LAHF - Load AH with Flags
    {"sahf", "10011110"},                      // SAHF - Store AH into Flags
    {"pushf", "10011100"},                     // PUSHF - Push Flags
    {"popf", "10011101"},                      // POPF - Pop Flags
    {"add %s, %s", "000000dw(Rr)"},            // ADD - Add Reg/memory with register to Either
    {"add %s, %02x", "10000000(R000)(D)"},     // ADD - Add immediate to register/memory
    {"add %s, %04x", "10000001(R000)(D)"},     // ADD - Add immediate to register/memory
    {"add %s, %04x", "10000011(R000)(D)"},     // ADD - Add immediate to register/memory
    {"add AL, %02x", "00000100(D)"},           // ADD - Add immediate to accumulator
    {"add AX, %04x", "00000101(D)"},           // ADD - Add immediate to accumulator
    {"adc %s, %s", "000100dw(Rr)"},            // ADC - Add with Carry Reg./memory with register to Either
    {"adc %s, %02x", "100000sw(R010)(D)"},     // ADC - Add with Carry immediate to register/memory
    {"adc %s, %04x", "10000001(R010)(D)"},     // ADC - Add with Carry immediate to register/memory
    {"adc %s, %04x", "10000011(R010)(D)"},     // ADC - Add with Carry immediate to register/memory
    {"adc AL, %02x", "00010100(D)"},           // ADC - Add with Carry immediate to accumulator
    {"adc AX, %04x", "00010101(D)"},           // ADC - Add with Carry immediate to accumulator
    {"inc %s", "1111111w(R000)"},              // INC - Increment register/memory
    {"inc %s", "01000rrr"},                    // INC - Increment register
    {"aaa", "00110111"},                       // AAA - ASCII Adjust for Add
    {"baa", "00100111"},                       // BAA - Decimal Adjust for Add
    {"sub %s, %s", "001010dw(Rr)"},            // SUB - Subtract Reg./memory and register to Either
    {"sub %s, %02x", "10000000(R101)(D)"},     // SUB - Subtract immediate from register/memory
    {"sub %s, %04x", "100000w1(R101)(D)"},     // SUB - Subtract immediate from register/memory
    {"sub AL, %02x", "00101100(D)"},           // SUB - Subtract immediate from accumulator
    {"sub AX, %04x", "00101101(D)"},           // SUB - Subtract immediate from accumulator
    {"ssb %s, %s", "000110dw(Rr)"},            // ssB - Subtract with Borrow Reg./memory and register to Either
    {"ssb %s, %02x", "10000000(R011)(D)"},     // ssB - Subtract with Borrow immediate from register/memory
    {"ssb %s, %04x", "10000001(R011)(D)"},     // ssB - Subtract with Borrow immediate from register/memory
    {"ssb %s, %04x", "10000011(R011)(D)"},     // ssB - Subtract with Borrow immediate from register/memory
    {"ssb AL, %02x", "00011100(D)"},           // ssB - Subtract with Borrow immediate from accumulator
    {"ssb AX, %04x", "00011101(D)"},           // ssB - Subtract with Borrow immediate from accumulator
    {"dec %s", "1111111w(R001)"},              // DEC - Decrement register/memory
    {"dec %s", "01001rrr"},                    // DEC - Decrement register
    {"neg %s", "1111011w(R011)"},              // NEG - Change sign (NOT+1)
    {"cmp %s, %s", "001110dw(Rr)"},            // CMP - Compare register/memory and register
    {"cmp %s, %02x", "10000000(R111)(D)"},     // CMP - Compare immediate with register/memory
    {"cmp %s, %04x", "10000001(R111)(D)"},     // CMP - Compare immediate with register/memory
    {"cmp %s, %04x", "10000011(R111)(D)"},     // CMP - Compare immediate with register/memory
    {"cmp AL, %02x", "00111100(D)"},           // CMP - Compare immediate with accumulator
    {"cmp AX, %04x", "00111101(D)"},           // CMP - Compare immediate with accumulator
    {"aas", "00111111"},                       // AAS - ASCII Adjust for Subtract
    {"das", "00101111"},                       // DAS - Decimal Adjust for Subtract
    {"mul %s", "1111011w(R100)"},              // MUL - Multiply (Unsigned) AX = AL * r/m
    {"imul %s", "1111011w(R101)"},             // IMUL - Integer Multiply (Signed)
    {"aam", "1101010000001010"},               // AAM - ASCII Adjust for Multiply
    {"div %s", "1111011w(R110)"},              // DIV - Divide (Unsigned)
    {"idiv %s", "1111011w(R111)"},             // IDIV - Integer Divide (Signed)
    {"aad", "1101010100001010"},               // AAD - ASCII Adjust for Divide
    {"cbw", "10011000"},                       // CBW - Convert Byte to Word
    {"cwd", "10011001"},                       // CWD - Convert Word to Double Word
    {"not %s", "1111011w(R010)"},              // NOT - Invert
    {"shl %s, 1", "1101000w(R100)"},           // SHL/SAL - Shift Logical/Arithmetic Left
    {"shl %s, CL", "1101001w(R100)"},          // SHL/SAL - Shift Logical/Arithmetic Left
    {"shr %s, 1", "1101000w(R101)"},           // SHR - Shift Logical Right
    {"shr %s, CL", "1101001w(R101)"},          // SHR - Shift Logical Right
    {"sar %s, 1", "1101000w(R111)"},           // SAR - Shift Arithmetic Right
    {"sar %s, CL", "1101001w(R111)"},          // SAR - Shift Arithmetic Right
    {"rol %s, 1", "1101000w(R000)"},           // ROL - Rotate Left
    {"rol %s, CL", "1101001w(R000)"},          // ROL - Rotate Left
    {"ror %s, 1", "1101000w(R001)"},           // ROR - Rotate Right
    {"ror %s, CL", "1101001w(R001)"},          // ROR - Rotate Right
    {"rcl %s, 1", "1101000w(R010)"},           // RCL - Rotate Through Carry Flag Left
    {"rcl %s, CL", "1101001w(R010)"},          // RCL - Rotate Through Carry Flag Left
    {"rcr %s, 1", "110100vw(R011)"},           // RCR - Rotate Through Carry Right
    {"rcr %s, CL", "110100vw(R011)"},          // RCR - Rotate Through Carry Right
    {"and %s, %s", "001000dw(Rr)"},            // AND - And Reg./memory and register to Either
    {"and %s, %02x", "10000000(R100)(D)"},     // AND - And immediate to register/memory
    {"and %s, %04x", "10000001(R100)(D)"},     // AND - And immediate to register/memory
    {"and AL, %02x", "00100100(D)"},           // AND - And immediate to accumulator
    {"and AX, %04x", "00100101(D)"},           // AND - And immediate to accumulator
    {"test %s, %s", "1000010w(Rr)"},           // TEST - And Function to Flags, No Result register/memory and register
    {"test %s, %02x", "11110110(R000)(D)"},    // TEST - And Function to Flags, No Result immediate Data and register/memory
    {"test %s, %04x", "11110111(R000)(D)"},    // TEST - And Function to Flags, No Result immediate Data and register/memory
    {"test AL, %02x", "10101000(D)"},          // TEST - And Function to Flags, No Result immediate Data and accumulator
    {"test AX, %04x", "10101001(D)"},          // TEST - And Function to Flags, No Result immediate Data and accumulator
    {"or %s, %s", "000010dw(Rr)"},             // OR - Or Reg./memory and register to Either
    {"or %s, %02x", "10000000(R001)(D)"},      // OR - Or immediate to register/memory
    {"or %s, %04x", "10000001(R001)(D)"},      // OR - Or immediate to register/memory
    {"or AL, %02x", "00001100(D)"},            // OR - Or immediate to accumulator
    {"or AX, %04x", "00001101(D)"},            // OR - Or immediate to accumulator
    {"xor %s, %s", "001100dw(Rr)"},            // XOR - Exclusive or Reg./memory and register to Either
    {"xor %s, %02x", "10000000(R110)(D)"},     // XOR - Exclusive or immediate to register/memory
    {"xor %s, %04x", "10000001(R110)(D)"},     // XOR - Exclusive or immediate to register/memory
    {"xor AL, %02x", "00110100(D)"},           // XOR - Exclusive or immediate to accumulator
    {"xor AX, %04x", "00110101(D)"},           // XOR - Exclusive or immediate to accumulator
    {"rep", "1111001z"},                       // REP - Repeat
    {"movs", "1010010w"},                      // MOVS - Move Byte/Word from DS:SI to ES:DI
    {"cmps", "1010011w"},                      // CMPS - Compare Byte/Word
    {"scas", "1010111w"},                      // SCAS - Scan Byte/Word
    {"lods", "1010110w"},                      // LODS - Load Byte/Word to AL/AX
    {"stos", "1010101w"},                      // STOS - Store Byte/Word from AL/AX
    {"call %04x", "11101000(Pw)"},             // CALL - Call Direct within segment
    {"call %s", "11111111(R010)"},             // CALL - Call Indirect within segment
    {"call %04x", "10011010oooooooooooooooo"}, // CALL - Call Direct Intersegment
    {"call %s", "11111111(R011)"},             // CALL - Call Indirect Intersegment
    {"jmp %04x", "11101001(Pw)"},              // JMP - Unconditional Jump Direct within segment
    {"jmp %02x", "11101011(P)"},               // JMP - Unconditional Jump Direct within segment-Short
    {"jmp %s", "11111111(R100)"},              // JMP - Unconditional Jump Indirect within segment
    {"jmp %04x", "11101010oooooooooooooooo"},  // JMP - Unconditional Jump Direct Intersegment
    {"jmp %s", "11111111(R101)"},              // JMP - Unconditional Jump Indirect Intersegment
    {"ret", "11000011"},                       // RET - Return from CALL Within segment
    {"ret %04x", "11000010(D)"},               // RET - Return from CALL Within segment Adding immediate to SP
    {"ret", "11001011"},                       // RET - Return from CALL Intersegment
    {"ret %04x", "11001010(D)"},               // RET - Return from CALL Intersegment Adding immediate to SP
    {"je %02x", "01110100(P)"},                // JE/JZ - Jump on Equal/Zero
    {"jl %02x", "01111100(P)"},                // JL/JNGE - Jump on Less
    {"jle %02x", "01111110(P)"},               // JLE/JNG - Jump on Less or Equal
    {"jb %02x", "01110010(P)"},                // JB/JNAE - Jump on Below
    {"jbe %02x", "01110110(P)"},               // JBE/JNA - Jump on Below or Equal
    {"jp %02x", "01111010(P)"},                // JP/JPE - Jump on Parity/Parity Even
    {"jo %02x", "01110000(P)"},                // JO - Jump on Overflow
    {"js %02x", "01111000(P)"},                // JS - Jump on Sign
    {"jne %02x", "01110101(P)"},               // JNE/JNZ - Jump on Not Equal/Not Zero
    {"jge %02x", "01111101(P)"},               // JNL/JGE - Jump on Greater or Equal
    {"jg %02x", "01111111(P)"},                // JNLE/JG - Jump on Greater
    {"jae %02x", "01110011(P)"},               // JNB/JAE - Jump on Above or Equal
    {"ja %02x", "01110111(P)"},                // JNBE/JA - Jump on Above
    {"jnp %02x", "01111011(P)"},               // JNP/JPO - Jump on Not Par/Par Odd
    {"jno %02x", "01110001(P)"},               // JNO - Jump on Not Overflow
    {"jns %02x", "01111001(P)"},               // JNS - Jump on Not Sign
    {"loop %02x", "11100010(P)"},              // LOOP - Loop CX Times
    {"loopz %02x", "11100001(P)"},             // LOOPZ/LOOPE - Loop While Zero/Equal
    {"loopnz %02x", "11100000(P)"},            // LOOPNZ/LOOPNE - Loop While Not Zero/Equal
    {"jcxz %02x", "11100011(P)"},              // JCXZ - Jump on CX Zero
    {"int %02x", "11001101iiiiiiii"},          // INT - Interrupt Type Specified
    {"int 03", "11001100"},                    // INT - Interrupt Type 3
    {"into", "11001110"},                      // INTO - Interrupt on Overflow
    {"iret", "11001111"},                      // IRET - Interrupt Return
    {"clc", "11111000"},                       // CLC - Clear Carry
    {"cmc", "11110101"},                       // CMC - Complement Carry
    {"stc", "11111001"},                       // STC - Set Carry
    {"cld", "11111100"},                       // CLD - Clear Direction
    {"std", "11111101"},                       // STD - Set Direction
    {"cli", "11111010"},                       // CLI - Clear Interrupt
    {"sti", "11111011"},                       // STI - Set Interrupt
    {"hlt", "11110100"},                       // HLT - Halt
    {"wait", "10011011"},                      // WAIT - Wait
    {"esc %s", "11011xxx(Rxxx)"},              // ESC - Escape (to External Device)
    {"lock", "11110000"},                      // LOCK - Bus Lock Prefix
};