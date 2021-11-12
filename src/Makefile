CC=cc
#Directories
SRCDIR = /home/bonbonbaron/hack/jollybean/src/
GENEDIR = /home/bonbonbaron/hack/jollybean/genes/
BUILDDIR = /home/bonbonbaron/hack/jollybean/build/
INSTALLDIR = /home/bonbonbaron/hack/jollybean/
JB_INCLUDE = $(SRCDIR)include/
INCLUDES = -I/home/bonbonbaron/hack/jollybean/src/include/ -I/home/bonbonbaron/hack/jollybean/genes/include/ 

#CFLAGS = -lSDL2main -lSDL2 -I/usr/include/SDL2/ -I/home/bonbonbaron/hack/jollybean/src/include/
CFLAGS = `pkg-config --cflags --libs sdl2` -I/usr/include/SDL2/
DEPS = $(shell find $(JB_INCLUDE) -name "*.h") $(shell find $(GENEDIR) -name "*.h") 
SRCS = $(shell find $(SRCDIR) -name "*.c") $(shell find $(GENEDIR) -name "*.c") 
#OBJS = $(addprefix $(BUILDDIR)/,$(SRCS:%.c=%.o))
OBJS = $(SRCS:%.c=%.o)
OUTPUTFILE = o
%.o: %.c 
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -g -o $@ 
	touch $@

#Actual thing being done
CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS = -Wl,--gc-sections
all: $(OBJS)   #jb colon main.o ctrl.o data.o image.o scene.o.... so it compiles each *.o file then links here.
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(INCLUDEDIR) $(INCLUDES) $(OBJS) -g -o $(OUTPUTFILE)
	touch $(OUTPUTFILE)

#	echo "lets to linking"
#	cd $(SRCDIR)
#	mv ./*.o $(BUILDDIR)
#	mv $(OUTPUTFILE) $(BUILDDIR)

#.PHONY: all
#all: $(OUTPUTFILE)
#$(OUTPUTFILE): $(OBJS)
#	ar ru $@ $^
#	ranlib $@
#clean:
#	rm *.o

#.PHONY: install
#install:
#	cp -p $(OUTPUTFILE) $(INSTALLDIR)

clean:
	rm $(BUILDDIR)*.o
	rm $(SRCDIR)$(OUTPUTFILE)


