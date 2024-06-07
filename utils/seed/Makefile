# Target: body

# Variables
CC             = cc
SRCDIR         = ./src
BUILDDIR       = ./build
BINDIR         = ./bin
INSTALLDIR     = /usr/local/lib

# Linker
${BINDIR}/body: ${BUILDIR} ${BINDIR} ../file/build/fileUtils.o ../sd/build/sd.o ../inf/build/inf.o ${BUILDDIR}/body.o ${BUILDDIR}/img.o ${BUILDDIR}/anim.o ${BUILDDIR}/coll.o 
	$(CC) -g ../file/build/fileUtils.o ../sd/build/sd.o ../inf/build/inf.o ${BUILDDIR}/img.o ${BUILDDIR}/anim.o ${BUILDDIR}/coll.o ${BUILDDIR}/body.o -L../../lib/ -lbotox -lfastjson $(shell pkg-config --libs zlib) $(shell sdl2-config --libs) $(shell libpng-config --libs) -o $@
	touch $@

# Compiler
${BUILDDIR}/%.o: ./src/%.c ./src/include/%.h ${BUILDDIR}
	$(CC) -g $(shell sdl2-config --cflags) $(shell pkg-config --cflags zlib) $(shell libpng-config --cflags) -I../sd/src/include -I./src/include -I../inf/src/include -I../../src/include -I../file/src/include/ -I${HOME}/hack/jollybean/src/include -I/usr/include/libfastjson -O0 -c $< -o $@
	touch $@

../file/build/fileUtils.o: ../file/src/fileUtils.c ../file/src/include/fileUtils.h
	make -C ../file

../sd/build/sd.o: ../sd/src/sd.c ../sd/src/include/sd.h
	make -C ../sd

../inf/build/inf.o: ../inf/src/inf.c ../inf/src/include/inf.h
	make -C ../inf

# Build directory (dependency)
${BUILDDIR}:
	mkdir -p ${BUILDDIR}

${BINDIR}:
	mkdir -p ${BINDIR}

.PHONY: Makefile
	make

# Cleaner
.PHONY: clean
clean:
	rm -rf ${BUILDDIR}
	rm -rf ${BINDIR}
