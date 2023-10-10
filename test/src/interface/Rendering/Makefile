BODY_DIR = ${HOME}/jb/build/Seed/Genome/Gene/Body

JB  = $(shell git rev-parse --show-toplevel)
SRC = ${JB}/src
INC = ${SRC}/include
BLD = ${JB}/build
UTL = ${JB}/utils
PRI = ${UTL}/body/build

COLORMAP_DIR = ${BODY_DIR}/Graybody/Colormap
CM_SRC_DIR = ${COLORMAP_DIR}/src
CM_INC_DIR = ${COLORMAP_DIR}/include
CM_OBJ_DIR = ${COLORMAP_DIR}/obj

COLOR_PALETTE_DIR = ${BODY_DIR}/Palette/Color
CP_SRC_DIR = ${COLOR_PALETTE_DIR}/src
CP_INC_DIR = ${COLOR_PALETTE_DIR}/include
CP_OBJ_DIR = ${COLOR_PALETTE_DIR}/obj

KR_DIR = ${HOME}/jb/src/Keyring
./o: ${CM_OBJ_DIR}/redColormap.o ${CM_OBJ_DIR}/blehColormap.o ${CM_OBJ_DIR}/heckColormap.o  ${CP_OBJ_DIR}/redColorPalette.o ${CP_OBJ_DIR}/blehColorPalette.o ${CP_OBJ_DIR}/heckColorPalette.o ./render.o ${JB}/build/xRender.o ${JB}/build/x.o ${JB}/build/data.o ${JB}/build/interface.o ${JB}/utils/body/build/previewImg.o 
	gcc ${CM_OBJ_DIR}/redColormap.o ${CM_OBJ_DIR}/blehColormap.o ${CM_OBJ_DIR}/heckColormap.o ${CP_OBJ_DIR}/redColorPalette.o ${CP_OBJ_DIR}/blehColorPalette.o ${CP_OBJ_DIR}/heckColorPalette.o ./render.o ${JB}/utils/body/build/previewImg.o ${JB}/build/interface.o $(shell sdl2-config --libs) ${JB}/build/xRender.o ${JB}/build/x.o ${JB}/build/data.o -lpthread -o $@
	touch $@

./render.o: ./render.c
	gcc -g -c render.c -I${KR_DIR} -I${JB}/src/include/ -I${JB}/utils/body/src/include/ -I${CM_INC_DIR} -I${CP_INC_DIR} $(shell sdl2-config --cflags --cflags) -o $@
	touch $@

${CM_OBJ_DIR}/%Colormap.o: ${CM_SRC_DIR}/%Colormap.c
	gcc -g -I${KR_DIR} -I${JB}/src/include -c $< $(shell sdl2-config --cflags) -o $@
	touch $@

${CP_OBJ_DIR}/%ColorPalette.o: ${CP_SRC_DIR}/%ColorPalette.c
	gcc -g -I${KR_DIR} -I${JB}/src/include -c $< $(shell sdl2-config --cflags) -o $@
	touch $@

${CP_SRC_DIR}/%ColorPalette.c:
	${JB}/utils/body/bin/body $*
	touch $@

${CM_SRC_DIR}/%Colormap.c:
	${JB}/utils/body/bin/body $*
	touch $@

${PRI}/previewImg.o: ${UTL}/body/src/previewImg.c ${BLD}/data.o ${BLD}/interface.o ${BLD}/x.o ${BLD}/xRender.o 
	make -C ${UTL}/body
	make -C ${JB}
	touch $@

${BLD}/%.o: ${SRC}/%.c ${INC}/%.h
	make -C ${JB}
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
