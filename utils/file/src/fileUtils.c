#include "fileUtils.h"

char JB_DIR_NAME[] = "jb";
char SRC_DIR_NAME[] = "src";
char BUILD_DIR_NAME[] = "build";

// An image name looks like this: /some/path/to/entityName.png
// Extension is the file extension including the leading '.'.
// The entity name is assumed to begin at the start of base name.
void parseName(char *filepathP, char *extension, U32 *entityNameIdxP, U32 *entityNameLenP) {
  U32 filenameLen = strlen(filepathP);
  U32 extLen = strlen(extension);  // including the leading "."
  // Make sure this file has the expected extension.
  assert(!strncasecmp(&filepathP[filenameLen - extLen], extension, extLen));

  // Init all outputs to zero.
  *entityNameIdxP = *entityNameLenP = 0;

  // Get both entity starting position.
  for (S32 i = filenameLen - 1; i >= 0; --i) {
    switch (filepathP[i]) {
#ifndef WINDOWS
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

static Error getSrcDir(char **srcDirPath, U32 nExtraSpaces, char *srcLocalDirName) {
  char *HOME = getenv("HOME");
  U32 nChars = strlen(HOME)         + strlen(SEP) + 
               strlen(JB_DIR_NAME)  + strlen(SEP) + 
               strlen(SRC_DIR_NAME) + strlen(SEP) + 
               strlen(srcLocalDirName)   + strlen(SEP) + 
               nExtraSpaces + 1;

  Error e = jbAlloc((void**) srcDirPath, sizeof(char), nChars);  // extra spaces if you plan to append filename to dir path
  if (!e) {
    memset(*srcDirPath, 0, sizeof(char) * nChars);
  }

  if (!e) {
    strcpy(*srcDirPath, HOME);
    strcat(*srcDirPath, SEP);
    strcat(*srcDirPath, JB_DIR_NAME);
    strcat(*srcDirPath, SEP);
    strcat(*srcDirPath, SRC_DIR_NAME);
    strcat(*srcDirPath, SEP);
    strcat(*srcDirPath, srcLocalDirName);
    strcat(*srcDirPath, SEP);
  }

  printf("src dir path result: %s\n", *srcDirPath);

  return e;
}

Error getSrcFilePath(char **srcFilePath, char *srcLocalDirName, char *srcFileName, char *srcFileSuffix) {
 Error e = getSrcDir(srcFilePath, strlen(srcFileName) + strlen(srcFileSuffix), srcLocalDirName);

 if (!e && *srcFilePath) {
   strcat(*srcFilePath, srcFileName);
   strcat(*srcFilePath, srcFileSuffix);
   printf("final src file path: %s\n", *srcFilePath);
 }
 else {
   jbFree((void**) srcFilePath);
 }

 return e;
}

static Error getBuildDir(char **buildDirPath, U32 nExtraSpaces, char *buildLocalDirName) {
  char *HOME = getenv("HOME");
  U32 nChars = strlen(HOME)                + strlen(SEP) + 
               strlen(JB_DIR_NAME)         + strlen(SEP) + 
               strlen(BUILD_DIR_NAME)      + strlen(SEP) + 
               strlen(buildLocalDirName)   + strlen(SEP) + 
               nExtraSpaces + 1;

  Error e = jbAlloc((void**) buildDirPath, sizeof(char), nChars);  // extra spaces if you plan to append filename to dir path
  if (!e) {
    memset(*buildDirPath, 0, sizeof(char) * nChars);
  }

  if (!e) {
    strcpy(*buildDirPath, HOME);
    strcat(*buildDirPath, SEP);
    strcat(*buildDirPath, JB_DIR_NAME);
    strcat(*buildDirPath, SEP);
    strcat(*buildDirPath, BUILD_DIR_NAME);
    strcat(*buildDirPath, SEP);
    strcat(*buildDirPath, buildLocalDirName);
    strcat(*buildDirPath, SEP);
  }

  printf("build dir path result: %s\n", *buildDirPath);

  return e;
}

Error getBuildFilePath(char **buildFilePath, char *buildLocalDirName, char *buildFileName, char *buildFileSuffix) {
 Error e = getBuildDir(buildFilePath, strlen(buildFileName) + strlen(buildFileSuffix), buildLocalDirName);

 if (!e && *buildFilePath) {
   strcat(*buildFilePath, buildFileName);
   strcat(*buildFilePath, buildFileSuffix);
   printf("final build file path: %s\n", *buildFilePath);
 }
 else {
   jbFree((void**) buildFilePath);
 }

 return e;
}