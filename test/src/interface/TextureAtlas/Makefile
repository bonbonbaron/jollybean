BODY_DIR = ${HOME}/jb/build/Seed/Genome/Gene/Body

COLORMAP_DIR = ${BODY_DIR}/Graybody/Colormap
CM_SRC_DIR = ${COLORMAP_DIR}/src
CM_INC_DIR = ${COLORMAP_DIR}/include
CM_OBJ_DIR = ${COLORMAP_DIR}/obj

COLOR_PALETTE_DIR = ${BODY_DIR}/Palette/Color
CP_SRC_DIR = ${COLOR_PALETTE_DIR}/src
CP_INC_DIR = ${COLOR_PALETTE_DIR}/include
CP_OBJ_DIR = ${COLOR_PALETTE_DIR}/obj

JB_DIR = $(shell git rev-parse --show-toplevel)
KR_DIR = ${HOME}/jb/src/Keyring
LIB_DIR := $(JB_DIR)/lib


_INCS   := $(shell find ${JB_DIR}/src -type d -name include) ${CM_INC_DIR} ${CP_INC_DIR} $(shell find ${JB_DIR}/utils/ -type d -name include)
INCS    := $(_INCS:%=-I%)

./o: ${CM_OBJ_DIR}/redColormap.o ${CM_OBJ_DIR}/blehColormap.o ${CM_OBJ_DIR}/heckColormap.o  ${CP_OBJ_DIR}/redColorPalette.o ${CP_OBJ_DIR}/blehColorPalette.o ${CP_OBJ_DIR}/heckColorPalette.o ./main.o ${LIB_DIR}/libjb.a ${JB_DIR}/utils/body/build/previewImg.o
	gcc --coverage ${CM_OBJ_DIR}/redColormap.o ${CM_OBJ_DIR}/blehColormap.o ${CM_OBJ_DIR}/heckColormap.o ${CP_OBJ_DIR}/redColorPalette.o ${CP_OBJ_DIR}/blehColorPalette.o ${CP_OBJ_DIR}/heckColorPalette.o ${JB_DIR}/utils/body/build/previewImg.o ./main.o $(shell sdl2-config --libs) -L${LIB_DIR} -ljb -lpthread -o $@
	touch $@

./main.o: ./main.c
	gcc --coverage -g -c main.c ${INCS} $(shell sdl2-config --cflags --cflags) -o $@
	touch $@

${CM_OBJ_DIR}/%Colormap.o: ${CM_SRC_DIR}/%Colormap.c
	gcc --coverage -g ${INCS} -c ${CM_SRC_DIR}/$*Colormap.c $(shell sdl2-config --cflags) -o $@
	touch $@

${CP_OBJ_DIR}/%ColorPalette.o: ${CP_SRC_DIR}/%ColorPalette.c
	gcc --coverage -g ${INCS} -c $< $(shell sdl2-config --cflags) -o $@
	touch $@

${CP_SRC_DIR}/%ColorPalette.c:
	${JB_DIR}/utils/body/bin/body $*
	touch $@

${CM_SRC_DIR}/%Colormap.c:
	${JB_DIR}/utils/body/bin/body $*
	touch $@

./%.o: %.c ${_INCS}
	${CC} --coverage -g ${INCS} -c $*.c -o $@
	touch $@

${BLD}/%.o: ${SRC}/%.c ${_INCS}
	gcc --coverage -g -c $< ${INCS} -o $@
	touch $@

$(LIB_DIR)/libjb.a:
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
	rm -f *.gcda *.gcno *.gcov
	rm -rf coverage
