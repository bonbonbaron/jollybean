# Target: strip

# Variables
CC             = cc
SRCDIR         = ./src
BUILDDIR       = ./build
BINDIR         = ./bin
INSTALLDIR     = /usr/local/lib
RPODIR        := $(shell git rev-parse --show-toplevel)
SRC           := $(RPODIR)/src

# Compiler
${BUILDDIR}/sd.o: ./src/sd.c ./src/include/sd.h ${BUILDDIR}
	$(CC) -g $(shell sdl2-config --cflags) $(shell pkg-config --cflags zlib) $(shell libpng-config --cflags) -I./src/include -I../inf/src/include -I$(SRC)/include -I$(SRC)/data/include -I../file/src/include/ -O0 -c ./src/sd.c -o $@
	touch $@

# Build directory (dependency)
${BUILDDIR}:
	mkdir -p ${BUILDDIR}

# Cleaner
.PHONY: clean
clean:
	rm -rf ${BUILDDIR}

