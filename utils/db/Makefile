# Target: db

# Variables
CC             = cc
SRCDIR         = ./src
BUILDDIR       = ./build
BINDIR         = ./bin
INSTALLDIR     = /usr/local/lib

# Compiler
${BUILDDIR}/db.o: ./src/db.c ./src/include/db.h ${BUILDDIR}
	$(CC) -g $(shell sdl2-config --cflags) $(shell pkg-config --cflags zlib) $(shell libpng-config --cflags) -I../strip/src/include -I./src/include -I../inflatable/src/include -I../../src/data/include -I../file/src/include/ -O0 -c ./src/db.c -o $@
	touch $@

# Build directory (dependency)
${BUILDDIR}:
	mkdir -p ${BUILDDIR}

# Cleaner
.PHONY: clean
clean:
	rm -rf ${BUILDDIR}
