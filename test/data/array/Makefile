CC=gcc

RPO_DIR := $(shell git rev-parse --show-toplevel)
LIB_DIR := $(RPO_DIR)/lib
_INCS   := $(shell find ${RPO_DIR}/src -type d -name 'include')
INCS    := -I. $(_INCS:%=-I%)

SRCS=$(shell find . -maxdepth 1 -type f -name "*.c")
OBJS=$(SRCS:%.c=%.o)

all: o

o: ${OBJS} 
	$(CC) -std=c11 --coverage -g ${OBJS} -L${LIB_DIR} -ljb -o $@

%.o: %.c
	$(CC) -std=c11 --coverage -g $(INCS) -c $< -o $@

$(LIB_DIR)/libjb:
	make -C ${RPO_DIR}

.PHONY: clean
clean:
	rm -rf o 2>/dev/null
	rm -rf coverage 2>/dev/null
	rm -f ${OBJS} 2>/dev/null
