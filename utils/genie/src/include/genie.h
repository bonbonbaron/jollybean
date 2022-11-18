#ifndef GENIE_H_
#define GENIE_H_
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "dir.h"

typedef enum {TEXT_FILE, DIR_FILE} ListType;

typedef union {
  Directory dir;
  char **stringA;
} List;

void genieListDir(Directory *dirP);
void genieListTxt(Directory *dirP);
Error genieListen(ListType fileType);
#endif
