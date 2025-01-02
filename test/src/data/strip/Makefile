include ../../common.mk

./o: $(CPI_OBJS) ./test.o $(LIBJB) $(UTL_DIR)/build/sd.o $(UTL_DIR)/build/inf.o 
	gcc -save-temps --coverage ./test.o $(UTL_DIR)/build/sd.o $(UTL_DIR)/build/inf.o $(CPI_OBJS) $(SDL_LFLAGS) -L$(LIB_DIR) -ljb -lpthread -lz -o $@

./test.o: ./test.c ./test.h
	gcc -save-temps --coverage -g -c test.c $(INCS) $(SDL_CFLAGS) -o $@
