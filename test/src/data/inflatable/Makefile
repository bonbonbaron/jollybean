CC=gcc

RPO_DIR := $(shell git rev-parse --show-toplevel)
LIB_DIR := $(RPO_DIR)/lib
INF_DIR := $(RPO_DIR)/utils/inf
INF_BLD := $(INF_DIR)/build
INF_SRC := $(INF_DIR)/src
_INCS   := $(shell find ${RPO_DIR}/src ${INF_SRC} -type d -name 'include')
INCS    := -I. $(_INCS:%=-I%)

SRCS    := $(shell find . -type f -name "*.c")
OBJS    := $(SRCS:%.c=%.o) ${INF_BLD}/inf.o

all: o 

# Linker
o: ${OBJS} $(LIB_DIR)/libjb.a
	rm -rf coverage/*
	$(CC) --coverage -g ${OBJS} -L${LIB_DIR} -lz -ljb -o $@

# Compiler
%.o: %.c
	$(CC) --coverage -g $(INCS) -c $< -o $@

$(INF_BLD)/%.o: $(INF_SRC)/%.c
	make -C $(INF_DIR)

$(LIB_DIR)/libjb.a:
	make -C ${RPO_DIR}

.PHONY: clean
clean:
	rm -rf o 2>/dev/null
	rm -rf coverage 2>/dev/null
	rm -f ${OBJS} 2>/dev/null
