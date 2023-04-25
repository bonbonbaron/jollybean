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
infUtil.o: ./src/infUtil.c ./src/include/infUtil.h ${BUILDDIR}
	$(CC) -g -I../../src/data/include -L../../lib -ljollybean $(sdl2-config --cflags) $(pkg-config --cflags zlib) -lz -c ./src/infUtil.c -o $@
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
