# Target: genie

# Variables
CC             = cc
SRCDIR         = ./src
BUILDDIR       = ./build
BINDIR         = ./bin
INSTALLDIR     = /usr/local/lib

# Compiler
${BUILDDIR}/genie.o: ./src/genie.c ./src/include/genie.h ../dir/build/dir.o ${BUILDDIR}
	$(CC) -g -I../dir/src/include -I./src/include -I${HOME}/hack/botox/src/include -I../file/src/include/ -I${HOME}/hack/jollybean/src/include -O0 -c ./src/genie.c -o $@
	touch $@

../dir/build/dir.o:
	make -C ../dir/

# Build directory (dependency)
${BUILDDIR}:
	mkdir -p ${BUILDDIR}

# Cleaner
.PHONY: clean
clean:
	rm -rf ${BUILDDIR}
