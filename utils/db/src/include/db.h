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

// Unions keep dbectories' element sizes uniform. 
// That away we can use just one database API, 
// and the interface can use it for everything.
typedef union {
  struct {
    U32  nColors;
    U32  paletteA[16];  // at most 16 colors allowed in a single palette
  } palette;
  U32 collisionTypeColor;  // one color maps to one typet
} EntryData;

// Single entry in the saved palettes database.
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
} Database;

Error dbGet(Database **dbPP, char *dbName, int argc, U8 verbose);
Error dbReplaceOriginal(Database *dbP, char *dbName, U8 verbose);
void dbAddEntry(Database *dbP, char *name, EntryData *dataP, U8 verbose);
Entry* dbFindValueByName(Database *dbP, char *name, U8 verbose);
char* dbFindNameByValue(Database *dbP, EntryData *valP, U8 verbose);
NameNode* dbFindNamesStartingWith(Database *dbP, char *nameStart, U8 verbose);
void nameNodeDel(NameNode **nodePP);
#endif
