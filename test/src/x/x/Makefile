include ../../common.mk

o: test.o xA.o $(LIBJB)
	$(CC) --coverage -g test.o xA.o -L${LIB_DIR} -ljb -o $@

%.o: %.c
	$(CC) --coverage -g $(INCS) -c $< -o $@
