include ../data.mk

o: test.o $(LIBJB) $(UTL_DIR)/build/inf.o 
	$(CC) test.o -L${LIB_DIR} -ljb $(UTL_DIR)/build/inf.o -lgcov -lz -o $@

test.o: test.c
	$(CC) --coverage -g $(INCS) -c $< -o $@
