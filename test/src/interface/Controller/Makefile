JB  := $(shell git rev-parse --show-toplevel)
SRC := ${JB}/src
INC := -I${SRC}/interface/include -I${SRC}/x/include -I${SRC}/data/include
BLD := ${JB}/build/src
OBJ := ${BLD}/interface/interface.o ${BLD}/x/x.o ${BLD}/data/fray.o ${BLD}/data/map.o ${BLD}/data/array.o ${BLD}/data/mem.o
./o: ./test.o ${OBJ}
	${CC} --coverage ./test.o ${OBJ} $(shell sdl2-config --libs) -lpthread -o $@ 
	touch $@

./%.o: %.c 
	${CC} --coverage -g ${INC} -c $*.c $(shell sdl2-config --cflags) -o $@
	touch $@

${BLD}/data/%.o: ${SRC}/data/%.c ${SRC}/data/include/%.h
	gcc --coverage -g -c $< ${INC} $(shell sdl2-config --cflags) -o $@
	touch $@

${BLD}/x/%.o: ${SRC}/x/%.c ${SRC}/x/include/%.h
	gcc --coverage -g -c $< ${INC} $(shell sdl2-config --cflags) -o $@
	touch $@

${BLD}/interface/%.o: ${SRC}/data/interface/%.c ${SRC}/data/interface/include/%.h
	gcc --coverage -g -c $< ${INC} $(shell sdl2-config --cflags) -o $@
	touch $@


.PHONY: clean
clean:
	rm -f ./o 2>/dev/null
	rm -f ./*.o 2>/dev/null
	rm -f ${BLD}/* 2>/dev/null
	rm -rf ./coverage 2>/dev/null
	rm *.gcda *.gcno *.gcov 2>/dev/null
