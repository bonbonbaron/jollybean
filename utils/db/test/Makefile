test: ../build/db.o 
	gcc main.c ../build/db.o -I../../../src/include/ -I../src/include/ -L../../../lib -ljollybean  -o $@

../build/db.o: ../src/db.c ../src/include/db.h
	make -C ..


