include ../../common.mk

o: ./test.o ${LIB_DIR}/libjb.a
	$(CC) --coverage -g $(SDL_LFLAGS) test.o -L${LIB_DIR} -ljb -o $@

./test.o: ./test.c 
	$(CC) --coverage -g $(SDL_CFLAGS) $(INCS) -c test.c -o $@
