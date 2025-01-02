include ../../common.mk

#~/jb/build/Seed/Genome/Gene/Body/Graybody/Colormap/src/I
# Lesson learned: Linker searches from left to right for unresolved dependencies.
./o: ${CPI_OBJS} ./main.o ${LIB_DIR}/libjb.a 
	gcc -Wall ./main.o -L${JB_DIR}/lib -ljollybean $(SDL_LFLAGS) -lpthread ${CPI_OBJS} -o $@
	touch $@

./main.o: ./main.c 
	gcc -Wall -g -c main.c -I${JB_DIR}/include/ -I${JB_DIR}/utils/body/src/include -I${CM_INC_DIR} -I${CP_INC_DIR} $(SDL_CFLAGS) -o $@
	touch $@
