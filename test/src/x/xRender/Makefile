CC=gcc

RPO_DIR := $(shell git rev-parse --show-toplevel)
LIB_DIR := $(RPO_DIR)/lib
_INCS   := $(shell find ${RPO_DIR}/src -type d -name 'include')
INCS    := -I. $(_INCS:%=-I%) -Iinclude

SRCS=$(shell find . -type f -name "*.c")
OBJS=$(SRCS:%.c=%.o)

all: o

o: ${OBJS} ${SRCS}
	$(CC) --coverage -g ${OBJS} -ljb -L${LIB_DIR} -L/usr/lib/aarch64-linux-gnu $(shell sdl2-config --libs) -lpthread -o $@

src/%.o: src/%.c include/%.h
	$(CC) --coverage -g $(INCS) $(shell sdl2-config --cflags) -c $< -o $@
	
%.o: %.c
	$(CC) --coverage -g $(INCS) $(shell sdl2-config --cflags) -c $< -o $@

src/%.c :
	${RPO_DIR}/utils/body/bin/body red bleh heck
	cp ${HOME}/jb/build/Seed/Genome/Gene/Body/Image/src/$<.c src
	cp ${HOME}/jb/build/Seed/Genome/Gene/Body/Image/Colormap/src/$<.c src
	cp ${HOME}/jb/build/Seed/Genome/Gene/Body/Image/ColorPalette/src/$<.c src

include/%.h:
	cp ${HOME}/jb/build/Seed/Genome/Gene/Body/Image/include/$<.h include 
	cp ${HOME}/jb/build/Seed/Genome/Gene/Body/Image/Colormap/include/$<.h include 
	cp ${HOME}/jb/build/Seed/Genome/Gene/Body/Image/ColorPalette/include/$<.h include 

$(LIB_DIR)/libjb:
	make -C ${RPO_DIR}

.PHONY: clean
clean:
	rm -rf o 2>/dev/null
	rm -rf coverage 2>/dev/null
	rm -f ${OBJS} 2>/dev/null
