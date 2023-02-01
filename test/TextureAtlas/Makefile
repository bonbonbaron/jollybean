BODY_DIR = ${HOME}/jb/build/Seed/Genome/Gene/Body

COLORMAP_DIR = ${BODY_DIR}/Graybody/Colormap
CM_SRC_DIR = ${COLORMAP_DIR}/src
CM_INC_DIR = ${COLORMAP_DIR}/include
CM_OBJ_DIR = ${COLORMAP_DIR}/obj

COLOR_PALETTE_DIR = ${BODY_DIR}/Palette/Color
CP_SRC_DIR = ${COLOR_PALETTE_DIR}/src
CP_INC_DIR = ${COLOR_PALETTE_DIR}/include
CP_OBJ_DIR = ${COLOR_PALETTE_DIR}/obj

#~/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/src/I
./o: ${CM_OBJ_DIR}/redColormap.o ${CM_OBJ_DIR}/blehColormap.o ${CM_OBJ_DIR}/heckColormap.o  ${CP_OBJ_DIR}/redColorPalette.o ${CP_OBJ_DIR}/blehColorPalette.o ${CP_OBJ_DIR}/heckColorPalette.o ./textureAtlas.o ../../build/xRender.o ../../build/x.o ../../build/data.o ../../build/interface.o
	gcc ${CM_OBJ_DIR}/redColormap.o ${CM_OBJ_DIR}/blehColormap.o ${CM_OBJ_DIR}/heckColormap.o ${CP_OBJ_DIR}/redColorPalette.o ${CP_OBJ_DIR}/blehColorPalette.o ${CP_OBJ_DIR}/heckColorPalette.o ./textureAtlas.o ../../build/interface.o $(shell sdl2-config --libs) ../../build/xRender.o ../../build/x.o ../../build/data.o -o $@
	touch $@

./textureAtlas.o: ./textureAtlas.c
	gcc -g -c textureAtlas.c -I../../src/include/ -I${CM_INC_DIR} -I${CP_INC_DIR} $(shell sdl2-config --cflags --cflags) -o $@
	touch $@

${CM_OBJ_DIR}/%Colormap.o: ${CM_SRC_DIR}/%Colormap.c
	gcc -g -I../../src/include -c $< $(shell sdl2-config --cflags) -o $@
	touch $@

${CP_OBJ_DIR}/%ColorPalette.o: ${CP_SRC_DIR}/%ColorPalette.c
	gcc -g -I../../src/include -c $< $(shell sdl2-config --cflags) -o $@
	touch $@

../../build/%.o: ../../src/%.c
	make -C ../../
	touch $@

${CP_SRC_DIR}/%ColorPalette.c:
	../../utils/body/bin/body $*
	touch $@

${CM_SRC_DIR}/%Colormap.c:
	../../utils/body/bin/body $*
	touch $@

.PHONY: clean
clean:
	rm -f ./o 2>/dev/null
	rm -f ./render.o 2>/dev/null
	rm -f ${CP_INC_DIR}/*.h 2>/dev/null
	rm -f ${CP_OBJ_DIR}/*.o 2>/dev/null
	rm -f ${CP_SRC_DIR}/*.c 2>/dev/null
	rm -f ${CM_INC_DIR}/*.h 2>/dev/null
	rm -f ${CM_OBJ_DIR}/*.o 2>/dev/null
	rm -f ${CM_SRC_DIR}/*.c 2>/dev/null
