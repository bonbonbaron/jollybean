# Target: body

# Variables
CC             = cc
SRCDIR         = ./src
BUILDDIR       = ./build
BINDIR         = ./bin
INSTALLDIR     = /usr/local/lib

JB = $(shell git rev-parse --show-toplevel)
SRC = $(JB)/src
KR  = ${HOME}/jb/src/Keyring

OBJS := ../file/build/fileUtils.o ../sd/build/sd.o ../inf/build/inf.o ${BUILDDIR}/body.o ${BUILDDIR}/img.o ${BUILDDIR}/anim.o ${BUILDDIR}/coll.o ${BUILDDIR}/previewImg.o 

INCS := -I../sd/src/include -I./src/include -I../inf/src/include -I$(SRC)/data/include -I$(SRC)/x/include -I$(SRC)/interface/include -I$(SRC)/include -I$(KR) -I../file/src/include/ -I/usr/include/libfastjson 

# Linker
${BINDIR}/body: $(OBJS)
	$(CC) --coverage -g $(OBJS) -L${JB}/lib/ -ljb -lfastjson -lpthread $(shell pkg-config --libs zlib) $(shell sdl2-config --libs) $(shell libpng-config --libs) -o $@
	touch $@

# Compiler
${BUILDDIR}/%.o: ./src/%.c ./src/include/%.h 
	$(CC) --coverage -g $(INCS) $(shell sdl2-config --cflags) $(shell pkg-config --cflags zlib) $(shell libpng-config --cflags) -O0 -c $< -o $@
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
	rm -rf ${BUILDDIR}/*
	rm -rf ${BINDIR}/*
