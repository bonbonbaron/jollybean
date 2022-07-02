CC=gcc

example: src/main.c ../build/libfluidlite.a
	${CC} -g -o $@ $< -I../include -I../libbotox/src/include -L../build/ -lfluidlite -L../libbotox/lib -lbotox -lz
