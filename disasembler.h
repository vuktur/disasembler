#ifndef DISASEMBLER_H
#define DISASEMBLER_H

#include <stdio.h>

/// @brief A header
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

/// @brief An instruction
typedef struct InstructionStruct {
    struct InstructionType *type;
    unsigned int data;
} Instruction;

int readHeader(FILE *file, Header *hdr);

int printHeader(Header *hdr);

Instruction readInstruction(char *text, int textLen, int *pos);

char* fieldExists(Instruction instr, char field);

int getField(Instruction instr, char field);

void getHexOperands(Instruction instr, char **formats, int **operands);

void getStrOperands(Instruction instr, char **formats, char **operands);

void getOperands(int **hexOperands, char **strOperands, Instruction instr);

int printInstruction(unsigned int pos, Instruction instruction);

int readText(FILE *file, Header *hdr);

#endif