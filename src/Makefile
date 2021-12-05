##############################
# Variables
##############################
CC=cc
# Directories
JBDIR = /home/$(USER)/hack/jollybean/
INSTALLDIR       = $(JBDIR)
ENGINEDIR        = $(JBDIR)src/
GAMEDIR          = $(JBDIR)game/
BUILDDIR         = $(JBDIR)build/
GAMEINCLUDEDIR   = $(GAMEDIR)include/
ENGINEINCLUDEDIR = $(ENGINE)include/
# Input files
GAME_INCLUDES    = $(GAMEINCLUDEDIR)*.h
ENGINE_INCLUDES  = $(ENGINEINCLUDEDIR)*.h
# Compile options
IFLAGS        = -I$(GAMEINCLUDEDIR) -I$(ENGINEINCLUDEDIR)
SDL_CFLAGS    = $(shell sdl2-config --cflags)
CFLAGS_COMMON = -Wall $(SDL_CFLAGS) \
							  $(IFLAGS) -ffunction-sections \
								-fdata-sections -s -fno-ident -fmerge-all-constants \
								-fomit-frame-pointer -fno-stack-protector 
CFLAGS_TINY   = $(CFLAGS_COMMON) #-Os    TODO: uncomment when ready for relase
CFLAGS_FAST   = $(CFLAGS_COMMON) #-Ofast    TODO: uncomment when ready for relase
SDL_LFLAGS    = $(shell sdl2-config --libs)
LFLAGS        = -Wl,--gc-sections -Wl,-z,norelro \
								-Wl,--build-id=none $(SDL_LFLAGS)
# Source files
GAME_SRC      = $(shell find $(GAMEDIR) -name "*.c") 
ENGINE_SRC    = $(shell find $(ENGINEDIR) -name "*.c") 
# Output files
GAME_OBJS     = $(GAME_SRC:%.c=%.o)
ENGINE_OBJS   = $(ENGINE_SRC:%.c=%.o)
OBJS          = $(ENGINE_OBJS) $(GAME_OBJS) 
OUTPUTFILE    = o

##############################
# Linker
##############################
$(OUTPUTFILE): $(OBJS)
	$(CC)  $(OBJS) $(LFLAGS) -o $(OUTPUTFILE) 
	touch $(OUTPUTFILE)

##############################
# Compiler
##############################
$(ENGINEDIR)%.o: $(ENGINEDIR)%.c $(ENGINE_INCLUDES)
	$(CC) $(CFLAGS_FAST) $(IFLAGS) -c $< -o $@
	touch $@
	
$(GAMEDIR)%.o: $(GAMEDIR)%.c $(GAME_INCLUDES) $(ENGINE_INCLUDES)
	$(CC) $(CFLAGS_TINY) $(IFLAGS) -c $< -o $@
	touch $@

##############################
.PHONY: install
install:
	cp -p $(OUTPUTFILE) $(INSTALLDIR)
.PHONY: clean
clean:
	rm -f $(OBJS)
	rm -f $(OUTPUTFILE)
