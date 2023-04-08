BODY_DIR = ${HOME}/jb/build/Seed/Genome/Gene/Body

KEYRING_DIR=${HOME}/jb/src/Keyring/

COLORMAP_DIR = ${BODY_DIR}/Graybody/Colormap
CM_SRC_DIR = ${COLORMAP_DIR}/src
CM_INC_DIR = ${COLORMAP_DIR}/include
CM_OBJ_DIR = ${COLORMAP_DIR}/obj

COLOR_PALETTE_DIR = ${BODY_DIR}/Palette/Color
CP_SRC_DIR = ${COLOR_PALETTE_DIR}/src
CP_INC_DIR = ${COLOR_PALETTE_DIR}/include
CP_OBJ_DIR = ${COLOR_PALETTE_DIR}/obj

ANIM_DIR = ${BODY_DIR}/Graybody/Animation
ANIM_SRC_DIR = ${ANIM_DIR}/src
ANIM_INC_DIR = ${ANIM_DIR}/include
ANIM_OBJ_DIR = ${ANIM_DIR}/obj

JB_DIR=../../../../

#~/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/src/I
# Lesson learned: Linker searches from left to right for unresolved dependencies.
./o: ${CM_OBJ_DIR}/redColormap.o ${CM_OBJ_DIR}/blehColormap.o ${CM_OBJ_DIR}/heckColormap.o  ${CP_OBJ_DIR}/redColorPalette.o ${CP_OBJ_DIR}/blehColorPalette.o ${CP_OBJ_DIR}/heckColorPalette.o ${ANIM_OBJ_DIR}/redAnim.o ./main.o ${JB_DIR}/lib/libjollybean.a ./testBiome.o
	gcc -Wall ./main.o -L${JB_DIR}/lib -ljollybean $(shell sdl2-config --libs) -lpthread ./testBiome.o ${CM_OBJ_DIR}/redColormap.o  ${CM_OBJ_DIR}/blehColormap.o ${CM_OBJ_DIR}/heckColormap.o ${CP_OBJ_DIR}/redColorPalette.o ${CP_OBJ_DIR}/blehColorPalette.o ${CP_OBJ_DIR}/heckColorPalette.o ${ANIM_OBJ_DIR}/redAnim.o -o $@
	touch $@

./main.o: ./main.c 
	gcc -Wall -g -c main.c -I${KEYRING_DIR} -I${JB_DIR}/src/include/ -I${JB_DIR}/utils/body/src/include -I${CM_INC_DIR} -I${CP_INC_DIR} $(shell sdl2-config --cflags) -o $@
	touch $@

${CM_OBJ_DIR}/%Colormap.o: ${CM_SRC_DIR}/%Colormap.c
	gcc -Wall -g -I${KEYRING_DIR} -I${JB_DIR}/src/include -c ${CM_SRC_DIR}/$*Colormap.c $(shell sdl2-config --cflags) -o $@
	touch $@

${CP_OBJ_DIR}/%ColorPalette.o: ${CP_SRC_DIR}/%ColorPalette.c
	gcc -Wall -g -I${KEYRING_DIR} -I${JB_DIR}/src/include -c $< $(shell sdl2-config --cflags) -o $@
	touch $@

${ANIM_OBJ_DIR}/%Anim.o: ${ANIM_SRC_DIR}/%Anim.c ${ANIM_INC_DIR}/%Anim.h
	gcc -Wall -g -I${JB_DIR}/src/include -I${HOME}/jb/src/Keyring/ -c $< $(shell sdl2-config --cflags) -o $@
	touch $@

${CP_SRC_DIR}/%ColorPalette.c:
	${JB_DIR}/utils/body/bin/body $*
	touch $@

${CM_SRC_DIR}/%Colormap.c:
	${JB_DIR}/utils/body/bin/body $*
	touch $@

${ANIM_SRC_DIR}/%Anim.c:
	${JB_DIR}/utils/body/bin/body $*
	touch $@

${ANIM_INC_DIR}/%Anim.h:
	${JB_DIR}/utils/body/bin/body $*
	touch $@

./testBiome.o: ./testBiome.c
	gcc -Wall -g -c testBiome.c -I${HOME}/jb/src/Keyring/ -I${JB_DIR}/src/include -I${ANIM_INC_DIR} -I${CM_INC_DIR} -I${CP_INC_DIR} $(shell sdl2-config --cflags) -o testBiome.o
	touch $@

${JB_DIR}/lib/libjollybean.a: ${JB_DIR}/build/*.o
	make -C ${JB_DIR}
	touch $@

${JB_DIR}/build/%.o: ${JB_DIR}/src/%.c
	make -C ${JB_DIR}/
	touch $@

.PHONY: clean
clean:
	rm -f ./o 2>/dev/null
	rm -f ./*.o 2>/dev/null
	rm -f ${CP_INC_DIR}/*.h 2>/dev/null
	rm -f ${CP_OBJ_DIR}/*.o 2>/dev/null
	rm -f ${CP_SRC_DIR}/*.c 2>/dev/null
	rm -f ${CM_INC_DIR}/*.h 2>/dev/null
	rm -f ${CM_OBJ_DIR}/*.o 2>/dev/null
	rm -f ${CM_SRC_DIR}/*.c 2>/dev/null
	rm -f ${ANIM_SRC_DIR}/*.c 2>/dev/null
	rm -f ${ANIM_OBJ_DIR}/*.c 2>/dev/null
