o: build/main.o build/xmlParser.o build/eb.o build/fray.o build/array.o build/vec3.o build/mem.o main.c xmlParser.c eb.c main.c libjb.a
	gcc -g -Wall build/*.o $(shell pkg-config libxml-2.0 --libs) -lz -L../../lib -ljb -o $@

build/%.o: %.c 
	gcc -g -Wall -I../../include $(shell pkg-config libxml-2.0 --cflags) -c $< -Iinclude -o $@

.PHONY: clean
clean:
	rm -f o
	rm -f build/*
