all: ./o

include $(shell git rev-parse --show-toplevel)/Makefile

INCS   := -I${RPO_DIR}/include 

o: mem_arena.o $(LIBJB)
	$(CC) --coverage -g $< -L${LIB_DIR} -ljb -o $@

%.o: %.c
	$(CC) --coverage -g $(INCS) -c $< -o $@

.PHONY: clean
clean:
	rm -f ./*.o 
	rm -f ./o
	rm -f ${HOME}/jb/build/*.o
