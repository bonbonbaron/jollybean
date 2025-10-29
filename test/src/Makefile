include ./common.mk

all: o

o: test-gene.o xGeneric.o genericGenes.o xMedia.o mediaGenes.o $(LIBJB)
	$(CC) --coverage -g $< xGeneric.o genericGenes.o xMedia.o mediaGenes.o -L${LIB_DIR} -ljb -o $@

%.o: %.c
	$(CC) -g $(INCS) -c $< -o $@
