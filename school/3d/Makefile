# Straight forward Makefile to compile all examples in a row

o: gles2Hack.c gles2Hack.h laMath.c laMath.h read-png.c read-png.h vert-shader.vert frag-shader.vert jb
	gcc -g -I../../src/data/include/ -lGLESv2 -lEGL -lm -lpng -lX11 gles2Hack.c verts.c laMath.c read-png.c -L../../lib -ljb -o $@ 

jb:
	make -C ../../

clean:
	rm o

