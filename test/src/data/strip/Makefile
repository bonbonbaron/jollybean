CC=gcc

RPO_DIR := $(shell git rev-parse --show-toplevel)
LIB_DIR := $(RPO_DIR)/lib
INF_DIR := $(RPO_DIR)/utils/infUtil
INF_BLD := $(INF_DIR)/build
INF_SRC := $(INF_DIR)/src
STR_DIR := $(RPO_DIR)/utils/stripUtil
STR_BLD := $(STR_DIR)/build
STR_SRC := $(STR_DIR)/src
FIL_DIR := $(RPO_DIR)/utils/file
FIL_BLD := $(FIL_DIR)/build
FIL_SRC := $(FIL_DIR)/src
_INCS   := $(shell find ${RPO_DIR}/src ${INF_SRC} ${STR_SRC} ${FIL_SRC} -type d -name 'include')
INCS    := -I. $(_INCS:%=-I%)

SRCS    := $(shell find . -type f -name "*.c")
OBJS    := $(SRCS:%.c=%.o) ${INF_BLD}/infUtil.o ${STR_BLD}/stripUtil.o ${FIL_BLD}/fileUtils.o

all: o 

o: ${OBJS} $(LIB_DIR)/libjb.a
	rm -rf coverage/*
	$(CC) --coverage -g ${OBJS} -L${LIB_DIR} -lz -ljb -o $@

%.o: %.c
	$(CC) --coverage -g $(INCS) -c $< -o $@

$(INF_BLD)/%.o: $(INF_SRC)/%.c
	make -C $(INF_DIR)

$(STR_BLD)/%.o: $(STR_SRC)/%.c
	make -C $(STR_DIR)

$(FIL_BLD)/%.o: $(FIL_SRC)/%.c
	make -C $(FIL_DIR)

$(LIB_DIR)/libjb.a:
	make -C ${RPO_DIR}

.PHONY: clean
clean:
	rm -rf o 2>/dev/null
	rm -rf coverage 2>/dev/null
	rm -f ${OBJS} 2>/dev/null
