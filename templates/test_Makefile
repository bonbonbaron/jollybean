CC=gcc

CUR_DIR := $(shell pwd)
RPO_DIR := $(shell git rev-parse --show-toplevel)
SRC_DIR := $(RPO_DIR)/src
BLD_DIR := $(RPO_DIR)/build/test
DEP_DIR := $(BLD_DIR)/.deps
DEPFLGS  = -MT $@ -MMD -MP -MF $(DEP_DIR)/$*.d
_SRCS    = data/array.c data/mem.c
SRCS    := $(CUR_DIR)/main.c $(_SRCS:%.c=$(SRC_DIR)/%.c)
OBJS    := $(CUR_DIR)/main.o $(_SRCS:%.c=$(BLD_DIR)/%.o)
BLD_SUB := $(dir $(OBJS))
DEP_SUB := $(BLD_SUB:$(BLD_DIR)%=$(DEP_DIR)%)
_INCS   := $(shell find ${SRC_DIR} -type d -name include)
INCS    := $(_INCS:%=-I%)
DEPS    := $(_SRCS:%.c=$(DEP_DIR)/%.d)
TGT=o

# Sentinel files that ensure the required subdirectories exist in build.
BLD_SEN := $(BLD_SUB:%=%.sentinel.bldsnl)
DEP_SEN := $(DEP_SUB:%=%.sentinel.depsnl)
#all: ; echo ${INCS}
all: $(TGT)

$(TGT): $(OBJS)
	$(CC) $^ -o $@

$(CUR_DIR)/%.o: $(CUR_DIR)/%.c
	$(CC) -g $(DEPFLGS) $(INCS) -c $< -o $@

# Get rid of default implicit recipe for object files.
$(BLD_DIR)/data/%.o: ${SRC_DIR}/data/%.c
# Force directories to exist before doing this.
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
