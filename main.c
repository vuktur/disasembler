#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "opcodes.h"

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

typedef struct InstructionStruct {
	char** type;
	long data;
} Instruction;

int readHeader(FILE* file, Header* hdr)
{
	fseek(file, 0, SEEK_SET);

	hdr->magic = getc(file)<<8 | getc(file);
	hdr->flags = getc(file);
	hdr->cpu = getc(file);
	hdr->hdrlen = getc(file);

	if (hdr->magic != 0x0103) {
		printf("bad magic number\n");
		return 1;
	}

	char* buf = malloc(hdr->hdrlen);
	fseek(file, 0, SEEK_SET);

	for (int i = 0; i<hdr->hdrlen; ++i)
		*(buf+i) = getc(file);

	if (hdr->hdrlen >= 32) {
		hdr->textlen = *((int*)buf+2);
		hdr->datalen = *((int*)buf+3);
		hdr->bsslen = *((int*)buf+4);
		hdr->entrylen = *((int*)buf+5);
		hdr->totallen = *((int*)buf+6);
		hdr->symslen = *((int*)buf+7);
	}

	/*long test = ftell(file);
	printf("%d\n", test);*/

	free(buf);
	return 0;
}

int printHeader(Header* hdr)
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



int readText(Header* hdr)
{
	char* text = malloc(hdr->textlen);
	for (int i = 0;* i<hdr->textlen; ++i)
		*(text+i) = getc(file);

	char* buf = malloc(4);

	for (int i = 0; i<textlen; ++i) {
		//printf("%02x ", *(text+i) & 0xff);
		printf("%08b\n", *(text+i) & 0xff);
	}

	for (int i = 0; i<textlen; ++i) {
		//printf("%02x ", *(text+i) & 0xff);
		printf("%08b\n", *(text+i) & 0xff);
	}
	printf("\n");*/
	return 0;
}

int main(int argc, char** argv)
{
	FILE* file;
	int status;
	//char* buffer;
	//long filelen;

	file = fopen(argv[1], "rb");

	/*
	fseek(file, 0, SEEK_END);
	ftell(file);
	fseek(file, 0, SEEK_SET);
	*/

	Header* hdr = malloc(sizeof(Header));
	status = readHeader(file, hdr);
	printHeader(hdr);

	status = readText(hdr);

	return 0;
}
