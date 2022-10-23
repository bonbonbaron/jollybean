# Target: img

# Variables
CC             = cc
SRCDIR         = ./src
BUILDDIR       = ./build
BINDIR         = ./bin
INSTALLDIR     = /usr/local/lib

# Linker
${BINDIR}/img: ${BUILDDIR}/img.o ../inflatable/build/inflatable.o ${BUILDIR} ${BINDIR}
	$(CC) -g ${BUILDDIR}/img.o ../inflatable/build/inflatable.o -L${HOME}/hack/botox/lib/ -lbotox $(shell pkg-config --libs zlib) $(shell sdl2-config --libs) $(shell libpng-config --libs) -o $@
	touch $@

# Compiler
${BUILDDIR}/img.o: ./src/img.c ./src/include/img.h ${BUILDDIR}
	$(CC) -g $(shell sdl2-config --cflags) $(shell pkg-config --cflags zlib) $(shell libpng-config --cflags) -I./src/include -I../inflatable/src/include -I${HOME}/hack/botox/src/include -c ./src/img.c -o $@
	touch $@

# Build directory (dependency)
${BUILDDIR}:
	mkdir -p ${BUILDDIR}

${BINDIR}:
	mkdir -p ${BINDIR}

# Cleaner
.PHONY: clean
clean:
	rm -rf ${BUILDDIR}
	rm -rf ${BINDIR}
