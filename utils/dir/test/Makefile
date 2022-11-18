test: ../build/dir.o 
	gcc main.c ../build/dir.o -I/home/bonbonbaron/hack/botox/src/include/ -I../src/include/ -L/home/bonbonbaron/hack/botox/lib -lbotox  -o $@

../build/dir.o: ../src/dir.c ../src/include/dir.h
	make -C ..


