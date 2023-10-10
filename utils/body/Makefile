# Target: body

# Variables
CC             = cc
SRCDIR         = ./src
BUILDDIR       = ./build
BINDIR         = ./bin
INSTALLDIR     = /usr/local/lib

JB = $(shell git rev-parse --show-toplevel)
BLD = $(JB)/build/src
SRC = $(JB)/src
KR  = ${HOME}/jb/src/Keyring

OBJS := ../file/build/fileUtils.o ../genie/build/genie.o ../stripUtil/build/stripUtil.o ../db/build/db.o ../infUtil/build/infUtil.o ${BUILDDIR}/body.o ${BUILDDIR}/img.o ${BUILDDIR}/anim.o ${BUILDDIR}/coll.o ${BUILDDIR}/previewImg.o ${BLD}/interface/interface.o 

INCS := -I../stripUtil/src/include -I./src/include -I../infUtil/src/include -I$(SRC)/data/include -I$(SRC)/x/include -I$(SRC)/interface/include -I$(SRC)/include -I$(KR) -I../file/src/include/ -I../db/src/include -I../genie/src/include -I/usr/include/libfastjson 

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

../stripUtil/build/stripUtil.o: ../stripUtil/src/stripUtil.c ../stripUtil/src/include/stripUtil.h
	make -C ../stripUtil

../infUtil/build/infUtil.o: ../infUtil/src/infUtil.c ../infUtil/src/include/infUtil.h
	make -C ../infUtil

../db/build/db.o: ../db/src/db.c ../db/src/include/db.h
	make -C ../db

../genie/build/genie.o: ../genie/src/genie.c ../genie/src/include/genie.h
	make -C ../genie

$(BLD)/data/%.o: $(SRC)/data/%.c
	make -C $(JB)

$(BLD)/x/%.o: $(SRC)/x/%.c
	make -C $(JB)

$(BLD)/interface/%.o: $(SRC)/interface/%.c
	make -C $(JB)

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
