JB  = $(shell git rev-parse --show-toplevel)
SRC = ${JB}/src
_INC = $(shell find ${SRC} -type d -name include) $(JB)/test/include
INC := -I${JB}/include
BLD = ${JB}/build
LIB = ${JB}/lib
KR  = ${HOME}/jb/src/Keyring

srcs_and_incs := $(shell find ${JB} -type f -name *.[ch])

./o: ./test.o $(LIB)/libjb.a
	${CC} -g --coverage ./test.o $(shell sdl2-config --libs) -L$(LIB) -ljb  -o $@ 
	touch $@

./test.o: ./test.c ./test.h
	${CC} -g --coverage $(INC) -c ./test.c

$(LIB)/libjb.a: $(srcs_and_incs)
	make -C $(JB)

.PHONY: clean
clean:
	rm -f ./o 2>/dev/null
	rm -f ./*.o 2>/dev/null
	rm -f ${BLD}/*.o 2>/dev/null
