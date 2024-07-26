IDIR =.
CC=gcc
CFLAGS=-I$(IDIR) -g

ODIR=obj


_DEPS = disasembler.h header.h instruction.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = disasembler.o header.o instruction.o main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

dis: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~
