test: ../build/genie.o  ../../dir/build/dir.o
	gcc -g main.c ../build/genie.o ../../dir/build/dir.o -I../../dir/src/include -I/home/bonbonbaron/hack/botox/src/include/ -I../src/include/ -L/home/bonbonbaron/hack/botox/lib -lbotox  -o $@

../build/genie.o: ../src/genie.c ../src/include/genie.h
	make -C ..

../../dir/build/dir.o: ../../dir/src/dir.c ../../dir/src/include/dir.h
	make -C ../../dir/
