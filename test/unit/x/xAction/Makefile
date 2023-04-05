JB  = ${HOME}/hack/jollybean
SRC = ${JB}/src
INC = ${SRC}/include
BLD = ${JB}/build
KR  = ${HOME}/jb/src/Keyring

./o: ./test.o ${BLD}/data.o ${BLD}/x.o ${BLD}/xAction.o
	${CC} ./test.o $(shell sdl2-config --libs) ${BLD}/xAction.o ${BLD}/x.o ${BLD}/data.o -o $@ 
	touch $@

./%.o: %.c 
	${CC} -g -I${INC} -I${KR} -c $*.c $(shell sdl2-config --cflags) -o $@
	touch $@

${BLD}/%.o: ${SRC}/%.c ${INC}/%.h
	make -C ${JB}
	touch $@

.PHONY: clean
clean:
	rm -f ./o 2>/dev/null
	rm -f ./*.o 2>/dev/null
	rm -f ${BLD}/*.o 2>/dev/null
