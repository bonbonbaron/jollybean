CC=gcc

RPO_DIR := $(shell git rev-parse --show-toplevel)
SRC_DIR := $(RPO_DIR)/src
BLD_DIR := $(RPO_DIR)/build/src

D_SRCS  := $(SRC_DIR)/data/strip.c $(SRC_DIR)/data/inflatable.c $(SRC_DIR)/data/bt.c $(SRC_DIR)/data/map.c $(SRC_DIR)/data/mail.c $(SRC_DIR)/data/fray.c $(SRC_DIR)/data/array.c  $(SRC_DIR)/data/mem.c

# Implemented systems must come before their dependency, x.c.
XI_SRCS := $(shell find $(SRC_DIR)/x -type f -name "x[^.]*.c") $(shell find $(SRC_DIR)/x -type f -name "x.c") $(shell find $(SRC_DIR)/interface -type f -name "*.c")

#SRCS    := $(XI_SRCS) $(D_SRCS)   # TODO uncomment this when ready to test X with data
SRCS    := $(D_SRCS) 

OBJS    := $(SRCS:$(SRC_DIR)/%.c=$(BLD_DIR)/%.o)

_INCS   := $(shell find ${SRC_DIR} -type d -name include)
INCS    := $(_INCS:%=-I%)

DEP_DIR := $(BLD_DIR)/.deps
DEPFLGS  = -MT $@ -MMD -MP -MF
DEPS    := $(SRCS:$(SRC_DIR)/%.c=$(DEP_DIR)/%.d)

# Sentinel files that ensure the required subdirectories exist in build.
BLD_SUB := $(dir $(OBJS))
BLD_SEN := $(BLD_SUB:%=%.sentinel.bldsnl)

DEP_SUB := $(BLD_SUB:$(BLD_DIR)%=$(DEP_DIR)%)
DEP_SEN := $(DEP_SUB:%=%.sentinel.depsnl)

SDLFLGS := $(shell sdl2-config --cflags)
TGT=${RPO_DIR}/lib/libjb.a

#all: ; echo ${OBJS}
all: $(TGT)

$(TGT): $(OBJS)
	ar rcs $(TGT) $(OBJS) 

$(BLD_DIR)/interface/%.o: ${SRC_DIR}/interface/%.c 
$(BLD_DIR)/interface/%.o: ${SRC_DIR}/interface/%.c $(DEP_DIR)/interface/%.d | ${BLD_SEN} ${DEP_SEN}
	$(CC) --coverage -g $(SDLFLGS) $(DEPFLGS) $(DEP_DIR)/interface/$*.d $(INCS) -c $< -o $@

$(BLD_DIR)/x/%.o: ${SRC_DIR}/x/%.c
$(BLD_DIR)/x/%.o: ${SRC_DIR}/x/%.c $(DEP_DIR)/x/%.d | ${BLD_SEN} ${DEP_SEN}
	$(CC) --coverage -g $(SDLFLGS) $(DEPFLGS) $(DEP_DIR)/x/$*.d $(INCS) -c $< -o $@

$(BLD_DIR)/data/%.o: ${SRC_DIR}/data/%.c
$(BLD_DIR)/data/%.o: ${SRC_DIR}/data/%.c $(DEP_DIR)/data/%.d | ${BLD_SEN} ${DEP_SEN}
	$(CC) --coverage -g $(DEPFLGS) $(DEP_DIR)/data/$*.d $(INCS) -c $< -o $@

# Mention each dependency as a target so Make doesn't fail above if it doesn't exist.
$(DEPS):
# Include each dependency Makefile (named *.d). Wildcard prevents failing on nonexistent files.
include $(wildcard $(DEPS))

.PRECIOUS: %.bldsnl
%.bldsnl:
	mkdir -p $(dir $@)
	touch $@

.PRECIOUS: %.depsnl
%.depsnl:
	mkdir -p $(dir $@)
	touch $@

.PHONY: clean
clean:
	rm -rf build/*
	rm -rf build/.[a-z]*
	rm -f *.o 2>/dev/null
	rm -f ${TGT}
	rm -f ${OBJS}
