#include "disasembler.h"
#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    FILE *file;
    int status;

    if (argc != 2) {
        printf("no file path specified\n");
        exit(1);
    }

    file = fopen(argv[1], "rb");

    Header *hdr = malloc(sizeof(Header));
    status = readHeader(file, hdr);
    printHeader(hdr);

    status = readText(file, hdr);
    // char a = 0b11111111;
    // for (int i = 0; i < NUM_OF_INSTRUCT_TYPES; ++i) {
    //     Instruction instr = {
    //         instructionTypes + i,
    //         &a,
    //         1};
    //     int res = getField(&instr, 'w');
    //     printf("res = %d\t%s\n", res, instr.type->printFormat);
    // }

    free(hdr);
    return 0;
}
