include $(shell git rev-parse --show-toplevel)/Makefile

all: o

o: build/main.o build/objParser.o build/eb.o build/vec3.o objParser.c eb.c main.c ../../lib/libjb.a
	gcc --coverage -g -Wall build/*.o -lz -L../../lib -ljb -o $@

build/%.o: %.c 
	gcc --coverage -g -Wall -I../../include -c $< -Iinclude -o $@

.PHONY: this-clean
this-clean:
	rm -f o
	rm -f build/*
