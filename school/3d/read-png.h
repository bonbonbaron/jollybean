#ifndef READ_PNG_H
#define READ_PNG_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <png.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "array.h"

typedef struct {
  unsigned char r, g, b;
} Color;

typedef struct {
  Color *dataP;
  int w, h, pixelSz, pitch;
} Image;

unsigned char readPng(char *imgPathA, Image *imgP, int verbose);

#endif
