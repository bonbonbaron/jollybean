include ../../common.mk

./o: ./test.o $(LIB_DIR)/libjb.a
	${CC} -g --coverage ./test.o $(SDL_LFLAGS) -L$(LIB_DIR) -ljb  -o $@ 

./test.o: ./test.c ./test.h
	${CC} -g --coverage $(INCS) -c ./test.c
