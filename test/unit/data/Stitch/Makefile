JB  = ${HOME}/hack/jollybean
SRC = ${JB}/src
INC = ${SRC}/include
BLD = ${JB}/build

./o: ./test.o ${BLD}/data.o
	${CC} ./test.o ${BLD}/data.o -o $@ 
	touch $@

./%.o: %.c 
	${CC} -g -I${INC} -c $*.c -o $@
	touch $@

${BLD}/%.o: ${SRC}/%.c ${INC}/%.h
	make -C ${JB}
	touch $@

.PHONY: clean
clean:
	rm -f ./o 2>/dev/null
	rm -f ./*.o 2>/dev/null
	rm -f ${BLD}/*.o 2>/dev/null
