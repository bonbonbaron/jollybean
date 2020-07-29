CC=gcc
CFLAGS = -L/usr/local/lib/ -lSDL2main -lSDL2 -I/usr/local/include/SDL2/ -I/home/bonbonbaron/Desktop/ge/
DEPS = ge.h
OBJS = main.o ctrl.o data.o image.o scene.o event.o
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
ge: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)
