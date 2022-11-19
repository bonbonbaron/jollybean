#ifndef DIR_H_
#define DIR_H_
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "data.h"

extern const char LOCAL_TROVE_BOOKKEEPING_DIR[];

#define NAME_LEN_ (32)

// Unions keep directories' element sizes uniform. 
// That away we can use just one directory API, 
// and the interface can use it for everything.
typedef union {
  struct {
    U32  nColors;
    U32  paletteA[16];  // at most 16 colors allowed in a single palette
  } palette;
  U32 collisionTypeColor;  // one color maps to one typet
} EntryData;

// Single entry in the saved palettes directory.
typedef struct {
  char name[NAME_LEN_];
  EntryData data;
} Entry;

// List nodes for finding multiple partial query hits
typedef struct _NameNode {
  int entryIdx;  // index to partially matching name
  struct _NameNode *nextP;
} NameNode;

// Lets us keep track of things that may be shared across multiple entities.
// BTW, this is really just an array. But easier to store this way.
typedef struct {
  U32 nEntries;
  Entry entryA[];  // Allows unlimited palettes when placed at end of struct.
} Directory;

Error dirGet(Directory **dirPP, char *dirName, int argc, U8 verbose);
Error dirReplaceOriginal(Directory *dirP, char *dirName, U8 verbose);
void dirAddEntry(Directory *dirP, char *name, EntryData *dataP, U8 verbose);
Entry* dirFindValueByName(Directory *dirP, char *name, U8 verbose);
char* dirFindNameByValue(Directory *dirP, EntryData *valP, U8 verbose);
NameNode* dirFindNamesStartingWith(Directory *dirP, char *nameStart, U8 verbose);
void nameNodeDel(NameNode **nodePP);
#endif
