# Target: dir

# Variables
CC             = cc
SRCDIR         = ./src
BUILDDIR       = ./build
BINDIR         = ./bin
INSTALLDIR     = /usr/local/lib

# Compiler
${BUILDDIR}/dir.o: ./src/dir.c ./src/include/dir.h ${BUILDDIR}
	$(CC) -g $(shell sdl2-config --cflags) $(shell pkg-config --cflags zlib) $(shell libpng-config --cflags) -I../strip/src/include -I./src/include -I../inflatable/src/include -I${HOME}/hack/botox/src/include -I../file/src/include/ -I${HOME}/hack/jollybean/src/include -O0 -c ./src/dir.c -o $@
	touch $@

# Build directory (dependency)
${BUILDDIR}:
	mkdir -p ${BUILDDIR}

# Cleaner
.PHONY: clean
clean:
	rm -rf ${BUILDDIR}
