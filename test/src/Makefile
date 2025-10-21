include ./common.mk

all: o

o: test-gene.o xGeneric.o $(LIBJB)
	$(CC) --coverage -g $< xGeneric.o -L${LIB_DIR} -ljb -o $@

test-share: test-share.o $(LIBJB)
	$(CC) --coverage -g $< -L${LIB_DIR} -ljb -o $@

%.o: %.c
	$(CC) --coverage -g $(INCS) -c $< -o $@
