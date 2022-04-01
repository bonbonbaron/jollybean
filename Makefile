##############################
# Variables
##############################
CC=cc
LIBJOLLYBEAN=libjollybean.a
JOLLYBEAN_SRC  = $(shell find ./src/ -name "*.c") 
JOLLYBEAN_OBJS = $(shell basename -a $(JOLLYBEAN_SRC:%.c=%.o))
JOLLYBEAN_INCLUDES = $(shell find -name "*.h")
BUILDDIR=./build
LIBDIR=./lib
INCLUDEDIR=./src/include
INSTALLDIR=/usr/lib

CFLAGS_COMMON = -Wall \
								-lbotox \
								-lyoyo \
							  -I$(INCLUDEDIR) -ffunction-sections \
								-fdata-sections -s -fno-ident -fmerge-all-constants \
								-fomit-frame-pointer -fno-stack-protector 
CFLAGS_TINY   = $(CFLAGS_COMMON) #-Os    #TODO: uncomment when ready for relase
CFLAGS_FAST   = $(CFLAGS_COMMON) -Ofast    #TODO: uncomment when ready for relase
SDL_LFLAGS    = $(shell sdl2-config --libs)
LFLAGS        = -Wl,--gc-sections -Wl,-z,norelro \

##############################
# Compiler
##############################
$(LIBJOLLYBEAN): $(JOLLYBEAN_OBJS) $(JOLLYBEAN_INCLUDES)
	ar rcs $(LIBDIR)/$(LIBJOLLYBEAN) $(BUILDDIR)/*
	touch $(LIBDIR)/$(LIBJOLLYBEAN)

$(JOLLYBEAN_OBJS): $(JOLLYBEAN_SRC) $(JOLLYBEAN_INCLUDES)
	$(CC) $(CFLAGS_FAST) -c $< -o $(BUILDDIR)/$@
	touch $(BUILDDIR)/$@
	
##############################
.PHONY: install
install:
	cp $(LIBDIR)/$(LIBJOLLYBEAN) $(INSTALLDIR)

.PHONY: clean
clean:
	rm -f $(BUILDDIR)/*
	rm -f $(LIBDIR)/$(LIBJOLLYBEAN)
