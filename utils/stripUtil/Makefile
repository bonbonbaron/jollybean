# Target: strip

# Variables
CC             = cc
SRCDIR         = ./src
BUILDDIR       = ./build
BINDIR         = ./bin
INSTALLDIR     = /usr/local/lib
RPODIR        := $(shell git rev-parse --show-toplevel)

# Compiler
${BUILDDIR}/stripUtil.o: ./src/stripUtil.c ./src/include/stripUtil.h ${BUILDDIR}
	$(CC) -g $(shell sdl2-config --cflags) $(shell pkg-config --cflags zlib) $(shell libpng-config --cflags) -I./src/include -I../infUtil/src/include -I../../src/include -I../file/src/include/ -I${RPODIR}/src/data/include -O0 -c ./src/stripUtil.c -o $@
	touch $@

# Build directory (dependency)
${BUILDDIR}:
	mkdir -p ${BUILDDIR}

# Cleaner
.PHONY: clean
clean:
	rm -rf ${BUILDDIR}
