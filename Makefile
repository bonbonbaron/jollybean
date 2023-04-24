CC=gcc

RPO_DIR := $(shell git rev-parse --show-toplevel)
SRC_DIR := $(RPO_DIR)/src
BLD_DIR := $(RPO_DIR)/build/src
DEP_DIR := $(BLD_DIR)/.deps

D_SRCS := $(shell find $(SRC_DIR)/data -type f -name "*.c")
IX_SRCS  := $(shell find $(SRC_DIR)/x $(SRC_DIR)/interface -type f -name "*.c")  # separate due to sdl cflags

SRCS    := $(IX_SRCS) $(D_SRCS) 
#SRCS    := $(DX_SRCS) 
OBJS    := $(SRCS:$(SRC_DIR)/%.c=$(BLD_DIR)/%.o)

_INCS   := $(shell find ${SRC_DIR} -type d -name include)
INCS    := $(_INCS:%=-I%)

DEPFLGS  = -MT $@ -MMD -MP -MF $(DEP_DIR)/$*.d
DEPS    := $(SRCS:$(SRC_DIR)/%.c=$(DEP_DIR)/%.d)

# Sentinel files that ensure the required subdirectories exist in build.
BLD_SUB := $(dir $(OBJS))
BLD_SEN := $(BLD_SUB:%=%.sentinel.bldsnl)

DEP_SUB := $(BLD_SUB:$(BLD_DIR)%=$(DEP_DIR)%)
DEP_SEN := $(DEP_SUB:%=%.sentinel.depsnl)

TGT=libjb

#all: ; echo ${OBJS}
all: $(TGT)

$(TGT): $(OBJS)
	ar rcs $(TGT) ${OBJS}

# Get rid of default implicit recipe for object files.
# Force directories to exist before doing this.
$(BLD_DIR)/interface/%.o: ${SRC_DIR}/interface/%.c
$(BLD_DIR)/interface/%.o: ${SRC_DIR}/interface/%.c $(DEP_DIR)/interface/%.d | ${BLD_SEN} ${DEP_SEN}
	$(CC) -g $(shell sdl2-config --cflags) $(DEPFLGS) $(INCS) -c $< -o $@

$(BLD_DIR)/x/%.o: ${SRC_DIR}/x/%.c
$(BLD_DIR)/x/%.o: ${SRC_DIR}/x/%.c $(DEP_DIR)/x/%.d | ${BLD_SEN} ${DEP_SEN}
	$(CC) -g $(shell sdl2-config --cflags) $(DEPFLGS) $(INCS) -c $< -o $@

$(BLD_DIR)/data/%.o: ${SRC_DIR}/data/%.c
$(BLD_DIR)/data/%.o: ${SRC_DIR}/data/%.c $(DEP_DIR)/data/%.d | ${BLD_SEN} ${DEP_SEN}
	$(CC) -g $(DEPFLGS) $(INCS) -c $< -o $@

# Mention each dependency as a target so Make doesn't fail above if it doesn't exist.
$(DEPS):
# Include each dependency Makefile (named *.d). Wildcard prevents failing on nonexistent files.
include $(wildcard $(DEPS))

%.bldsnl:
	mkdir -p $(BLD_SUB)
	touch $(BLD_SEN)

%.depsnl:
	mkdir -p $(DEP_SUB)
	touch $(DEP_SEN)

.PHONY: clean
clean:
	rm -rf build/*
	rm -rf build/.[a-z]*
	rm -f *.o 2>/dev/null
	rm -f ${TGT}
	rm -f ${OBJS}
