all: ./o

include $(shell git rev-parse --show-toplevel)/Makefile
include ${RPO_DIR}/test/Images.mk

INCS   := -I${RPO_DIR}/include -I${RPO_DIR}/test/include -I${UTL_DIR}/include 

.PHONY: clean
clean:
	rm -f ./*.o 
	rm -f ./o
	rm -f ${HOME}/jb/build/*.o
