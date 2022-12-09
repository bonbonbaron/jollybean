#include "fileUtils.h"

char JB_DIR_NAME[] = "jb";
char SRC_DIR_NAME[] = "src";

// An image name looks like this: /some/path/to/entityName.png
// Extension is the file extension including the leading '.'.
// The entity name is assumed to begin at the start of base name.
void parseName(char *filepathP, char *extension, U32 *entityNameIdxP, U32 *entityNameLenP) {
  U32 filenameLen = strlen(filepathP);
  U32 extLen = strlen(extension);  // including the leading "."
  // Make sure this file has a ".png" extension.
  assert(!strncasecmp(&filepathP[filenameLen - extLen], extension, extLen));

  // Init all outputs to zero.
  *entityNameIdxP = *entityNameLenP = 0;

  // Get both entity starting position.
  for (U8 i = filenameLen - 1; i >= 0; --i) {
    switch (filepathP[i]) {
#ifdef WINDOWS
      case '/':
#else
      case '\\':
#endif
        *entityNameIdxP = i + 1;
        goto breakout;
    }
  }
  breakout:
  // Get both entity length.
  *entityNameLenP = filenameLen - *entityNameIdxP - extLen;
}

void writeRawData8(FILE *fP, U8 *byteA, U32 nBytes) {
  U8 *cmpDataP = byteA;
  U8 *cmpDataEndP = cmpDataP + nBytes;
  for (U32 counter = 0; cmpDataP < cmpDataEndP; ++cmpDataP) {
    fprintf(fP, "0x%02x, ", *cmpDataP);
    if (++counter >= 16) {
      fprintf(fP, "\n\t");
      counter = 0;
    }
  }
}

void writeRawData16(FILE *fP, U16 *byteA, U16 nBytes) {
  U16 *cmpDataP = byteA;
  U16 *cmpDataEndP = cmpDataP + nBytes;
  for (U16 counter = 0; cmpDataP < cmpDataEndP; ++cmpDataP) {
    fprintf(fP, "0x%04x, ", *cmpDataP);
    if (++counter >= 8) {
      fprintf(fP, "\n\t");
      counter = 0;
    }
  }
}

void writeRawData32(FILE *fP, U32 *byteA, U32 nBytes) {
  U32 *cmpDataP = byteA;
  U32 *cmpDataEndP = cmpDataP + nBytes;
  for (U32 counter = 0; cmpDataP < cmpDataEndP; ++cmpDataP) {
    fprintf(fP, "0x%08x, ", *cmpDataP);
    if (++counter >= 4) {
      fprintf(fP, "\n\t");
      counter = 0;
    }
  }
}

Error getSrcDir(char **srcDirPath, U32 nExtraSpaces, char *srcLocalDirName) {
  char *HOME = getenv("HOME");
  U32 nChars = strlen(HOME)         + strlen(SEP) + 
               strlen(JB_DIR_NAME)  + strlen(SEP) + 
               strlen(SRC_DIR_NAME) + strlen(SEP) + 
               strlen(srcLocalDirName)   + strlen(SEP) + 
               + nExtraSpaces + 1;

  Error e = jbAlloc((void**) srcDirPath, sizeof(char), nChars);  // extra spaces if you plan to append filename to dir path
  if (!e) {
    memset(*srcDirPath, 0, sizeof(char) * nChars);
  }

  if (!e) {
    strcpy(*srcDirPath, JB_DIR_NAME);
    strcat(*srcDirPath, SEP);
    strcat(*srcDirPath, SRC_DIR_NAME);
    strcat(*srcDirPath, SEP);
    strcat(*srcDirPath, srcLocalDirName);
    strcat(*srcDirPath, SEP);
  }

  printf("src dir path result: %s\n", *srcDirPath);

  return e;
}

Error getSrcFilePath(char **srcFilePath, char *srcLocalDirName, char *srcFileName) {
 Error e = getSrcDir(srcFilePath, strlen(srcFileName), srcLocalDirName);

 if (!e && *srcFilePath) {
   strcat(*srcFilePath, srcFileName);
   printf(*srcFilePath);
 }
 else {
   jbFree((void**) srcFilePath);
 }


 return e;
}
