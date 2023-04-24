##############################
# Variables
##############################
CC                = gcc
SRCDIR            = ./src
BUILDDIR          = ./build
LIBDIR            = ./lib
REQUIRED_OBJS     = $(shell find src -name "*.c" -exec basename {} \; | sed -n -E 's,(.*)\.c,\./build/\1\.o,gp')
INCLUDEDIR        = ./src/include
INSTALLDIR        = /usr/local/lib
LIBJOLLBEAN       = ./lib/libjollybean.a
JOLLBEANSRC       = $(shell find ./src -name "*.c") 
JOLLBEAN_INCLUDES = $(shell find -name "*.h")
TGTINCLUDDIR      = /usr/local/include/jollybean/
IFLAGS            = -I./src/include/ -I ./src/data/include -I./src/interface/include -I./src/x/include -I/home/bonbonbaron/jb/src/Keyring/

# short-enums will make the enums as few bytes as possible to contain full range of vaulues.
# This enables type-safety of enums in functions.
CFLAGS_COMMON     = -fshort-enums#-ffunction-sections \
							    	 -fdata-sections -s -fno-ident -fmerge-all-constants \
							    	 -fomit-frame-pointer -fno-stack-protector -O0
							    	 #-fomit-frame-pointer -fno-stack-protector -O3  #-mfpu=neon -O3
CFLAGS_TINY       = $(CFLAGS_COMMON) #-Os    #TODO: uncomment when ready for relase
CFLAGS_FAST       = $(CFLAGS_COMMON) #-Ofast    #TODO: uncomment when ready for relase
SDL_CFLAGS        = $(shell sdl2-config --cflags)
SDL_LFLAGS        = $(shell sdl2-config --libs)
LFLAGS            = -Wl,--gc-sections -Wl,-z,norelro

MAIN_SRC = $(BUILDDIR)/main.c
MAIN_OUT = ./game

##############################
# Archiver
##############################
$(LIBJOLLBEAN): $(BUILDDIR) $(LIBDIR) $(REQUIRED_OBJS)
	ar rcs $(LIBJOLLBEAN) $(JOLLBEAN_OBJS)
	touch $@

##############################
# Compiler
##############################
$(BUILDDIR)/%.o: ./src/data/%.c ./src/data/include/%.h
	$(CC) -g -Wall $(SDL_CFLAGS) $(CFLAGS_FAST) $(IFLAGS) -g -c $< -o $@
	touch $@

$(BUILDDIR)/%.o: ./src/interface/%.c ./src/interface/include/%.h
	$(CC) -g -Wall $(SDL_CFLAGS) $(CFLAGS_FAST) $(IFLAGS) -g -c $< -o $@
	touch $@

$(BUILDDIR)/%.o: ./src/x/%.c ./src/x/include/%.h
	$(CC) -g -Wall $(SDL_CFLAGS) $(CFLAGS_FAST) $(IFLAGS) -g -c $< -o $@
	touch $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(LIBDIR):
	mkdir -p $(LIBDIR)
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
