#ifndef ANIM_H
#define ANIM_H
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <png.h>
#include <assert.h>
#include <string.h>
#include "data.h"
#include "json.h"
#include "fileUtils.h"

typedef struct _TagNode {
  char *name;
  U32 from, to;
  char *direction;
  struct _TagNode *nextP;
} TagNode;

typedef struct _FrameNode {
  U32 x, y, w, h, duration;
  struct _FrameNode *nextP;
} FrameNode;

typedef struct {
  TagNode *tagNodeA;
  FrameNode *frameNodeA;
} AnimJsonData;

void getBaseNameIndices(char *filepathP, char *extension, U32 *startIdxP, U32 *endIdxP);
void writeRawData8(FILE *fP, U8 *byteA, U32 nBytes);
void writeRawData16(FILE *fP, U16 *byteA, U16 nBytes);
void writeRawData32(FILE *fP, U32 *byteA, U32 nBytes);
Error anim (char *entityNameP, U8 verbose, AnimJsonData **animPP);
#endif
