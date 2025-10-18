include ../../common.mk

all: libtestx 

libtestx: testx.o $(LIBJB)
	$(CC) --coverage -g test.o xA.o -L${LIB_DIR} -ljb -o $@

testx.o: testx.c testx.h
	$(CC) --coverage -g $(INCS) -c $< -o $@
