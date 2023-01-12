# Target: genie

# Variables
CC             = cc
SRCDIR         = ./src
BUILDDIR       = ./build
BINDIR         = ./bin
INSTALLDIR     = /usr/local/lib

# Compiler
${BUILDDIR}/genie.o: ./src/genie.c ./src/include/genie.h ../db/build/db.o ${BUILDDIR}
	$(CC) -g -I../db/src/include -I./src/include -I../../src/include -I../file/src/include/ -I${HOME}/hack/jollybean/src/include -O0 -c ./src/genie.c -o $@
	touch $@

../db/build/db.o:
	make -C ../db/

# Build db (dependency)
${BUILDDIR}:
	mkdir -p ${BUILDDIR}

# Cleaner
.PHONY: clean
clean:
	rm -rf ${BUILDDIR}
