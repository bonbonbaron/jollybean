include ../x.mk

o: test.o $(LIBJB)
	$(CC) --coverage -g test.o -L${LIB_DIR} $(SDL_LFLAGS) -ljb -o $@

test.o: test.c
	$(CC) --coverage -g $(INCS) $(SDL_CFLAGS) -c $< -o $@
