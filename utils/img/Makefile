# Target: img

# Variables
CC             = cc
SRCDIR         = ./src
BUILDDIR       = ./build
BINDIR         = ./bin
INSTALLDIR     = /usr/local/lib

# Linker
${BINDIR}/img: ${BUILDDIR}/img.o ../dir/build/dir.o ../strip/build/strip.o ../inflatable/build/inflatable.o ${BUILDIR} ${BINDIR}
	$(CC) -g ${BUILDDIR}/img.o ../dir/build/dir.o ../strip/build/strip.o ../file/build/fileUtils.o ../inflatable/build/inflatable.o -L${HOME}/hack/botox/lib/ -lbotox $(shell pkg-config --libs zlib) $(shell sdl2-config --libs) $(shell libpng-config --libs) -o $@
	touch $@

# Compiler
${BUILDDIR}/img.o: ./src/img.c ./src/include/img.h ${BUILDDIR}
	$(CC) -g $(shell sdl2-config --cflags) $(shell pkg-config --cflags zlib) $(shell libpng-config --cflags) -I../strip/src/include -I./src/include -I../inflatable/src/include -I${HOME}/hack/botox/src/include -I../file/src/include/ -I${HOME}/hack/jollybean/src/include -I../dir/src/include -O1 -c ./src/img.c -o $@
	touch $@

../strip/build/strip.o: ../strip/src/strip.c ../strip/src/include/strip.h
	make -C ../strip

../inflatable/build/inflatable.o: ../inflatable/src/inflatable.c ../inflatable/src/include/inflatable.h
	make -C ../inflatable

../dir/build/dir.o: ../dir/src/dir.c ../dir/src/include/dir.h
	make -C ../dir

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
