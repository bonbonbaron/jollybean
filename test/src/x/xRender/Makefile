CC=gcc

RPO_DIR := $(shell git rev-parse --show-toplevel)
LIB_DIR := $(RPO_DIR)/lib
_INCS   := $(shell find ${RPO_DIR}/src -type d -name 'include')
INCS    := -I. $(_INCS:%=-I%) -Iinclude

SRCS=$(shell find . -type f -name "*.c")
OBJS=$(SRCS:%.c=%.o)

all: o

o: ${OBJS} 
	$(CC) --coverage -g ${OBJS} -L${LIB_DIR} $(shell sdl2-config --libs) -ljb -lpthread -o $@

%.o: %.c
	$(CC) --coverage -g $(INCS) $(shell sdl2-config --cflags) -c $< -o $@

$(LIB_DIR)/libjb:
	make -C ${RPO_DIR}

.PHONY: clean
clean:
	rm -rf o 2>/dev/null
	rm -rf coverage 2>/dev/null
	rm -f ${OBJS} 2>/dev/null
