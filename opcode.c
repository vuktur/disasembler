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
        strcpy(res, "SS");
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
    {"mov %s, %s", "100010dwmmrrrRRR"},                    // MOV - Move register/memory to/from register
    {"mov %s, %02x", "11000110mm000RRRDDDDDDDD"},          // MOV - Move immediate to register/memory
    {"mov %s, %04x", "11000111mm000RRRDDDDDDDDDDDDDDDD"},  // MOV - Move immediate to register/memory
    {"mov %s, %02x", "10110rrrDDDDDDDD"},                  // MOV - Move immediate to register
    {"mov %s, %04x", "10111rrrDDDDDDDDDDDDDDDD"},          // MOV - Move immediate to register
    {"mov AL, [%04x]", "10100000aaaaaaaaaaaaaaaa"},        // MOV - Move memory to accumulator
    {"mov AX, [%04x]", "10100001aaaaaaaaaaaaaaaa"},        // MOV - Move memory to accumulator
    {"mov [%04x], AL", "10100010aaaaaaaaaaaaaaaa"},        // MOV - Move accumulator to memory
    {"mov [%04x], AX", "10100011aaaaaaaaaaaaaaaa"},        // MOV - Move accumulator to memory
    {"mov %s, %s", "10001110mm0SSRRR"},                    // MOV - Move register/memory to segment register
    {"mov %s, %s", "10001100mm0SSRRR"},                    // MOV - Move segment register to register/memory
    {"push %s", "11111111mm110RRR"},                       // PUSH - Push register/memory
    {"push %s", "01010rrr"},                               // PUSH - Push register
    {"push %s", "000SS110"},                               // PUSH - Push segment register
    {"pop %s", "10001111mm000RRR"},                        // POP - Pop register/memory
    {"pop %s", "01011rrr"},                                // POP - Pop register
    {"pop %s", "000SS111"},                                // POP - Pop segment register
    {"xchg %s, %s", "1000011wmmrrrRRR"},                   // XCHG - Exchange register/memory with register
    {"xchg AX, %s", "10010rrr"},                           // XCHG - Exchange register with accumulator
    {"in AL, %02x", "11100100pppppppp"},                   // IN - Input from fixed port
    {"in AX, %02x", "11100101pppppppp"},                   // IN - Input from fixed port
    {"in AL, DX", "11101100"},                             // IN - Input from variable port
    {"in AX, DX", "11101101"},                             // IN - Input from variable port
    {"out %02x, AL", "11100110pppppppp"},                  // OUT - Output to fixed port
    {"out %02x, AX", "11100111pppppppp"},                  // OUT - Output to fixed port
    {"out DX, AL", "11101110"},                            // OUT - Output to variable port
    {"out DX, AX", "11101111"},                            // OUT - Output to variable port
    {"xlat", "11010111"},                                  // XLAT - Translate Byte to AL
    {"lea %s", "10001101mmrrrRRR"},                        // LEA - Load EA to register
    {"lds %s", "11000101mmrrrRRR"},                        // LDS - Load Pointer to DS
    {"les %s", "11000100mmrrrRRR"},                        // LES - Load Pointer to ES
    {"lahf", "10011111"},                                  // LAHF - Load AH with Flags
    {"sahf", "10011110"},                                  // SAHF - Store AH into Flags
    {"pushf", "10011100"},                                 // PUSHF - Push Flags
    {"popf", "10011101"},                                  // POPF - Pop Flags
    {"add %s, %s", "000000dwmmrrrRRR"},                    // ADD - Add Reg/memory with register to Either
    {"add %s, %02x", "10000000mm000RRRDDDDDDDD"},          // ADD - Add immediate to register/memory
    {"add %s, %04x", "10000001mm000RRRDDDDDDDDDDDDDDDD"},  // ADD - Add immediate to register/memory
    {"add %s, %04x", "10000011mm000RRRDDDDDDDD"},          // ADD - Add immediate to register/memory
    {"add AL, %02x", "00000100DDDDDDDD"},                  // ADD - Add immediate to accumulator
    {"add AX, %04x", "00000101DDDDDDDDDDDDDDDD"},          // ADD - Add immediate to accumulator
    {"adc %s, %s", "000100dwmmrrrRRR"},                    // ADC - Add with Carry Reg./memory with register to Either
    {"adc %s, %02x", "100000swmm010RRRDDDDDDDD"},          // ADC - Add with Carry immediate to register/memory
    {"adc %s, %04x", "10000001mm010RRRDDDDDDDDDDDDDDDD"},  // ADC - Add with Carry immediate to register/memory
    {"adc %s, %04x", "10000011mm010RRRDDDDDDDD"},          // ADC - Add with Carry immediate to register/memory
    {"adc AL, %02x", "00010100DDDDDDDD"},                  // ADC - Add with Carry immediate to accumulator
    {"adc AX, %04x", "00010101DDDDDDDDDDDDDDDD"},          // ADC - Add with Carry immediate to accumulator
    {"inc %s", "1111111wmm000RRR"},                        // INC - Increment register/memory
    {"inc %s", "01000rrr"},                                // INC - Increment register
    {"aaa", "00110111"},                                   // AAA - ASCII Adjust for Add
    {"baa", "00100111"},                                   // BAA - Decimal Adjust for Add
                // 10000011 00101110 00100001 00000000 00100000
    {"sub %s, %s", "001010dwmmrrrRRR"},                    // SUB - Subtract Reg./memory and register to Either
    {"sub %s, %02x", "10000000mm101RRRDDDDDDDD"},          // SUB - Subtract immediate from register/memory
    {"sub %s, %04x", "10000001mm101RRRDDDDDDDDDDDDDDDD"},  // SUB - Subtract immediate from register/memory
    {"sub %s, %04x", "10000011mm101RRRDDDDDDDD"},          // SUB - Subtract immediate from register/memory
    {"sub AL, %02x", "00101100DDDDDDDD"},                  // SUB - Subtract immediate from accumulator
    {"sub AX, %04x", "00101101DDDDDDDDDDDDDDDD"},          // SUB - Subtract immediate from accumulator
    {"ssb %s, %s", "000110dwmmrrrRRR"},                    // SSB - Subtract with Borrow Reg./memory and register to Either
    {"ssb %s, %02x", "10000000mm011RRRDDDDDDDD"},          // SSB - Subtract with Borrow immediate from register/memory
    {"ssb %s, %04x", "10000001mm011RRRDDDDDDDDDDDDDDDD"},  // SSB - Subtract with Borrow immediate from register/memory
    {"ssb %s, %04x", "10000011mm011RRRDDDDDDDD"},          // SSB - Subtract with Borrow immediate from register/memory
    {"ssb AL, %02x", "00011100DDDDDDDD"},                  // SSB - Subtract with Borrow immediate from accumulator
    {"ssb AX, %04x", "00011101DDDDDDDDDDDDDDDD"},          // SSB - Subtract with Borrow immediate from accumulator
    {"dec %s", "1111111wmm001RRR"},                        // DEC - Decrement register/memory
    {"dec %s", "01001rrr"},                                // DEC - Decrement register
    {"neg %s", "1111011wmm011RRR"},                        // NEG - Change sign (NOT+1)
    {"cmp %s, %s", "001110dwmmrrrRRR"},                    // CMP - Compare register/memory and register
    {"cmp %s, %02x", "10000000mm111RRRDDDDDDDD"},          // CMP - Compare immediate with register/memory
    {"cmp %s, %04x", "10000001mm111RRRDDDDDDDDDDDDDDDD"},  // CMP - Compare immediate with register/memory
    {"cmp %s, %04x", "10000011mm111RRRDDDDDDDD"},          // CMP - Compare immediate with register/memory
    {"cmp AL, %02x", "00111100DDDDDDDD"},                  // CMP - Compare immediate with accumulator
    {"cmp AX, %04x", "00111101DDDDDDDDDDDDDDDD"},          // CMP - Compare immediate with accumulator
    {"aas", "00111111"},                                   // AAS - ASCII Adjust for Subtract
    {"das", "00101111"},                                   // DAS - Decimal Adjust for Subtract
    {"mul %s", "1111011wmm100RRR"},                        // MUL - Multiply (Unsigned) AX = AL * r/m
    {"imul %s", "1111011wmm101RRR"},                       // IMUL - Integer Multiply (Signed)
    {"aam", "1101010000001010"},                           // AAM - ASCII Adjust for Multiply
    {"div %s", "1111011wmm110RRR"},                        // DIV - Divide (Unsigned)
    {"idiv %s", "1111011wmm111RRR"},                       // IDIV - Integer Divide (Signed)
    {"aad", "1101010100001010"},                           // AAD - ASCII Adjust for Divide
    {"cbw", "10011000"},                                   // CBW - Convert Byte to Word
    {"cwd", "10011001"},                                   // CWD - Convert Word to Double Word
    {"not %s", "1111011wmm010RRR"},                        // NOT - Invert
    {"shl %s, 1", "1101000wmm100RRR"},                     // SHL/SAL - Shift Logical/Arithmetic Left
    {"shl %s, CL", "1101001wmm100RRR"},                    // SHL/SAL - Shift Logical/Arithmetic Left
    {"shr %s, 1", "1101000wmm101RRR"},                     // SHR - Shift Logical Right
    {"shr %s, CL", "1101001wmm101RRR"},                    // SHR - Shift Logical Right
    {"sar %s, 1", "1101000wmm111RRR"},                     // SAR - Shift Arithmetic Right
    {"sar %s, CL", "1101001wmm111RRR"},                    // SAR - Shift Arithmetic Right
    {"rol %s, 1", "1101000wmm000RRR"},                     // ROL - Rotate Left
    {"rol %s, CL", "1101001wmm000RRR"},                    // ROL - Rotate Left
    {"ror %s, 1", "1101000wmm001RRR"},                     // ROR - Rotate Right
    {"ror %s, CL", "1101001wmm001RRR"},                    // ROR - Rotate Right
    {"rcl %s, 1", "1101000wmm010RRR"},                     // RCL - Rotate Through Carry Flag Left
    {"rcl %s, CL", "1101001wmm010RRR"},                    // RCL - Rotate Through Carry Flag Left
    {"rcr %s, 1", "110100vwmm011RRR"},                     // RCR - Rotate Through Carry Right
    {"rcr %s, CL", "110100vwmm011RRR"},                    // RCR - Rotate Through Carry Right
    {"and %s, %s", "001000dwmmrrrRRR"},                    // AND - And Reg./memory and register to Either
    {"and %s, %02x", "10000000mm100RRRDDDDDDDD"},          // AND - And immediate to register/memory
    {"and %s, %04x", "10000001mm100RRRDDDDDDDDDDDDDDDD"},  // AND - And immediate to register/memory
    {"and AL, %02x", "00100100DDDDDDDD"},                  // AND - And immediate to accumulator
    {"and AX, %04x", "00100101DDDDDDDDDDDDDDDD"},          // AND - And immediate to accumulator
    {"test %s, %s", "1000010wmmrrrRRR"},                   // TEST - And Function to Flags, No Result register/memory and register
    {"test %s, %02x", "11110110mm000RRRDDDDDDDD"},         // TEST - And Function to Flags, No Result immediate Data and register/memory
    {"test %s, %04x", "11110111mm000RRRDDDDDDDDDDDDDDDD"}, // TEST - And Function to Flags, No Result immediate Data and register/memory
    {"test AL, %02x", "10101000DDDDDDDD"},                 // TEST - And Function to Flags, No Result immediate Data and accumulator
    {"test AX, %04x", "10101001DDDDDDDDDDDDDDDD"},         // TEST - And Function to Flags, No Result immediate Data and accumulator
    {"or %s, %s", "000010dwmmrrrRRR"},                     // OR - Or Reg./memory and register to Either
    {"or %s, %02x", "10000000mm001RRRDDDDDDDD"},           // OR - Or immediate to register/memory
    {"or %s, %04x", "10000001mm001RRRDDDDDDDDDDDDDDDD"},   // OR - Or immediate to register/memory
    {"or AL, %02x", "00001100DDDDDDDD"},                   // OR - Or immediate to accumulator
    {"or AX, %04x", "00001101DDDDDDDDDDDDDDDD"},           // OR - Or immediate to accumulator
    {"xor %s, %s", "001100dwmmrrrRRR"},                    // XOR - Exclusive or Reg./memory and register to Either
    {"xor %s, %02x", "10000000mm110RRRDDDDDDDD"},          // XOR - Exclusive or immediate to register/memory
    {"xor %s, %04x", "10000001mm110RRRDDDDDDDDDDDDDDDD"},  // XOR - Exclusive or immediate to register/memory
    {"xor AL, %02x", "00110100DDDDDDDD"},                  // XOR - Exclusive or immediate to accumulator
    {"xor AX, %04x", "00110101DDDDDDDDDDDDDDDD"},          // XOR - Exclusive or immediate to accumulator
    {"rep", "1111001z"},                                   // REP - Repeat
    {"movs", "1010010w"},                                  // MOVS - Move Byte/Word from DS:SI to ES:DI
    {"cmps", "1010011w"},                                  // CMPS - Compare Byte/Word
    {"scas", "1010111w"},                                  // SCAS - Scan Byte/Word
    {"lods", "1010110w"},                                  // LODS - Load Byte/Word to AL/AX
    {"stos", "1010101w"},                                  // STOS - Store Byte/Word from AL/AX
    {"call %04x", "11101000PPPPPPPPPPPPPPPP"},             // CALL - Call Direct within segment
    {"call %s", "11111111mm010RRR"},                       // CALL - Call Indirect within segment
    {"call %04x", "10011010oooooooooooooooo"},             // CALL - Call Direct Intersegment
    {"call %s", "11111111mm011RRR"},                       // CALL - Call Indirect Intersegment
    {"jmp %04x", "11101001PPPPPPPPPPPPPPPP"},              // JMP - Unconditional Jump Direct within segment
    {"jmp %02x", "11101011PPPPPPPP"},                      // JMP - Unconditional Jump Direct within segment-Short
    {"jmp %s", "11111111mm100RRR"},                        // JMP - Unconditional Jump Indirect within segment
    {"jmp %04x", "11101010oooooooooooooooo"},              // JMP - Unconditional Jump Direct Intersegment
    {"jmp %s", "11111111mm101RRR"},                        // JMP - Unconditional Jump Indirect Intersegment
    {"ret", "11000011"},                                   // RET - Return from CALL Within segment
    {"ret %04x", "11000010DDDDDDDDDDDDDDDD"},              // RET - Return from CALL Within segment Adding immediate to SP
    {"ret", "11001011"},                                   // RET - Return from CALL Intersegment
    {"ret %04x", "11001010DDDDDDDDDDDDDDDD"},              // RET - Return from CALL Intersegment Adding immediate to SP
    {"je %02x", "01110100PPPPPPPP"},                       // JE/JZ - Jump on Equal/Zero
    {"jl %02x", "01111100PPPPPPPP"},                       // JL/JNGE - Jump on Less
    {"jle %02x", "01111110PPPPPPPP"},                      // JLE/JNG - Jump on Less or Equal
    {"jb %02x", "01110010PPPPPPPP"},                       // JB/JNAE - Jump on Below
    {"jbe %02x", "01110110PPPPPPPP"},                      // JBE/JNA - Jump on Below or Equal
    {"jp %02x", "01111010PPPPPPPP"},                       // JP/JPE - Jump on Parity/Parity Even
    {"jo %02x", "01110000PPPPPPPP"},                       // JO - Jump on Overflow
    {"js %02x", "01111000PPPPPPPP"},                       // JS - Jump on Sign
    {"jne %02x", "01110101PPPPPPPP"},                      // JNE/JNZ - Jump on Not Equal/Not Zero
    {"jge %02x", "01111101PPPPPPPP"},                      // JNL/JGE - Jump on Greater or Equal
    {"jg %02x", "01111111PPPPPPPP"},                       // JNLE/JG - Jump on Greater
    {"jae %02x", "01110011PPPPPPPP"},                      // JNB/JAE - Jump on Above or Equal
    {"ja %02x", "01110111PPPPPPPP"},                       // JNBE/JA - Jump on Above
    {"jnp %02x", "01111011PPPPPPPP"},                      // JNP/JPO - Jump on Not Par/Par Odd
    {"jno %02x", "01110001PPPPPPPP"},                      // JNO - Jump on Not Overflow
    {"jns %02x", "01111001PPPPPPPP"},                      // JNS - Jump on Not Sign
    {"loop %02x", "11100010PPPPPPPP"},                     // LOOP - Loop CX Times
    {"loopz %02x", "11100001PPPPPPPP"},                    // LOOPZ/LOOPE - Loop While Zero/Equal
    {"loopnz %02x", "11100000PPPPPPPP"},                   // LOOPNZ/LOOPNE - Loop While Not Zero/Equal
    {"jcxz %02x", "11100011PPPPPPPP"},                     // JCXZ - Jump on CX Zero
    {"int %02x", "11001101iiiiiiii"},                      // INT - Interrupt Type Specified
    {"int 03", "11001100"},                                // INT - Interrupt Type 3
    {"into", "11001110"},                                  // INTO - Interrupt on Overflow
    {"iret", "11001111"},                                  // IRET - Interrupt Return
    {"clc", "11111000"},                                   // CLC - Clear Carry
    {"cmc", "11110101"},                                   // CMC - Complement Carry
    {"stc", "11111001"},                                   // STC - Set Carry
    {"cld", "11111100"},                                   // CLD - Clear Direction
    {"std", "11111101"},                                   // STD - Set Direction
    {"cli", "11111010"},                                   // CLI - Clear Interrupt
    {"sti", "11111011"},                                   // STI - Set Interrupt
    {"hlt", "11110100"},                                   // HLT - Halt
    {"wait", "10011011"},                                  // WAIT - Wait
    {"esc %s", "11011xxxmmxxxRRR"},                        // ESC - Escape (to External Device)
    {"lock", "11110000"},                                  // LOCK - Bus Lock Prefix
};