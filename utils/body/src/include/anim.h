#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <png.h>
#include <assert.h>
#include <string.h>
#include "data.h"
#include "json.h"
#include "fileUtils.h"
#include "xAnim.h"

#define TEST_ 1

const char TROVE_ANIM_DIR[] = "/home/bonbonbaron/jb/src/Animation/";
const char TROVE_GENE_DIR[] = "~/jb/src/Gene/";
static int verbose = 0;

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
} Animation;

void getBaseNameIndices(char *filepathP, char *extension, U32 *startIdxP, U32 *endIdxP);
void writeRawData8(FILE *fP, U8 *byteA, U32 nBytes);
void writeRawData16(FILE *fP, U16 *byteA, U16 nBytes);
void writeRawData32(FILE *fP, U32 *byteA, U32 nBytes);
void parseName(char *filepathP, char *extension, U32 *entityNameIdxP, U32 *entityNameLenP);
Error anim (char *filepath, U8 verbose, Animation **animPP);
