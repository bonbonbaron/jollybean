##############################
# Variables
##############################
CC             = cc
SRCDIR         = ./src
BUILDDIR       = ./build
LIBDIR         = ./lib
INCLUDEDIR     = ./src/include
INSTALLDIR     = /usr/local/lib
LIBJOLLBEAN       = ./lib/libjollybean.a
JOLLBEANSRC      = $(shell find ./src -name "*.c") 
#$(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))
JOLLBEAN_OBJS     = $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(JOLLBEANSRC:.c=.o))
JOLLBEAN_INCLUDES = $(shell find -name "*.h")
TGTINCLUDDIR = /usr/local/include/jollybean/
IFLAGS=-I./src/include/

CFLAGS_COMMON  = -Wall \
							   $(IFLAGS) -ffunction-sections \
							 	 -fdata-sections -s -fno-ident -fmerge-all-constants \
							 	 -fomit-frame-pointer -fno-stack-protector -O0
							 	 #-fomit-frame-pointer -fno-stack-protector -O3  #-mfpu=neon -O3
CFLAGS_TINY    = $(CFLAGS_COMMON) #-Os    #TODO: uncomment when ready for relase
CFLAGS_FAST    = $(CFLAGS_COMMON) #-Ofast    #TODO: uncomment when ready for relase
SDL_CFLAGS     = $(shell sdl2-config --cflags)
SDL_LFLAGS     = $(shell sdl2-config --libs)
LFLAGS         = -Wl,--gc-sections -Wl,-z,norelro

MAIN_SRC = $(BUILDDIR)/main.c
MAIN_OUT = ./game

##############################
# Archiver
##############################
$(LIBJOLLBEAN): $(BUILDDIR) $(LIBDIR) $(JOLLBEAN_OBJS) $(JOLLBEAN_INCLUDES)
	ar rcs $(LIBJOLLBEAN) $(JOLLBEAN_OBJS)
	touch $@

##############################
# Compiler
##############################
$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(JOLLBEAN_INCLUDES)
	$(CC) $(SDL_CFLAGS) $(CFLAGS_FAST) -g -c $< -o $@
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
