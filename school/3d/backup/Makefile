# Straight forward Makefile to compile all examples in a row

o: gles2Hack.c gles2Hack.h laMath.c laMath.h
	gcc -g -lGLESv2 -lEGL -lm -lX11 gles2Hack.c laMath.c -o $@ 

clean:
	rm o

