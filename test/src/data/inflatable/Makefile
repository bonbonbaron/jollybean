CC=gcc

RPO_DIR := $(shell git rev-parse --show-toplevel)
LIB_DIR := $(RPO_DIR)/lib
UTL_DIR := $(RPO_DIR)/utils
INCS    := -I${RPO_DIR}/include -I$(RPO_DIR)/test/include

SRCS    := $(shell find . -type f -name "*.c")
OBJS    := $(SRCS:%.c=%.o) $(UTL_DIR)/build/inf.o

all: o 

# Linker
o: ${OBJS} $(LIB_DIR)/libjb.a
	rm -rf coverage/*
	$(CC) --coverage -g ${OBJS} -L${LIB_DIR} -lz -ljb -o $@

# Compiler
%.o: %.c
	$(CC) --coverage -g $(INCS) -c $< -o $@

$(UTL_DIR)/build/inf.o:
	make -C $(UTL_DIR)

$(LIB_DIR)/libjb.a:
	make -C ${RPO_DIR}

.PHONY: clean
clean:
	rm -rf o 2>/dev/null
	rm -rf coverage 2>/dev/null
	rm -f ${OBJS} 2>/dev/null
