# Target: body

# Variables
CC             = cc
SRCDIR         = ./src
BUILDDIR       = ./build
BINDIR         = ./bin
INSTALLDIR     = /usr/local/lib

JB = ${HOME}/hack/jollybean
BLD = ${JB}/build
SRC = ${JB}/src
INC = ${SRC}/include
KR  = ${HOME}/jb/src/Keyring

# Linker
${BINDIR}/body: ../file/build/fileUtils.o ../genie/build/genie.o ../strip/build/strip.o ../db/build/db.o ../inflatable/build/inflatable.o ${BUILDDIR}/body.o ${BUILDDIR}/img.o ${BUILDDIR}/anim.o ${BUILDDIR}/coll.o ${BUILDDIR}/previewImg.o ${BLD}/interface.o ${BLD}/data.o
	$(CC) -g ../file/build/fileUtils.o ../genie/build/genie.o ../db/build/db.o ../strip/build/strip.o ../inflatable/build/inflatable.o ${BUILDDIR}/img.o ${BUILDDIR}/anim.o ${BUILDDIR}/coll.o ${BUILDDIR}/previewImg.o ${BUILDDIR}/body.o -L${JB}/lib/ -ljollybean -lfastjson -lpthread $(shell pkg-config --libs zlib) $(shell sdl2-config --libs) $(shell libpng-config --libs) -o $@
	touch $@

# Compiler
${BUILDDIR}/%.o: ./src/%.c ./src/include/%.h 
	$(CC) -g $(shell sdl2-config --cflags) $(shell pkg-config --cflags zlib) $(shell libpng-config --cflags) -I../strip/src/include -I./src/include -I../inflatable/src/include -I${INC} -I${KR} -I../file/src/include/ -I../db/src/include -I../genie/src/include -I/usr/include/libfastjson -O0 -c $< -o $@
	touch $@

../file/build/fileUtils.o: ../file/src/fileUtils.c ../file/src/include/fileUtils.h
	make -C ../file

../strip/build/strip.o: ../strip/src/strip.c ../strip/src/include/strip.h
	make -C ../strip

../inflatable/build/inflatable.o: ../inflatable/src/inflatable.c ../inflatable/src/include/inflatable.h
	make -C ../inflatable

../db/build/db.o: ../db/src/db.c ../db/src/include/db.h
	make -C ../db

../genie/build/genie.o: ../genie/src/genie.c ../genie/src/include/genie.h
	make -C ../genie

${BLD}/%.o: ${SRC}/%.c
	make -C ${JB}

# Build dbectory (dependency)
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
