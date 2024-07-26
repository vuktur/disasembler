#include "header.h"
#include <stdio.h>
#include <stdlib.h>


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
    printf("magic: %04x \t", hdr->magic & 0xffff);
    printf("flags: %02x \t", hdr->flags & 0xff);
    printf("cpu: %02x \t", hdr->cpu & 0xff);
    printf("hdrlen: %dB \n", hdr->hdrlen);

    if (hdr->hdrlen >= 32) {
        printf("textlen: %dB \t", hdr->textlen);
        printf("datalen: %dB \t", hdr->datalen);
        printf("bsslen: %dB \t", hdr->bsslen);
        printf("entrylen: %dB \t", hdr->entrylen);
        printf("totallen: %dB \t", hdr->totallen);
        printf("symslen: %dB \n", hdr->symslen);
    }

    return 0;
}