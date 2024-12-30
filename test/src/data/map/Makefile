include ../data.mk

o: test.o $(LIBJB)
	$(CC) --coverage -g $< -L${LIB_DIR} -ljb -o $@

%.o: %.c
	$(CC) --coverage -g $(INCS) -c $< -o $@
