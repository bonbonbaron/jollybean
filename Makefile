##############################
# Variables
##############################
CC=cc
# Directories
JBDIR = /home/$(USER)/hack/jollybean/
INSTALLDIR       = $(JBDIR)

COREDIR          = $(JBDIR)core/
ENGINEDIR        = $(JBDIR)
SYSDIR           = $(JBDIR)sys/
GAMEDIR          = $(JBDIR)game/

COREINCLUDEDIR   = $(COREDIR)include/
ENGINEINCLUDEDIR = $(ENGINEDIR)/
SYSINCLUDEDIR    = $(SYSDIR)include/
GAMEINCLUDEDIR   = $(GAMEDIR)include/
# Input files
CORE_INCLUDES    = $(COREINCLUDEDIR)*.h
ENGINE_INCLUDES  = $(ENGINEINCLUDEDIR)*.h
SYS_INCLUDES     = $(SYSINCLUDEDIR)*.h
GAME_INCLUDES    = $(GAMEINCLUDEDIR)*.h
# Compile options
IFLAGS        = -I$(COREINCLUDEDIR) -I$(ENGINEINCLUDEDIR) -I$(SYSINCLUDEDIR) -I$(GAMEINCLUDEDIR) 
SDL_CFLAGS    = $(shell sdl2-config --cflags)
CFLAGS_COMMON = -g -Wall $(SDL_CFLAGS) \
							  $(IFLAGS) -ffunction-sections \
								-fdata-sections -s -fno-ident -fmerge-all-constants \
								-fomit-frame-pointer -fno-stack-protector 
CFLAGS_TINY   = $(CFLAGS_COMMON) #-Os    #TODO: uncomment when ready for relase
CFLAGS_FAST   = $(CFLAGS_COMMON) #-Ofast    #TODO: uncomment when ready for relase
SDL_LFLAGS    = $(shell sdl2-config --libs)
LFLAGS        = -Wl,--gc-sections -Wl,-z,norelro \
								-Wl,--build-id=none $(SDL_LFLAGS)
# Source files
CORE_SRC      = $(shell find $(COREDIR) -name "*.c") 
ENGINE_SRC    = $(shell find $(ENGINEDIR) -name "jb.c")
SYS_SRC       = $(shell find $(SYSDIR) -name "*.c") 
GAME_SRC      = $(shell find $(GAMEDIR) -name "*.c") 

# Output files
CORE_OBJS   = $(CORE_SRC:%.c=%.o)
ENGINE_OBJS = $(ENGINE_SRC:%.c=%.o)
SYS_OBJS    = $(SYS_SRC:%.c=%.o)
GAME_OBJS   = $(GAME_SRC:%.c=%.o)
OBJS        = $(ENGINE_OBJS) $(GAME_OBJS) $(CORE_OBJS) $(SYS_OBJS)

OUTPUTFILE  = o

##############################
# Linker
##############################
$(OUTPUTFILE): $(OBJS)
	#ar rc jb.a $(OBJS)
	$(CC)  $(OBJS) $(LFLAGS) -o $(OUTPUTFILE) 
	touch $(OUTPUTFILE)

##############################
# Compiler
##############################
$(COREDIR)%.o: $(COREDIR)%.c $(CORE_INCLUDES)
	$(CC) $(CFLAGS_FAST) $(IFLAGS) -c $< -o $@
	touch $@

$(ENGINEDIR)%.o: $(ENGINEDIR)%.c $(ENGINE_INCLUDES)
	$(CC) $(CFLAGS_FAST) $(IFLAGS) -c $< -o $@
	touch $@

$(SYSDIR)%.o: $(SYSDIR)%.c $(SYS_INCLUDES)
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
