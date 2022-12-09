test: ./main.c ../build/genie.o  ../../db/build/db.o
	gcc -g main.c ../build/genie.o ../../db/build/db.o -I../../db/src/include -I/home/bonbonbaron/hack/botox/src/include/ -I../src/include/ -L/home/bonbonbaron/hack/botox/lib -lbotox  -o $@

../build/genie.o: ../src/genie.c ../src/include/genie.h
	make -C ..

../../db/build/db.o: ../../db/src/db.c ../../db/src/include/db.h
	make -C ../../db/
