#include "dir.h"

static Error _dirCreate(char *filepath) {
  const static Directory dirPrototype = {0};
  FILE *fP = fopen(filepath, "wb");
  if (fP) {
    fwrite(&dirPrototype, sizeof(Directory), 1, fP);
    fclose(fP);
  }
  else { 
    printf("Failed to create missing file, %s\n", filepath);
    return E_BAD_ARGS;
  }
  return SUCCESS;
}

Error dirGet(Directory **dirPP, char *dirName, int argc, U8 verbose) {
  char *HOME = getenv("HOME");
  char filepath[strlen(HOME) + strlen(LOCAL_TROVE_BOOKKEEPING_DIR) + strlen(dirName)];
  strcpy(filepath, HOME);
  strcat(filepath, LOCAL_TROVE_BOOKKEEPING_DIR);
  strcat(filepath, dirName);

  Error e = SUCCESS;

  // Read just top-level object 
  FILE *fP = fopen(filepath, "rb");
  if (!fP) {
    fclose(fP);
    e = _dirCreate(filepath);
    if (!e) {
      fP = fopen(filepath, "rb");  // Should open if we successfully created it.
      if (!fP)
        e = E_BAD_ARGS;
    }
  }
  else {
    printf("no such directory file:\n\t%s\n", filepath);
  }

  U32 nBytesTotal = 0;
  if (!e) {
    fseek(fP, 0, SEEK_END);
    nBytesTotal = ftell(fP);
    e = jbAlloc((void**) dirPP, 2 *(nBytesTotal + (argc * sizeof(Entry))), 1);  // "* 2" just for safety
  }
  if (!e) {
    fseek(fP, 0, SEEK_SET);
    U32 nBytesRead = fread(*dirPP, 1, nBytesTotal, fP);
    if (verbose)
      printf("read %d bytes out of %d expected\n", nBytesRead, nBytesTotal);
    assert(nBytesRead == nBytesTotal);
    if (verbose)
      printf("Existing directory contains %d entries and %d bytes.\n", (*dirPP)->nEntries, nBytesTotal);

    fclose(fP);
  }

  return e;
}

Error dirReplaceOriginal(Directory *dirP, char *dirName, U8 verbose) {
  char *HOME = getenv("HOME");
  char filepath[strlen(HOME) + strlen(LOCAL_TROVE_BOOKKEEPING_DIR) + strlen(dirName)];
  strcpy(filepath, HOME);
  strcat(filepath, LOCAL_TROVE_BOOKKEEPING_DIR);
  strcat(filepath, dirName);

  Error e = SUCCESS;

  // Open the file.
  FILE *fP = fopen(filepath, "wb");
  if (!fP) {
    fclose(fP);
    return E_BAD_ARGS;
  }

  // Write to it.
  U32 nBytesToWrite = sizeof(Directory) + (dirP->nEntries * sizeof(Entry));
  U32 nBytesWritten = fwrite(dirP, 1, nBytesToWrite, fP);
  if (verbose) {
    printf("\n\n%d actual of %d expected bytes written to directory file:.\n", nBytesWritten, nBytesToWrite);
    printf("\tDirectory: %d bytes\n", sizeof(Directory));
    printf("\t%d entries (%d bytes each): %d bytes\n", dirP->nEntries, sizeof(Entry), nBytesToWrite);
    printf("Output filepath:\n");
    printf("\t%s\n", filepath);
  }

  // Close it.
  fclose(fP);

  return e;
}

void dirAddEntry(Directory *dirP, char *name, EntryData *dataP, U8 verbose) {
  if (verbose)
    printf("adding %s to directory\n", name);

  strcpy(dirP->entryA[dirP->nEntries].name, name);
  dirP->entryA[dirP->nEntries++].data = *dataP;
}

Entry* dirFindValueByName(Directory *dirP, char *name, U8 verbose) {
  for (int i = 0; i < dirP->nEntries; ++i)
    if (!strcmp(name, dirP->entryA[i].name))
      return &dirP->entryA[i];
  if (verbose)
    printf("Could not find entry named %s in directory.\n", name);
  return NULL;
}

char* dirFindNameByValue(Directory *dirP, EntryData *valP, U8 verbose) {
  for (int i = 0; i < dirP->nEntries; ++i)
    if (!memcmp(valP, &dirP->entryA[i].data, sizeof(EntryData)))
      return dirP->entryA[i].name;
  if (verbose)
    printf("Could not find entry.\n");
  return NULL;
}
