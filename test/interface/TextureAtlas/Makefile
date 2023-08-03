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
#~/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/src/I
./o: ${CM_OBJ_DIR}/redColormap.o ${CM_OBJ_DIR}/blehColormap.o ${CM_OBJ_DIR}/heckColormap.o  ${CP_OBJ_DIR}/redColorPalette.o ${CP_OBJ_DIR}/blehColorPalette.o ${CP_OBJ_DIR}/heckColorPalette.o ./main.o ${JB_DIR}/build/xRender.o ${JB_DIR}/build/x.o ${JB_DIR}/build/data.o ${JB_DIR}/build/interface.o ${JB_DIR}/utils/body/build/previewImg.o
	gcc --coverage ${CM_OBJ_DIR}/redColormap.o ${CM_OBJ_DIR}/blehColormap.o ${CM_OBJ_DIR}/heckColormap.o ${CP_OBJ_DIR}/redColorPalette.o ${CP_OBJ_DIR}/blehColorPalette.o ${CP_OBJ_DIR}/heckColorPalette.o ${JB_DIR}/utils/body/build/previewImg.o ./main.o ${JB_DIR}/build/interface.o $(shell sdl2-config --libs) ${JB_DIR}/build/xRender.o ${JB_DIR}/build/x.o ${JB_DIR}/build/data.o -lpthread -o $@
	touch $@

./main.o: ./main.c
	gcc --coverage -g -c main.c -I${KR_DIR} -I${JB_DIR}/src/include/ -I${JB_DIR}/utils/body/src/include -I${CM_INC_DIR} -I${CP_INC_DIR} $(shell sdl2-config --cflags --cflags) -o $@
	touch $@

${CM_OBJ_DIR}/%Colormap.o: ${CM_SRC_DIR}/%Colormap.c
	gcc --coverage -g -I${KR_DIR} -I${JB_DIR}/src/include -c ${CM_SRC_DIR}/$*Colormap.c $(shell sdl2-config --cflags) -o $@
	touch $@

${CP_OBJ_DIR}/%ColorPalette.o: ${CP_SRC_DIR}/%ColorPalette.c
	gcc --coverage -g -I${KR_DIR} -I${JB_DIR}/src/include -c $< $(shell sdl2-config --cflags) -o $@
	touch $@

${CP_SRC_DIR}/%ColorPalette.c:
	${JB_DIR}/utils/body/bin/body $*
	touch $@

${CM_SRC_DIR}/%Colormap.c:
	${JB_DIR}/utils/body/bin/body $*
	touch $@

./%.o: %.c 
	${CC} --coverage -g -I${INC} -c $*.c -o $@
	touch $@

${BLD}/%.o: ${SRC}/%.c ${INC}/%.h
	gcc --coverage -g -c $< -I${INC} -o $@
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
	rm -f *.gcda *.gcno *.gcov
	rm -rf coverage
