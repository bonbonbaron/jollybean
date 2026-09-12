include $(shell git rev-parse --show-toplevel)/Makefile

all: o

o: build/main.o build/xmlParser.o build/eb.o build/vec3.o xmlParser.c eb.c main.c ../../lib/libjb.a
	gcc --coverage -g -Wall build/*.o $(shell pkg-config libxml-2.0 --libs) -lz -L../../lib -ljb -o $@

build/%.o: %.c 
	gcc --coverage -g -Wall -I../../include $(shell pkg-config libxml-2.0 --cflags) -c $< -Iinclude -o $@

.PHONY: this-clean
this-clean:
	rm -f o
	rm -f build/*
