#include "disasembler.h"
#include "opcode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_OF_HEX_FIELDS 6
#define NUM_OF_STR_FIELDS 3

int readHeader(FILE *file, Header *hdr)
{
    fseek(file, 0, SEEK_SET);

    hdr->magic = getc(file) << 8 | getc(file);
    hdr->flags = getc(file);
    hdr->cpu = getc(file);
    hdr->hdrlen = getc(file);

    if (hdr->magic != 0x0103) {
        printf("bad magic number\n");
        return 1;
    }

    char *buf = malloc(hdr->hdrlen);
    fseek(file, 0, SEEK_SET);

    for (int i = 0; i < hdr->hdrlen; ++i)
        *(buf + i) = getc(file);

    if (hdr->hdrlen >= 32) {
        hdr->textlen = *((int *)buf + 2);
        hdr->datalen = *((int *)buf + 3);
        hdr->bsslen = *((int *)buf + 4);
        hdr->entrylen = *((int *)buf + 5);
        hdr->totallen = *((int *)buf + 6);
        hdr->symslen = *((int *)buf + 7);
    }

    free(buf);
    return 0;
}

int printHeader(Header *hdr)
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

Instruction readInstruction(char *text, int textLen, int *pos)
{
    int currPos = *pos;
    int num_of_candidates = NUM_OF_INSTRUCT_TYPES;
    char *candidates = malloc(NUM_OF_INSTRUCT_TYPES);
    for (int i = 0; i < NUM_OF_INSTRUCT_TYPES; ++i)
        candidates[i] = 1;

    while (currPos - (*pos) < 4 && currPos + 1 < textLen) {
        char currByte = *(text + currPos);
        for (int i = 0; i < 8; ++i) {
            char bit = ((currByte & (1 << (7 - i)))) >> (7 - i);
            for (int j = 0; j < NUM_OF_INSTRUCT_TYPES; ++j) {
                if (candidates[j] != 0) {
                    char opchar = instructionTypes[j].opcode[(currPos - (*pos)) * 8 + i];
                    if ((opchar - '0' == 0 || opchar - '0' == 1) && opchar - '0' != bit) {
                        candidates[j] = 0;
                        --num_of_candidates;
                    }
                }
            }
        }
        if (num_of_candidates > 1)
            ++currPos;
        else
            break;
    }

    InstructionType *match = NULL;
    if (num_of_candidates == 1) {
        for (int i = 0; i < NUM_OF_INSTRUCT_TYPES; ++i) {
            if (candidates[i] != 0)
                match = (InstructionType *)(instructionTypes + i);
        }
    }
    free(candidates);
    Instruction res = {match, 0};

    if (match != NULL) {
        for (int i = 0; i < strlen(match->opcode) / 8; i++)
            res.data = (res.data <<= 8) | (*(text + (*pos)++) & 0xff);
    }

    return res;
}

char *fieldExists(Instruction instr, char field)
{
    for (char *a = instr.type->opcode; *a; ++a) {
        if (*a == field)
            return a;
    }
    return NULL;
}

int getField(Instruction instr, char field)
{
    char *a = instr.type->opcode;
    int res = 0;
    int mask = 0;
    int div = 0;

    for (int i = 0; *(a + i); ++i) {
        mask <<= 1;
        div <<= 1;
        if (*(a + i) == field) {
            mask |= 1;
            if (*(a + i + 1) != field)
                div |= 1;
        }
    }

    if (mask == 0 || div == 0)
        return 0;

    res = (instr.data & mask) / div;
    return res;
}

void getHexOperands(Instruction instr, char **formats, int **operands)
{
    char possibleFields[NUM_OF_HEX_FIELDS] = {'D', 'P', 'o', 'i', 'a', 'p'};

    for (int i = 0; i < 2; ++i) {
        char *format = formats[i];
        if (format == NULL || *format != '0')
            continue;

        int mask = (*(format + 1) == '4') ? 0xffff : 0xff;
        operands[i] = malloc(sizeof(int));

        for (int j = 0; j < NUM_OF_HEX_FIELDS; ++j) {
            if (fieldExists(instr, possibleFields[j])) {
                *(operands[i]) = getField(instr, possibleFields[j]) & mask;
                possibleFields[j] = '\0';
            }
        }
    }
}

