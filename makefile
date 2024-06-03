program: main.c opcode.c
	gcc main.c disasembler.c opcode.c -I. -o dis
