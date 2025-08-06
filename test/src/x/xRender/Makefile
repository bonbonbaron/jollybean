include ../../common.mk

./o: $(CPI_OBJS) ./test.o ${LIB_DIR}/libjb.a ${UTL_DIR}/build/*.o 
	gcc -DNDEBUG --coverage $(CPI_OBJS) ${UTL_DIR}/build/previewImg.o ./test.o $(shell sdl2-config --libs) -L${LIB_DIR} -ljb -lpthread -o $@

./test.o: ./test.c
	gcc -DNDEBUG --coverage -g -c test.c ${INCS} $(shell sdl2-config --cflags) -o $@


