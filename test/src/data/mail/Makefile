include ../data.mk

o: test.o $(LIBJB)
	$(CC) -g test.o -L${LIB_DIR} -ljb -lgcov -o $@

%.o: %.c
	$(CC) --coverage -g $(INCS) -c $< -o $@
