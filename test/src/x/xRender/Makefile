SRC_DIR = ${HOME}/jb/src
BUILD_DIR = ${HOME}/jb/build

JB_DIR = $(shell git rev-parse --show-toplevel)
UTILS = ${JB_DIR}/utils
KR_DIR = ${HOME}/jb/src/Keyring
LIB_DIR := $(JB_DIR)/lib

INCS   := -I${UTILS}/include -I${JB_DIR}/include -I${JB_DIR}/test/include

all: ./o

include ${JB_DIR}/test/Images.mk


./o: $(CPI_OBJS) ./test.o ${LIB_DIR}/libjb.a ${UTILS}/build/*.o ${JB_DIR}/build/src/x/xRender.o ${JB_DIR}/src/x/xRender.c
	gcc --coverage $(CPI_OBJS) ${UTILS}/build/previewImg.o ./test.o $(shell sdl2-config --libs) -L${LIB_DIR} -ljb -lpthread -o $@

./test.o: ./test.c
	gcc --coverage -g -c test.c ${INCS} $(shell sdl2-config --cflags) -o $@

.PHONY: clean
clean:
	rm -f ./test.o 
	rm -f ./o
	rm -f ${HOME}/jb/build/*.o


