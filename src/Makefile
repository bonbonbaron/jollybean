CC=gcc
CFLAGS = -lSDL2main -lSDL2 -I/usr/include/SDL2/ -I/home/bonbonbaron/hack/jollybean/src/include/
DEPS = jb.h
OBJS = data.o main.o
OUTPUTFILE = jb
INSTALLDIR = /home/bonbonbaron/hack/jollybean/
INCLUDEDIR = /home/bonbonbaron/hack/jollybean/src/include/
%.o: %.c $(DEPS)
	$(CC) -I$(INCLUDEDIR) -c -Os -o $@ $< $(CFLAGS)  #Compile @.c ($<) outputting @.o. Include 
	touch $@
jb: $(OBJS)   #jb colon main.o ctrl.o data.o image.o scene.o.... so it compiles each *.o file then links here.
	$(CC) -o $@ $^ $(CFLAGS)

#.PHONY: all
#all: $(OUTPUTFILE)
#$(OUTPUTFILE): $(OBJS)
#	ar ru $@ $^
#	ranlib $@
clean:
	rm *.o

.PHONY: install
install:
	cp -p $(OUTPUTFILE) $(INSTALLDIR)


