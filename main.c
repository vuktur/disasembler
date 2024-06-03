#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opcode.h"
#include "disasembler.h"

int main(int argc, char **argv)
{
    FILE *file;
    int status;

    file = fopen(argv[1], "rb");

    Header *hdr = malloc(sizeof(Header));
    status = readHeader(file, hdr);
    printHeader(hdr);

    status = readText(file, hdr);

    free(hdr);
    return 0;
}
