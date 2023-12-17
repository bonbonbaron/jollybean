# Straight forward Makefile to compile all examples in a row

o: gles2Hack.c gles2Hack.h laMath.c laMath.h read-png.c read-png.h array.c array.h mem.c mem.h common.h vert-shader.vert frag-shader.vert
	gcc -g -lGLESv2 -lEGL -lm -lpng -lX11 gles2Hack.c verts.c laMath.c read-png.c array.c mem.c -o $@ 

clean:
	rm o

