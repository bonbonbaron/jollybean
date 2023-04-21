JB  = ${HOME}/hack/jollybean
SRC = ${JB}/src
INC = ${SRC}/include
BLD = ${JB}/build

./o: ./test.o ${BLD}/interface.o ${BLD}/data.o ${BLD}/x.o
	${CC} --coverage ./test.o $(shell sdl2-config --libs) ${BLD}/interface.o -lpthread ${BLD}/x.o ${BLD}/data.o -o $@ 
	touch $@

./%.o: %.c 
	${CC} --coverage -g -I${INC} -c $*.c $(shell sdl2-config --cflags) -o $@
	touch $@

${BLD}/%.o: ${SRC}/%.c ${INC}/%.h
	gcc --coverage -g -c $< -I${INC} $(shell sdl2-config --cflags) -o $@
	touch $@

.PHONY: clean
clean:
	rm -f ./o 2>/dev/null
	rm -f ./*.o 2>/dev/null
	rm -f ${BLD}/* 2>/dev/null
	rm -rf ./coverage 2>/dev/null
	rm *.gcda *.gcno *.gcov 2>/dev/null
