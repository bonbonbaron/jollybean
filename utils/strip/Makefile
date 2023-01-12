# Target: strip

# Variables
CC             = cc
SRCDIR         = ./src
BUILDDIR       = ./build
BINDIR         = ./bin
INSTALLDIR     = /usr/local/lib

# Compiler
${BUILDDIR}/strip.o: ./src/strip.c ./src/include/strip.h ${BUILDDIR}
	$(CC) -g $(shell sdl2-config --cflags) $(shell pkg-config --cflags zlib) $(shell libpng-config --cflags) -I./src/include -I../inflatable/src/include -I../../src/include -I../file/src/include/ -I${HOME}/hack/jollybean/src/include -O0 -c ./src/strip.c -o $@
	touch $@

# Build directory (dependency)
${BUILDDIR}:
	mkdir -p ${BUILDDIR}

# Cleaner
.PHONY: clean
clean:
	rm -rf ${BUILDDIR}
