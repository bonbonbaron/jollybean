o: main.c xmlParser.c eb.c include/eb.h include/xmlParser.h 
	gcc -g -Wall $(shell pkg-config libxml-2.0 --cflags --libs) -lz *.c -Iinclude -o o 

clean:
	rm o
