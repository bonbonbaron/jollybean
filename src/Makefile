CC=gcc
CFLAGS = -lSDL2main -lSDL2 -I/usr/include/SDL2/ -I/home/bonbonbaron/hack/jollybean/src/
DEPS = jb.h
OBJS = main.o ctrl.o data.o image.o scene.o event.o
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
ge: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)
