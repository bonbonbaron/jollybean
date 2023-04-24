JB_DIR=$(shell git rev-parse --show-toplevel)
o: test.c ${JB_DIR}/build/data.o
	gcc --coverage test.c ${JB_DIR}/build/data.o -I${JB_DIR}/src/include -o $@

${JB_DIR}/build/%.o: ${JB_DIR}/src/%.c
	gcc --coverage -g -c $< -o $@
