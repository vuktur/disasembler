#include "disasembler.h"
#include "instruction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define isBit(a) ((a) == 0 || (a) == 1)

const char hexChars[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

char *replacedStr(char *source, char *target, char *newStr)
{
    int len = strlen(source) - strlen(target) + strlen(newStr);
    char *res = (char *)malloc(len + 1);
    for (int is = 0, ir = 0; is < strlen(source); ++is, ++ir) {
        int targetFound = 1;
        for (int j = 0; j < strlen(target); ++j)
            if (source[is + j] != target[j] || is + j >= strlen(source))
                targetFound = 0;
        if (targetFound) {
            for (int j = 0; j < strlen(newStr); ++j) {
                res[ir + j] = newStr[j];
            }
            is += strlen(target) - 1;
            ir += strlen(newStr) - 1;
        } else {
            res[ir] = source[is];
        }
    }
    return res;
}

char *replacedStrField(char *source, char field, char *newStr)
{
    int len = strlen(source) - 3 + strlen(newStr);
    char *res = (char *)malloc(len + 1);
    for (int is = 0, ir = 0; is < strlen(source); ++is, ++ir) {
        if (source[is] == '$' && source[is + 1] == field) {
            for (int j = 0; j < strlen(newStr); ++j) {
                res[ir + j] = newStr[j];
            }
            is += 1;
            ir += strlen(newStr) - 1;
        } else {
            res[ir] = source[is];
        }
    }
    return res;
}

char *byteFormatHex(char *bytes, int len)
{
    char *res = malloc(2 * len + 1);
    for (int i = 0; i < len; ++i) {
        res[2 * i] = hexChars[(bytes[i] >> 4) & 0xf];
        res[2 * i + 1] = hexChars[bytes[i] & 0xf];
    }
    res[2 * len] = '\0';
    return res;
}

char *byteFormatRevHex(char *bytes, int len)
{
    char *res = malloc(2 * len + 1);
    for (int i = 0; i < len; ++i) {
        res[2 * i] = hexChars[(bytes[len - i - 1] >> 4) & 0xf];
        res[2 * i + 1] = hexChars[bytes[len - i - 1] & 0xf];
    }
    res[2 * len] = '\0';
    return res;
}

char *stripLeadingZeros(char *str)
{
    int c = 0;
    while (str[c] == '0')
        ++c;
    if (str[c] == '\0')
        --c;
    for (int j = 0; str[j + c - 1]; ++j)
        str[j] = str[j + c];
    return str;
}

int fieldExists(const InstructionType *instrType, char field)
{
    for (const char *a = instrType->codeFormat; *a && *a != '('; ++a) {
        if (*a == field)
            return 1;
    }
    return 0;
}

int getField(Instruction *instr, char field)
{
    int res = 0, mask = 0, div = 0, i = 0;

    for (const char *a = instr->type->codeFormat; *a && *a != '('; ++a, ++i) {
        mask <<= 1;
        div <<= 1;
        if (*a == field) {
            mask |= 1;
            if (*(a + 1) != field)
                div |= 1;
        }
    }

    if (mask == 0 || div == 0)
        return 0;
    for (int j = 0; j < i / 8; ++j)
        res = res << 8 | *(instr->data + j);
    res = (res & mask) / div;
    return res;
}

int decomposeRegMem(char byte, char *mod, char *reg, char *rxm)
{
    if (mod)
        *mod = (byte & 0xc0) >> 6;
    if (reg)
        *reg = (byte & 0x38) >> 3;
    if (rxm)
        *rxm = (byte & 0x07) >> 0;
    return 0;
}

int byteMatch(char byte, const char *str)
{
    if (str == NULL || str[0] == '\0') {
        return 2;
    } else if (str[0] == '(') {
        if (str[1] == 'R') {
            if (str[2] == 'r') {
            } else if (isBit(str[2] - '0') && isBit(str[3] - '0') && isBit(str[4] - '0')) {
                char srcReg, refReg;
                refReg = ((str[2] - '0') << 2) | ((str[3] - '0') << 1) | (str[4] - '0');
                decomposeRegMem(byte, NULL, &srcReg, NULL);
                return (refReg == srcReg) ? 1 : 0;
            }
        }
        return 1;
    } else if (isBit(str[0] - '0')) {
        for (int i = 0; i < 8; ++i) {
            if (str[i] == '\0')
                return -1;
            char bit = ((1 << (7 - i)) & byte) >> (7 - i);
            if (isBit(str[i] - '0') && str[i] - '0' != bit)
                return 0;
        }
        return 1;
    }
    return -1;
}

const char **splitCodeFormat(const InstructionType *instrType)
{
    const char **parts = malloc(sizeof(char *) * MAX_INSTRUCT_LEN);
    return splitCodeFormatTo(instrType, parts);
}

const char **splitCodeFormatTo(const InstructionType *instrType, const char **parts)
{
    parts[0] = instrType->codeFormat;
    for (int i = 0; i < 14; ++i) {
        if (parts[i]) {
            if (*(parts[i]) == '(') {
                parts[i + 1] = parts[i];
                while (*(parts[i + 1]++) != ')')
                    continue;
            } else if (isBit(*(parts[i]) - '0')) {
                parts[i + 1] = parts[i] + 8;
            }
        }
        if (parts[i] == NULL || (parts[i + 1] && *(parts[i + 1]) == 0))
            parts[i + 1] = NULL;
    }
    return parts;
}

int calcInstrLength(Instruction *instr)
{
    const char **codeFormatParts = splitCodeFormat(instr->type);
    int res = calcInstrLengthFrom(instr, codeFormatParts);
    free(codeFormatParts);
    return res;
}

int calcInstrLengthFrom(Instruction *instr, const char **codeFormatParts)
{
    if (instr->type == NULL)
        return 0;

    int length = 0, partLength;
    const char *part;
    for (int i = 0; codeFormatParts[i] != NULL; ++i) {
        part = codeFormatParts[i];
        partLength = 0;
        if (part[0] == '(') {
            switch (part[1]) {
            case 'R':
                partLength = 1;
                char mod, rxm;
                decomposeRegMem(*(instr->data + length), &mod, NULL, &rxm);
                if (mod == 0b01)
                    partLength = 2;
                else if ((mod == 0b00 && rxm == 0b110) || mod == 0b10)
                    partLength = 3;
                break;
            case 'D':
            case 'a':
            case 'P':
                partLength = 1;
                if (part[2] == 'w' || (getField(instr, 'w') | getField(instr, 'W')) == 1)
                    partLength = 2;
                break;
            case 'p':
            case 'i':
                partLength = 1;
                break;
            case 'o':
                partLength = 4;
                break;
            }
        } else if (isBit(part[0] - '0')) {
            partLength = 1;
        }

        instr->partsLengths[i] = partLength;
        length += partLength;
    }

    instr->length = length;
    return length;
}

Instruction readInstruction(char *text, int textLen, unsigned int pos)
{
    Instruction res = {NULL, text + pos, 0, {0}};
    const char **codeFormatParts = malloc(sizeof(char *) * MAX_INSTRUCT_LEN);

    for (int i = 0; i < NUM_OF_INSTRUCT_TYPES; ++i) {
        int currentPos = 0, isMatch = 1, status;
        splitCodeFormatTo(&(instructionTypes[i]), codeFormatParts);
        while (pos + currentPos < textLen) {
            status = byteMatch(*(text + pos + currentPos), codeFormatParts[currentPos]);
            if (status == 1) {
                ++currentPos;
            } else {
                if (status == 0)
                    isMatch = 0;
                break;
            }
        }

        if (isMatch) {
            res.type = instructionTypes + i;
            calcInstrLengthFrom(&res, codeFormatParts);
            break;
        }
    }

    free(codeFormatParts);
    return res;
}

char *getDisp(char mod, char rxm, int pos, Instruction *instr)
{
    char *res;
    if (mod == 0b00) {
        if (rxm == 0b110) { // EA = disph, displ
            char disp[2];
            disp[0] = instr->data[pos + 2];
            disp[1] = instr->data[pos + 1];
            res = byteFormatHex(disp, 2);
        } else { // disp = 0
            res = malloc(1);
            res[0] = '\0';
        }
    } else if (mod == 0b01) { // disp = displ sign extended to word
        res = malloc(4);
        if ((instr->data[pos + 1] & 0x80) == 0) {
            res[0] = '+';
            char *tmp = stripLeadingZeros(byteFormatHex(&instr->data[pos + 1], 1));
            strcpy(res + 1, tmp);
            free(tmp);
        } else {
            res[0] = '-';
            char twoComp = (instr->data[pos + 1] ^ 0xff) + 1;
            char *tmp = stripLeadingZeros(byteFormatHex(&twoComp, 1));
            strcpy(res + 1, tmp);
            free(tmp);
        }
    } else if (mod == 0b10) { // disp = disph, displ
        res = malloc(6);
        res[0] = '+';
        char disp[2];
        disp[0] = instr->data[pos + 2];
        disp[1] = instr->data[pos + 1];
        char *tmp = stripLeadingZeros(byteFormatHex(disp, 2));
        strcpy(res + 1, tmp);
        free(tmp);
    } else { // if (mod == 0b11) { // r/m is treated as a reg field
        res = malloc(1);
        res[0] = '\0';
    }
    return res;
}

int printInstruction(unsigned int pos, Instruction *instr)
{
    char *value = byteFormatHex(instr->data, instr->length);
    printf("%04x: %-13s ", pos, value);
    free(value);

    char fields[NUM_OF_PRINTFMT_FIELDS] = {'\0'};
    char *datas[NUM_OF_INSTRUCT_TYPES] = {NULL};
    for (int i = 0, j = 0; instr->type->printFormat[i + 1] != '\0'; ++i) {
        if (instr->type->printFormat[i] == '$')
            fields[j++] = instr->type->printFormat[i + 1];
    }

    const char **codeFormatParts = splitCodeFormat(instr->type);

    char c, *data;
    for (int i = 0; i < NUM_OF_PRINTFMT_FIELDS && fields[i]; ++i) {
        c = fields[i];
        switch (c) {

        case 'R':
            char mod = 0, rxm = 0;
            int j, k;
            for (j = 0, k = 0; j < MAX_INSTRUCT_LEN;
                 k += instr->partsLengths[j], ++j) {
                if (codeFormatParts[j][0] == '(' && codeFormatParts[j][1] == 'R') {
                    decomposeRegMem(instr->data[k], &mod, NULL, &rxm);
                    break;
                }
            }

            if (mod == 0b11) {
                int w = 1;
                if (fieldExists(instr->type, 'w') | fieldExists(instr->type, 'W'))
                    w = getField(instr, 'w') | getField(instr, 'W');
                data = getRegPrintStr(rxm, w);
            } else {
                char *tmp = getRegMemPrintStr(mod, rxm);
                char *disp = getDisp(mod, rxm, k, instr);
                data = replacedStr(tmp, "+disp", disp);
                free(tmp);
                free(disp);
            }
            break;

        case 'r':
        case 's':
            char reg;
            if (fieldExists(instr->type, c)) {
                reg = getField(instr, c);
            } else {
                for (int j = 0, k = 0; j < MAX_INSTRUCT_LEN; k += instr->partsLengths[j], ++j) {
                    if (codeFormatParts[j][0] == '(' && codeFormatParts[j][1] == 'R' && codeFormatParts[j][2] == c) {
                        decomposeRegMem(instr->data[k], NULL, &reg, NULL);
                        break;
                    }
                }
            }
            if (c == 'r') {
                int w = 1;
                if (fieldExists(instr->type, 'w') | fieldExists(instr->type, 'W'))
                    w = getField(instr, 'w') | getField(instr, 'W');
                data = getRegPrintStr(reg, w);
            } else if (c == 's') {
                data = getSegPrintStr(reg);
            }
            break;

        case 'P':
        case 'D':
        case 'a':
        case 'p':
        case 'o':
        case 'i':
            int isRD = 0;
            for (int j = 0, k = 0; j < MAX_INSTRUCT_LEN; k += instr->partsLengths[j], ++j) {
                if (codeFormatParts[j][0] == '(' && codeFormatParts[j][1] == 'R')
                    isRD = 1;
                if (codeFormatParts[j][0] == '(' && codeFormatParts[j][1] == c) { // "(D)", "(a)", ...
                    if (c == 'D') {
                        if (isRD == 1 && (!fieldExists(instr->type, 'W') ||
                                          ((getField(instr, 'W') & 0b10) == 0b10))) {
                            data = byteFormatRevHex(instr->data + k, instr->partsLengths[j]);
                            data = stripLeadingZeros(data);
                        } else {
                            data = byteFormatRevHex(instr->data + k, instr->partsLengths[j]);
                        }
                    } else if (c == 'P') {
                        char offset[2];

                        int offsetByte = instr->data[k] & 0xff;
                        if (codeFormatParts[j][2] == 'w') {
                            offsetByte += (instr->data[k + 1] << 8) & 0xffff;
                            offsetByte += (pos + instr->length) & 0xffff;
                        } else {
                            if ((offsetByte & 0x80) == 0) {
                                offsetByte = ((pos + instr->length) & 0xffff) + offsetByte;
                            } else {
                                offsetByte = ((pos + instr->length) & 0xffff) - ((offsetByte ^ 0xff) + 0x1);
                            }
                        }
                        offset[0] = (offsetByte >> 8) & 0xff;
                        offset[1] = offsetByte & 0xff;
                        data = byteFormatHex(offset, 2);
                    } else {
                        data = byteFormatHex(instr->data + k, instr->partsLengths[j]);
                    }
                    break;
                }
            }
            break;

        case 'w':
            int w = getField(instr, 'w') | getField(instr, 'W');
            data = malloc(2);
            data[0] = (w == 0) ? 'l' : 'x';
            data[1] = '\0';
            break;

        case 'c':
            data = malloc(3);
            strcpy(data, getField(instr, 'r') == 0 ? "1" : "CL");
            break;

        default:
            data = malloc(1);
            data[0] = '\0';
            break;
        }

        datas[i] = data;
    }

    // switch r/m and reg or seg bits if the direction bit is 1
    if (fieldExists(instr->type, 'd') && getField(instr, 'd') == 1) {
        int posR = 0, posrs = 0;
        for (int i = 0; fields[i] != '\0'; ++i) {
            if (fields[i] == 'R')
                posR = i;
            if (fields[i] == 'r' || fields[i] == 's')
                posrs = i;
        }

        fields[posR] = fields[posrs];
        fields[posrs] = 'R';
    }

    char *res = malloc(33);
    strcpy(res, instr->type->printFormat);
    for (int i = 0; i < NUM_OF_PRINTFMT_FIELDS && fields[i]; ++i) {
        char *newRes = replacedStrField(res, fields[i], datas[i]);
        free(res);
        res = newRes;
        free(datas[i]);
    }

    printf("%s\n", res);

    free(codeFormatParts);
    return 0;
}

int readText(FILE *file, Header *hdr)
{
    char *text = malloc(hdr->textlen);
    fseek(file, hdr->hdrlen, SEEK_SET);

    for (int i = 0; i < hdr->textlen; ++i)
        *(text + i) = getc(file);

    unsigned int pos = 0;
    while (pos < hdr->textlen) {
        Instruction res = readInstruction(text, hdr->textlen, pos);
        if (res.type == NULL) {
            printf("instruction has no match\n");
            free(text);
            return 1;
        }
        if (res.length == 0) {
            printf("zero length instruction\n");
            free(text);
            return 1;
        }
        printInstruction(pos, &res);
        pos += res.length;
    }

    free(text);
    return 0;
}