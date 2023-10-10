# Target: fileUtils
RPODIR := $(shell git rev-parse --show-toplevel)
SRCDIR := ${RPODIR}/src/data
INCDIR := ${SRCDIR}/include
BLDDIR := ${RPODIR}/build
INCS   := ${INCDIR}/mem.h ${INCDIR}/common.h 
./build/fileUtils.o: ./src/fileUtils.c ./src/include/fileUtils.h ${INCS} ./build
	gcc -g -lfastjson -I/usr/include/libfastjson -I${INCDIR} -I./src/include ./src/fileUtils.c -c -o $@

# Build directory (dependency)
./build:
	mkdir -p ./build

# Cleaner
.PHONY: clean
clean:
	rm -rf ./build
