CC=gcc

RPO_DIR := $(shell git rev-parse --show-toplevel)
LIB_DIR := $(RPO_DIR)/lib
_INCS   := $(shell find ${RPO_DIR}/src -type d -name 'include') $(RPO_DIR)/test/include
INCS    := -I. $(_INCS:%=-I%)

SRCS=$(shell find . -type f -name "*.c")
OBJS=$(SRCS:%.c=%.o)

all: o

o: ${OBJS} $(LIB_DIR)/libjb.a
	$(CC) --coverage -g ${OBJS} -L${LIB_DIR} -ljb -o $@

%.o: %.c
	$(CC) --coverage -g $(INCS) -c $< -o $@

$(LIB_DIR)/libjb.a: 
	make -C ${RPO_DIR}

.PHONY: clean
clean:
	rm -rf o 2>/dev/null
	rm -rf coverage 2>/dev/null
	rm -f ${OBJS} 2>/dev/null
