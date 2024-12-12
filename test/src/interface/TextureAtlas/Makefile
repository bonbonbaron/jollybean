BODY_DIR = ${HOME}/jb/build/Seed/Genome/Gene/Body

COLORMAP_DIR = ${BODY_DIR}/Image/Colormap
CM_SRC_DIR = ${COLORMAP_DIR}/src
CM_INC_DIR = ${COLORMAP_DIR}/include
CM_OBJ_DIR = ${COLORMAP_DIR}/obj

COLOR_PALETTE_DIR = ${BODY_DIR}/Image/ColorPalette
CP_SRC_DIR = ${COLOR_PALETTE_DIR}/src
CP_INC_DIR = ${COLOR_PALETTE_DIR}/include
CP_OBJ_DIR = ${COLOR_PALETTE_DIR}/obj

JB_DIR = $(shell git rev-parse --show-toplevel)
UTILS = ${JB_DIR}/utils
KR_DIR = ${HOME}/jb/src/Keyring
LIB_DIR := $(JB_DIR)/lib


_INCS   := $(shell find ${JB_DIR}/src -type d -name include) ${CM_INC_DIR} ${CP_INC_DIR} ${UTILS}/include
INCS    := $(_INCS:%=-I%)

./o: ${CM_OBJ_DIR}/redColormap.o ${CM_OBJ_DIR}/blehColormap.o ${CM_OBJ_DIR}/heckColormap.o  ${CP_OBJ_DIR}/redColorPalette.o ${CP_OBJ_DIR}/blehColorPalette.o ${CP_OBJ_DIR}/heckColorPalette.o ./test.o ${LIB_DIR}/libjb.a ${UTILS}/build/*.o
	gcc --coverage ${CM_OBJ_DIR}/redColormap.o ${CM_OBJ_DIR}/blehColormap.o ${CM_OBJ_DIR}/heckColormap.o ${CP_OBJ_DIR}/redColorPalette.o ${CP_OBJ_DIR}/blehColorPalette.o ${CP_OBJ_DIR}/heckColorPalette.o ${UTILS}/build/previewImg.o ./test.o $(shell sdl2-config --libs) -L${LIB_DIR} -ljb -lpthread -o $@
	touch $@

./test.o: ./test.c
	gcc --coverage -g -c test.c ${INCS} $(shell sdl2-config --cflags --cflags) -o $@
	touch $@

${CM_OBJ_DIR}/%Colormap.o: ${CM_SRC_DIR}/%Colormap.c
	gcc -save-temps --coverage -g ${JB_INCS} ${INCS} -c ${CM_SRC_DIR}/$*Colormap.c $(shell sdl2-config --cflags) -o $@

${CP_OBJ_DIR}/%ColorPalette.o: ${CP_SRC_DIR}/%ColorPalette.c
	gcc -save-temps --coverage -g ${JB_INCS} ${INCS} -c $< $(shell sdl2-config --cflags) -o $@

${CP_SRC_DIR}/%ColorPalette.c: ${UTILS}/build/body
	${UTILS}/build/body $*

${CM_SRC_DIR}/%Colormap.c: ${UTILS}/build/body
	${UTILS}/build/body $*

${UTILS}/build/body:
	make -C ${UTILS}

${UTILS}/build/%.o:
	make -C ${UTILS}

${JB_DIR}/build/%.o: ${JB_DIR}/src/%.c
	make -C ${JB_DIR}

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