void getStrOperands(Instruction instr, char **formats, char **operands)
{
    char possibleFields[NUM_OF_STR_FIELDS] = {'r', 'S', 'R'};
    char actualFields[2] = {'\0', '\0'};

    for (int i = 0; i < 2; ++i) {
        char *format = formats[i];
        if (format == NULL || *format != 's')
            continue;

        if (fieldExists(instr, 'r') && possibleFields[0]) {
            char w = fieldExists(instr, 'w') ? getField(instr, w)
                                             : (fieldExists(instr, 'D') ? strlen(fieldExists(instr, 'D')) / 8 - 1 : 1);
            operands[i] = getReg(getField(instr, 'r'), w);
            actualFields[i] = 'r';
            possibleFields[0] = '\0';

        } else if (fieldExists(instr, 'S') && possibleFields[1]) {
            operands[i] = getSeg(getField(instr, 'S'));
            actualFields[i] = 'S';
            possibleFields[1] = '\0';

        } else if (fieldExists(instr, 'R') && possibleFields[2]) {
            operands[i] = getRegMem(getField(instr, 'M'), getField(instr, 'R'));
            actualFields[i] = 'R';
            possibleFields[2] = '\0';
        }
    }

    if (fieldExists(instr, 'd')) {
        int d = getField(instr, 'd');
        if ((d == 0 && (actualFields[0] == 'r' || actualFields[0] == 'S')) || (d == 1 || actualFields[0] == 'R')) {
            char *tmp = operands[1];
            operands[1] = operands[0];
            operands[0] = tmp;
        }
    }
}

void getOperands(int **hexOperands, char **strOperands, Instruction instr)
{
    char *formats[2] = {NULL, NULL};
    for (char *a = instr.type->format; *a; ++a) {
        if (*a == '%') {
            if (*(a + 1)) {
                if (formats[0] == NULL)
                    formats[0] = a + 1;
                else if (formats[1] == NULL)
                    formats[1] = a + 1;
            }
        }
    }

    if (formats[0] == NULL)
        return;

    getHexOperands(instr, formats, hexOperands);
    getStrOperands(instr, formats, strOperands);
}

int printInstruction(unsigned int pos, Instruction instr)
{
    printf("%04x: ", pos);
    int len = strlen(instr.type->opcode) / 8;
    printf("%0*x\t    ", 2 * len, instr.data);

    int *hexOperands[2] = {NULL, NULL};
    char *strOperands[2] = {NULL, NULL};
    getOperands(hexOperands, strOperands, instr);

    printf(instr.type->format,
           hexOperands[0] ? *(hexOperands[0]) : (strOperands[0] ? strOperands[0] : 0),
           hexOperands[1] ? *(hexOperands[1]) : (strOperands[1] ? strOperands[1] : 0));
    printf("\n");

    for (int i = 0; i < 2; ++i) {
        if (hexOperands[i] != NULL)
            free(hexOperands[i]);
        if (strOperands[i] != NULL)
            free(strOperands[i]);
    }
    return 0;
}

int readText(FILE *file, Header *hdr)
{
    char *text = malloc(hdr->textlen);
    fseek(file, hdr->hdrlen, SEEK_SET);

    for (int i = 0; i < hdr->textlen; ++i)
        *(text + i) = getc(file);

    unsigned int pos = 0;
    while (pos + 1 < hdr->textlen) {
        unsigned int prevPos = pos;
        Instruction res = readInstruction(text, hdr->textlen, &pos);
        if (res.type == NULL) {
            printf("instruction has no match\n");
            free(text);
            return 1;
        }
        printInstruction(prevPos, res);
    }

    free(text);
    return 0;
}