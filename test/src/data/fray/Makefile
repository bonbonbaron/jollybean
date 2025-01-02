include ../../common.mk

o: test.o $(LIBJB)
	$(CC) --coverage -g test.o -L${LIB_DIR} -ljb -o $@

%.o: %.c
	$(CC) --coverage -g $(INCS) -c $< -o $@
