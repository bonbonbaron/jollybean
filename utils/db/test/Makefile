test: ../build/db.o 
	gcc main.c ../build/db.o -I/home/bonbonbaron/hack/botox/src/include/ -I../src/include/ -L/home/bonbonbaron/hack/botox/lib -lbotox  -o $@

../build/db.o: ../src/db.c ../src/include/db.h
	make -C ..


