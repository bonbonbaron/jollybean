# Target: img

# Variables
CC             = cc
SRCDIR         = ./src
BUILDDIR       = ./build
BINDIR         = ./bin
INSTALLDIR     = /usr/local/lib

# Linker
${BINDIR}/body: \
	../file/build/fileUtils.o \
	../genie/build/genie.o \
	../strip/build/strip.o \
	${BUILDDIR}/body.o \
	${BUILDDIR}/img.o \
	${BUILDDIR}/anim.o \
	${BUILDDIR}/coll.o \
	../dir/build/dir.o \
	../genie/build/genie.o \
	../strip/build/strip.o \
	../inflatable/build/inflatable.o \
	${BUILDIR} ${BINDIR}
	$(CC) -g \
	${BUILDDIR}/img.o \
	../file/build/fileUtils.o \
	../dir/build/dir.o \
	../strip/build/strip.o \
	../inflatable/build/inflatable.o \
	-L${HOME}/hack/botox/lib/ -lbotox \
	-lfastjson\
	$(shell pkg-config --libs zlib) \
	$(shell sdl2-config --libs) \
	$(shell libpng-config --libs) \
	-o $@
	touch $@

# Compiler
${BUILDDIR}/%.o: ./src/%.c ./src/include/%.h ${BUILDDIR}
	$(CC) -g \
		$(shell sdl2-config --cflags) \
		$(shell pkg-config --cflags zlib) \
		$(shell libpng-config --cflags) \
		-I../strip/src/include \
		-I./src/include \
		-I../inflatable/src/include \
		-I${HOME}/hack/botox/src/include \
		-I../file/src/include/ \
		-I${HOME}/hack/jollybean/src/include \
		-I../dir/src/include \
		-I../genie/src/include \
		-I/usr/include/libfastjson \
		-O0 -c $< -o $@
	touch $@

../file/build/fileUtils.o: ../file/src/fileUtils.c ../file/src/include/fileUtils.h
	make -C ../file

../strip/build/strip.o: ../strip/src/strip.c ../strip/src/include/strip.h
	make -C ../strip

../inflatable/build/inflatable.o: ../inflatable/src/inflatable.c ../inflatable/src/include/inflatable.h
	make -C ../inflatable

../dir/build/dir.o: ../dir/src/dir.c ../dir/src/include/dir.h
	make -C ../dir

../genie/build/genie.o: ../genie/src/genie.c ../genie/src/include/genie.h
	make -C ../genie

# Build directory (dependency)
${BUILDDIR}:
	mkdir -p ${BUILDDIR}

${BINDIR}:
	mkdir -p ${BINDIR}

.PHONY: Makefile
	make

# Cleaner
.PHONY: clean
clean:
	rm -rf ${BUILDDIR}
	rm -rf ${BINDIR}
