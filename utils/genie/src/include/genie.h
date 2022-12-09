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
#include "db.h"

typedef enum {TEXT_FILE, DIR_FILE} ListType;

typedef union {
  Database db;
  char **stringA;
} List;

void genieListDb(Database *db);
void genieListTxt(Database *db);
char* genieAsk(char* question, ListType listType, List *listP, U8 verbose);
#endif
