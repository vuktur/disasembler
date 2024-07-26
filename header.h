#ifndef HEADER_H
#define HEADER_H

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

int readHeader(FILE *file, Header *hdr);

int printHeader(Header *hdr);

#endif