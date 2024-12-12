# This is to make including these tools less painful in unit tests.
RPO_DIR := $(shell git rev-parse --show-toplevel)

BODY_SRCS := src/body/anim.c src/body/body.c src/body/coll.c src/body/img.c src/body/previewImg.c
BODY_OBJS := build/anim.o build/body.o build/coll.o build/img.o build/previewImg.o

OBJS := $(BODY_OBJS) build/fileUtils.o build/inf.o build/sd.o
INC := -I./include -I$(RPO_DIR)/src/data/include -I$(RPO_DIR)/src/interface/include -I$(RPO_DIR)/src/x/include -I/usr/include/libfastjson -I$(RPO_DIR)/src/include

all: $(OBJS) build/body

# Linker
build/body: $(OBJS)
	$(CC) -Wall --coverage -g $(OBJS) -L${JB}/lib/ -ljb -lfastjson -lpthread $(shell pkg-config --libs zlib) $(shell sdl2-config --libs) $(shell libpng-config --libs) -o $@

# Body
build/body.o: src/body/body.c 
	$(CC) -Wall -c --coverage -g $< $(shell sdl2-config --cflags) $(INC) -o $@

build/anim.o: src/body/anim.c
	$(CC) -Wall -c --coverage -g $< $(shell sdl2-config --cflags) $(INC) -o $@

build/img.o: src/body/img.c
	$(CC) -Wall -c --coverage -g $< $(shell sdl2-config --cflags) $(INC) -o $@

build/coll.o: src/body/coll.c
	$(CC) -Wall -c --coverage -g $< $(shell sdl2-config --cflags) $(INC) -o $@

build/previewImg.o: src/body/previewImg.c
	$(CC) -Wall -c --coverage -g $< $(shell sdl2-config --cflags) $(INC) -o $@

# File
build/fileUtils.o: src/file/fileUtils.c
	$(CC) -Wall -c --coverage -g $< $(INC) -o $@

# Inflatable
build/inf.o: src/inf/inf.c
	$(CC) -Wall -c --coverage -g $< $(INC) -o $@

# StripData
build/sd.o: src/sd/sd.c
	$(CC) -Wall -c --coverage -g $< $(INC) -o $@

clean:
	rm build/*
