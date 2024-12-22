BODY_DIR = ${HOME}/jb/build/Seed/Genome/Gene/Body

COLORMAP_DIR = ${BODY_DIR}/Image/Colormap
CM_SRC_DIR = ${COLORMAP_DIR}/src
CM_INC_DIR = ${COLORMAP_DIR}/include
CM_OBJ_DIR = ${COLORMAP_DIR}/obj

COLOR_PALETTE_DIR = ${BODY_DIR}/Image/ColorPalette
CP_SRC_DIR = ${COLOR_PALETTE_DIR}/src
CP_INC_DIR = ${COLOR_PALETTE_DIR}/include
CP_OBJ_DIR = ${COLOR_PALETTE_DIR}/obj

JB_DIR = ${HOME}/hack/jollybean/
UTIL_DIR = $(JB_DIR)/utils
KR_DIR = ${HOME}/jb/src/Keyring
JB_INCS = -I${JB_DIR}src/x/include -I${JB_DIR}src/interface/include -I${JB_DIR}src/data/include -I$(JB_DIR)/test/include

./o: ${CM_OBJ_DIR}/redColormap.o ${CM_OBJ_DIR}/blehColormap.o ${CM_OBJ_DIR}/heckColormap.o  ${CP_OBJ_DIR}/redColorPalette.o ${CP_OBJ_DIR}/blehColorPalette.o ${CP_OBJ_DIR}/heckColorPalette.o ./test.o ${JB_DIR}/lib/libjb.a ${CM_SRC_DIR}/blehColormap.c ${CM_SRC_DIR}/heckColormap.c ${CM_SRC_DIR}/redColormap.c $(UTIL_DIR)/build/sd.o $(UTIL_DIR)/build/inf.o test.h
	gcc -save-temps --coverage ./test.o $(UTIL_DIR)/build/inf.o $(UTIL_DIR)/build/sd.o ${CM_OBJ_DIR}/redColormap.o ${CM_OBJ_DIR}/blehColormap.o ${CM_OBJ_DIR}/heckColormap.o ${CP_OBJ_DIR}/redColorPalette.o ${CP_OBJ_DIR}/blehColorPalette.o ${CP_OBJ_DIR}/heckColorPalette.o -L/usr/local/lib -Wl,-rpath,/usr/local/lib -Wl,--enable-new-dtags $(shell sdl2-config --libs) -L${JB_DIR}/lib -ljb -lpthread -lz -o $@

./test.o: ./test.c ./test.h
	gcc -save-temps --coverage -g -c test.c ${JB_INCS} -I${KR_DIR} -I${JB_DIR}src/include/ -I$(UTIL_DIR)/include -I${CM_INC_DIR} -I${CP_INC_DIR} $(shell sdl2-config --cflags) -o $@

${CM_OBJ_DIR}/%Colormap.o: ${CM_SRC_DIR}/%Colormap.c
	gcc -save-temps --coverage -g ${JB_INCS} -I${KR_DIR} -I${JB_DIR}src/include -c ${CM_SRC_DIR}/$*Colormap.c $(shell sdl2-config --cflags) -o $@

${CP_OBJ_DIR}/%ColorPalette.o: ${CP_SRC_DIR}/%ColorPalette.c
	gcc -save-temps --coverage -g ${JB_INCS} -I${KR_DIR} -I${JB_DIR}src/include -c $< $(shell sdl2-config --cflags) -o $@

${CP_SRC_DIR}/%ColorPalette.c: $(UTIL_DIR)/build/body 
	$(UTIL_DIR)/build/body -v $*

${CM_SRC_DIR}/%Colormap.c: $(UTIL_DIR)/build/body
	$(UTIL_DIR)/build/body -v $*

$(UTIL_DIR)/build/body:
	make -C $(UTIL_DIR)

${JB_DIR}build/%.o: ${JB_DIR}src/%.c
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
