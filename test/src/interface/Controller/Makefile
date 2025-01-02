include ../../common.mk

./o: ./test.o ${JBLIB}
	${CC} --coverage ./test.o -L${LIB_DIR} -ljb $(SDL_LFLAGS) -lpthread -o $@ 
	touch $@

./%.o: %.c 
	${CC} --coverage -g ${INCS} -c $*.c $(SDL_CFLAGS) -o $@
	touch $@
