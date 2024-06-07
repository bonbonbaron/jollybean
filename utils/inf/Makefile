##############################
# Variables
##############################
CC             = cc
SRCDIR         = ./src
BUILDDIR       = ./build
LIBDIR         = ./lib
INCLUDEDIR     = ./src/include
INSTALLDIR     = /usr/local/lib

##############################
# Compiler
##############################
inf.o: ./src/inf.c ./src/include/inf.h ${BUILDDIR}
	$(CC) -g -I../../src/data/include -L../../lib -ljb $(sdl2-config --cflags) $(pkg-config --cflags zlib) -lz -c ./src/inf.c -o $@
	mv $@ build/
	touch build/$@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

#############################

.PHONY: install
install:
	mkdir -p $(TGTINCLUDDIR)
	cp $(JOLLBEAN_INCLUDES) $(TGTINCLUDDIR)
	cp $(LIBJOLLBEAN) $(INSTALLDIR)

.PHONY: clean
clean:
	rm -f $(JOLLBEAN_OBJS)
	rm -f $(LIBJOLLBEAN)
