#include "db.h"

const char LOCAL_TROVE_BOOKKEEPING_DIR[] = "/jb/src/Bookkeeping/";

static Error _dbNew(char *filepath, U8 verbose) {
  const static Database dbPrototype = {0};
  FILE *fP = fopen(filepath, "wb");
  if (fP) {
    if (verbose)
      printf("Writing %d bytes to new file.\n", sizeof(Database));
    fwrite(&dbPrototype, sizeof(Database), 1, fP);
    fclose(fP);
  }
  else { 
    printf("Failed to create missing file, %s\n", filepath);
    return E_FILE_IO;
  }
  return SUCCESS;
}

Error dbGet(Database **dbPP, char *dbName, int argc, U8 verbose) {
  char *HOME = getenv("HOME");
  char filepath[strlen(HOME) + strlen(LOCAL_TROVE_BOOKKEEPING_DIR) + strlen(dbName)];
  strcpy(filepath, HOME);
  strcat(filepath, LOCAL_TROVE_BOOKKEEPING_DIR);
  strcat(filepath, dbName);

  Error e = SUCCESS;
  U8 fileIsNew = 0;

  // Make sure the db file exists.
  FILE *fP = fopen(filepath, "rb");
  if (!fP) {
    fileIsNew = 1;
    e = _dbNew(filepath, verbose);
    if (!e) {
      fP = fopen(filepath, "rb");  // Should open if we successfully created it.
      if (!fP) {
        printf("no such db file:\n\t%s\n", filepath);
        e = E_FILE_IO;
      }
    }
  }
  else {
  }

  // See how big the file is.
  U32 nBytesTotal = 0;
  if (!e) {
    if (fseek(fP, 0, SEEK_END))  // returns 0 if successful
      e = E_FILE_IO;
  }
  if (!e) {
    if (fileIsNew) {
      if (verbose)
        printf("file's new, so pretending we read 0 bytes even if we didn't\n");
      nBytesTotal = 0;
    }
    else {
      if (verbose)
        printf("file's not new, so we're treating it like an adult\n");
      nBytesTotal = ftell(fP);
    }
    // Make room to store the file's data in memory.
    e = jbAlloc((void**) dbPP, 2 *(nBytesTotal + (argc * sizeof(Entry))), 1);  // "* 2" just for safety
  }
  // Read it into memory and make sure it's as big as we say it is.
  if (!e) {
    rewind(fP);
    U32 nBytesRead = fread(*dbPP, 1, nBytesTotal, fP);
    if (verbose)
      printf("read %d bytes out of %d expected\n", nBytesRead, nBytesTotal);
    assert(nBytesRead == nBytesTotal);
    if (verbose)
      printf("Existing database contains %d entries and %d bytes.\n", (*dbPP)->nEntries, nBytesTotal);

    fclose(fP);
  }

  return e;
}

Error dbReplaceOriginal(Database *dbP, char *dbName, U8 verbose) {
  char *HOME = getenv("HOME");
  char filepath[strlen(HOME) + strlen(LOCAL_TROVE_BOOKKEEPING_DIR) + strlen(dbName)];
  strcpy(filepath, HOME);
  strcat(filepath, LOCAL_TROVE_BOOKKEEPING_DIR);
  strcat(filepath, dbName);

  Error e = SUCCESS;

  // Open the file.
  FILE *fP = fopen(filepath, "wb");
  if (!fP) {
    fclose(fP);
    return E_BAD_ARGS;
  }

  // Write to it.
  U32 nBytesToWrite = sizeof(Database) + (dbP->nEntries * sizeof(Entry));
  U32 nBytesWritten = fwrite(dbP, 1, nBytesToWrite, fP);
  if (verbose) {
    printf("\n\n%d actual of %d expected bytes written to database file:.\n", nBytesWritten, nBytesToWrite);
    printf("\tDatabase: %d bytes\n", sizeof(Database));
    printf("\t%d entries (%d bytes each): %d bytes\n", dbP->nEntries, sizeof(Entry), nBytesToWrite);
    printf("Output filepath:\n");
    printf("\t%s\n", filepath);
  }

  // Close it.
  fclose(fP);

  return e;
}

void dbAddEntry(Database *dbP, char *name, EntryData *dataP, U8 verbose) {
  if (verbose)
    printf("adding %s to database\n", name);

  strcpy(dbP->entryA[dbP->nEntries].name, name);
  dbP->entryA[dbP->nEntries++].data = *dataP;
}

// List nodes for finding multiple partial query hits
Error nameNodeNew(NameNode **nodePP) {
  if (!nodePP)
    return E_BAD_ARGS;
  Error e = jbAlloc((void**) nodePP, sizeof(NameNode), 1);
  if (!e) 
    memset(*nodePP, 0, sizeof(NameNode));
  return e;
}

static Error _nameNodeGrow(NameNode **nodePP) {
  if (!nodePP)
    return E_BAD_ARGS;
  Error e = SUCCESS;
  // If this is the first node
  if (!*nodePP) {
    e = nameNodeNew(nodePP);
  }
  // If this is after the first node
  else {
    e = nameNodeNew(&(*nodePP)->nextP);
    if (!e) {
      *nodePP = (*nodePP)->nextP;
    }
  }
  return e;
}

void nameNodeDel(NameNode **nodePP) {
  if (nodePP && *nodePP) {
    // Recurse all the way to the end then free backwards.
    if ((*nodePP)->nextP) {
      nameNodeDel(&(*nodePP)->nextP);
    }
    // Actual deletion
    jbFree((void**) nodePP);
    *nodePP = NULL;
  }
}

// Querying functions
Entry* dbFindValueByName(Database *dbP, char *name, U8 verbose) {
  for (int i = 0; i < dbP->nEntries; ++i)
    if (!strcmp(name, dbP->entryA[i].name))
      return &dbP->entryA[i];
  if (verbose)
    printf("Could not find entry named %s in db.\n", name);
  return NULL;
}

char* dbFindNameByValue(Database *dbP, EntryData *valP, U8 verbose) {
  for (int i = 0; i < dbP->nEntries; ++i)
    if (!memcmp(valP, &dbP->entryA[i].data, sizeof(EntryData)))
      return dbP->entryA[i].name;
  if (verbose)
    printf("Could not find entry.\n");
  return NULL;
}

NameNode* dbFindNamesStartingWith(Database *dbP, char *nameStart, U8 verbose) {
  assert(strlen(nameStart) < NAME_LEN_);
  NameNode *nodeP = NULL;
  // Look for nodes starting with input string.
  NameNode *firstNodeP = NULL;
  Error e = SUCCESS;
  for (int i = 0; !e && i < dbP->nEntries; ++i) {
    if (!memcmp(nameStart, &dbP->entryA[i].name, strlen(nameStart))) {
      if (firstNodeP == NULL) {
        // Allocate first name node to build list of matching names from.
        e = nameNodeNew(&nodeP); 
        // Leave if it returns an error.
        if (!e) {
          nodeP->entryIdx = i;
          firstNodeP = nodeP;
        }
      }
      else {
        e = _nameNodeGrow(&nodeP);  // nodeP = nodeP->next (after allocating)
        if (!e) {
          nodeP->entryIdx = i;
        }
      }
      if (verbose) {
        printf("Match found: %s\n", dbP->entryA[i].name);
      }
    }
  }

  if (e) {
    nameNodeDel(&firstNodeP);
  }

  return firstNodeP;
}
