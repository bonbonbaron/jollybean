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
inflatable.o: ./src/inflatable.c ./src/include/inflatable.h
	$(CC) -L${HOME}/hack/botox/lib/ -lbotox $(sdl2-config --cflags) $(pkg-config --cflags zlib) -lz -c ./src/inflatable.c -o $@
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
