##############################
# Variables
##############################
CC=cc
# Directories
SRC_DIR = /home/$(USER)/hack/jollybean/src/

BOTOX_DIR        = $(SRC_DIR)botox/
ENGINE_DIR       = $(SRC_DIR)engine/
FRAMEWORK_DIR    = $(SRC_DIR)framework/
X_DIR            = $(SRC_DIR)x/

BOTOX_INC_DIR     = $(BOTOX_DIR)include/
ENGINE_INC_DIR    = $(ENGINE_DIR)include/
FRAMEWORK_INC_DIR = $(FRAMEWORK_DIR)include/
X_INC_DIR         = $(X_DIR)include/
# Input files
BOTOX_INCLUDES     = $(BOTOX_INC_DIR)*.h
ENGINE_INCLUDES    = $(ENGINE_INC_DIR)*.h
FRAMEWORK_INCLUDES = $(FRAMEWORK_INC_DIR)*.h
X_INCLUDES         = $(X_INC_DIR)*.h
# Compile options
IFLAGS        = -I$(BOTOX_INC_DIR) -I$(FRAMEWORK_INC_DIR) -I$(X_INC_DIR) -I$(ENGINE_INC_DIR) 
SDL_CFLAGS    = $(shell sdl2-config --cflags)
CFLAGS_COMMON = -g -Wall \
							  $(IFLAGS) -ffunction-sections \
								-fdata-sections -s -fno-ident -fmerge-all-constants \
								-fomit-frame-pointer -fno-stack-protector 
CFLAGS_TINY   = $(CFLAGS_COMMON) #-Os    #TODO: uncomment when ready for relase
CFLAGS_FAST   = $(CFLAGS_COMMON) #-Ofast    #TODO: uncomment when ready for relase
SDL_LFLAGS    = $(shell sdl2-config --libs)
LFLAGS        = -Wl,--gc-sections -Wl,-z,norelro \
								-Wl,--build-id=none $(SDL_LFLAGS)
# Source files
BOTOX_SRC     = $(shell find $(BOTOX_DIR) -name "*.c") 
ENGINE_SRC    = $(shell find $(ENGINE_DIR) -name "*.c")
FRAMEWORK_SRC = $(shell find $(FRAMEWORK_DIR) -name "*.c") 
X_SRC         = $(shell find $(X_DIR) -name "*.c") 

# Output files
BOTOX_OBJS   = $(BOTOX_SRC:%.c=%.o)
X_OBJS   = $(X_SRC:%.c=%.o)
FRAMEWORK_OBJS    = $(FRAMEWORK_SRC:%.c=%.o)
ENGINE_OBJS = $(ENGINE_SRC:%.c=%.o)
OBJS        = $(BOTOX_OBJS) $(FRAMEWORK_OBJS) $(X_OBJS) $(ENGINE_OBJS)
#OBJS        = $(BOTOX_OBJS) $(X_OBJS)

OUTPUTFILE  = m

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
$(BOTOX_DIR)%.o: $(BOTOX_DIR)%.c 
	$(CC) $(CFLAGS_FAST) -c $< -o $@
	touch $@

$(FRAMEWORK_DIR)%.o: $(FRAMEWORK_DIR)%.c 
	$(CC) $(CFLAGS_FAST) -c $< -o $@
	touch $@
	
$(X_DIR)%.o: $(X_DIR)%.c 
	$(CC) $(CFLAGS_TINY) $(SDL_CFLAGS) -c $< -o $@
	touch $@

$(ENGINE_DIR)%.o: $(ENGINE_DIR)%.c 
	$(CC) $(CFLAGS_FAST) $(SDL_CFLAGS) -c $< -o $@
	touch $@


##############################
.PHONY: install
install:
	cp -p $(OUTPUTFILE) $(INSTALLDIR)

.PHONY: clean
clean:
	rm -f $(OBJS)
	rm -f $(OUTPUTFILE)
