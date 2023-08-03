JB  = $(shell git rev-parse --show-toplevel)
SRC = ${JB}/src
INC = ${SRC}/include
BLD = ${JB}/build

./o: ./test.o ${BLD}/data.o ${BLD}/x.o ./xA.o
	${CC} --coverage -lm ./test.o ./xA.o ${BLD}/x.o ${BLD}/data.o -o $@ 
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
	rm -f ./*.o 2>/dev/null
	rm -f ${BLD}/* 2>/dev/null
	rm -rf ./coverage 2>/dev/null
	rm *.gcda *.gcno *.gcov 2>/dev/null
