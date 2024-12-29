#include "fileUtils.h"

char JB_DIR_NAME[] = "jb";
char SRC_DIR_NAME[] = "src";
char RESOURCE_DIR_NAME[] = "resource";
char BUILD_DIR_NAME[] = "build";

// An image name looks like this: /some/path/to/entityName.png
// Extension is the file extension including the leading '.'.
// The entity name is assumed to begin at the start of base name.
char* parseName(char *filepathP, char *extension, U8 verbose) {
  U32 filenameLen = strlen(filepathP);
  U32 extLen = strlen(extension);  // including the leading "."
  U32 entityNameIdx = 0;
  U32 entityNameLen = 0;
  // Make sure this file has the expected extension.
  assert(!strncasecmp(&filepathP[filenameLen - extLen], extension, extLen));

  // Get both entity starting position.
  for (S32 i = filenameLen - 1; i >= 0; --i) {
    switch (filepathP[i]) {
#ifndef WINDOWS
      case '/':
#else
      case '\\':
#endif
        entityNameIdx = i + 1;
        goto breakout;
    }
  }
  breakout:
  // Get both entity length.
  entityNameLen = filenameLen - entityNameIdx - extLen;

  if (entityNameLen == 0) {
    if (verbose) {
      printf("entity name is empty!\n");
    }
    return NULL;
  }

  char *outputNameP = (char*) jbAlloc(sizeof(char), (entityNameLen + 1) * sizeof(char));
  memset(outputNameP, 0, (entityNameLen + 1) * sizeof(char));
  memcpy((void*) outputNameP, (void*) &filepathP[entityNameIdx], entityNameLen);
  return outputNameP;
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
  fprintf(fP, "\t");
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

static char* getSrcDir(U32 nExtraSpaces, char *srcLocalDirName, U8 verbose) {
  char *HOME = getenv("HOME");
  U32 nChars = strlen(HOME)         + strlen(SEP) + 
               strlen(JB_DIR_NAME)  + strlen(SEP) + 
               strlen(SRC_DIR_NAME) + strlen(SEP) + 
               strlen(srcLocalDirName)   + strlen(SEP) + 
               nExtraSpaces + 1;

  char* srcDirPath = jbAlloc(sizeof(char), nChars);  // extra spaces if you plan to append filename to dir path
  memset(srcDirPath, 0, sizeof(char) * nChars);

  strcpy(srcDirPath, HOME);
  strcat(srcDirPath, SEP);
  strcat(srcDirPath, JB_DIR_NAME);
  strcat(srcDirPath, SEP);
  strcat(srcDirPath, SRC_DIR_NAME);
  strcat(srcDirPath, SEP);
  strcat(srcDirPath, srcLocalDirName);
  strcat(srcDirPath, SEP);

  if (verbose) {
    printf("src dir path result: %s\n", srcDirPath);
  }
  return srcDirPath;
}

static char* getResourceDir(U32 nExtraSpaces, char *resourceLocalDirName, U8 verbose) {
  char *HOME = getenv("HOME");
  U32 nChars = strlen(HOME)         + strlen(SEP) + 
               strlen(JB_DIR_NAME)  + strlen(SEP) + 
               strlen(RESOURCE_DIR_NAME) + strlen(SEP) + 
               strlen(resourceLocalDirName)   + strlen(SEP) + 
               nExtraSpaces + 1;

  char* resourceDirPath = jbAlloc(sizeof(char), nChars);  // extra spaces if you plan to append filename to dir path
  memset(resourceDirPath, 0, sizeof(char) * nChars);

  strcpy(resourceDirPath, HOME);
  strcat(resourceDirPath, SEP);
  strcat(resourceDirPath, JB_DIR_NAME);
  strcat(resourceDirPath, SEP);
  strcat(resourceDirPath, RESOURCE_DIR_NAME);
  strcat(resourceDirPath, SEP);
  strcat(resourceDirPath, resourceLocalDirName);
  strcat(resourceDirPath, SEP);

  if (verbose) {
    printf("resource dir path result: %s\n", resourceDirPath);
  }
  return resourceDirPath;
}

char* getResourceFilePath(char *resourceLocalDirName, char *resourceFileName, char *resourceFileSuffix, U8 verbose) {
  char* resourceFilePath = getResourceDir(strlen(resourceFileName) + strlen(resourceFileSuffix), resourceLocalDirName, verbose);

  assert (resourceFilePath);
  strcat(resourceFilePath, resourceFileName);
  strcat(resourceFilePath, resourceFileSuffix);
  if (verbose) {
    printf("final resource file path: %s\n", resourceFilePath);
  }
  return resourceFilePath;
}

char* getSrcFilePath(char *srcLocalDirName, char *srcFileName, char *srcFileSuffix, U8 verbose) {
  char* srcFilePath = getSrcDir(strlen(srcFileName) + strlen(srcFileSuffix), srcLocalDirName, verbose);

  assert (srcFilePath);
  strcat(srcFilePath, srcFileName);
  strcat(srcFilePath, srcFileSuffix);
  if (verbose) {
    printf("final src file path: %s\n", srcFilePath);
  }
  return srcFilePath;
}

static char* getBuildDir(U32 nExtraSpaces, char *buildLocalDirName, U8 verbose) {
  char *HOME = getenv("HOME");
  U32 nChars = strlen(HOME)                + strlen(SEP) + 
    strlen(JB_DIR_NAME)         + strlen(SEP) + 
    strlen(BUILD_DIR_NAME)      + strlen(SEP) + 
    strlen(buildLocalDirName)   + strlen(SEP) + 
    nExtraSpaces + 1;

  char* buildDirPath = jbAlloc(sizeof(char), nChars);  // extra spaces if you plan to append filename to dir path
  memset(buildDirPath, 0, sizeof(char) * nChars);

  strcpy(buildDirPath, HOME);
  strcat(buildDirPath, SEP);
  strcat(buildDirPath, JB_DIR_NAME);
  strcat(buildDirPath, SEP);
  strcat(buildDirPath, BUILD_DIR_NAME);
  strcat(buildDirPath, SEP);
  strcat(buildDirPath, buildLocalDirName);
  strcat(buildDirPath, SEP);

  if (verbose) {
    printf("build dir path result: %s\n", buildDirPath);
  }
  return buildDirPath;
}

char* getBuildFilePath( char *buildLocalDirName, char *buildFileName, char *buildFileSuffix, U8 verbose) {
  char* buildFilePath = getBuildDir(strlen(buildFileName) + strlen(buildFileSuffix), buildLocalDirName, verbose);

  if (buildFilePath) {
    strcat(buildFilePath, buildFileName);
    strcat(buildFilePath, buildFileSuffix);
    if (verbose) {
      printf("final build file path: %s\n", buildFilePath);
    }
  }
  else {
    jbFree((void**) &buildFilePath);
  }
  return buildFilePath;
}

FILE* getBuildFile(char *buildLocalDirName, char *buildFileName, char *buildFileSuffix, U8 verbose) {
  FILE  *fP = NULL;
  if (!buildLocalDirName || !buildFileName || !buildFileSuffix) {
    printf("[getBuildFile] error: one of the args is NULL or 0.\n");
    return NULL;
  }
  char *buildFilePath = getBuildFilePath(buildLocalDirName, buildFileName, buildFileSuffix, verbose);
  if (verbose) {
    printf("About to open build file %s...\n", buildFilePath);
  }
  // Open file.
  fP = fopen(buildFilePath, "w");
  assert(fP);
  if (buildFilePath) {
    jbFree((void**) &buildFilePath);
  }
  return fP;
}

FILE* getSrcFile(char *srcLocalDirName, char *srcFileName, char *srcFileSuffix, U8 verbose) {
  FILE  *fP = NULL;
  if (!srcLocalDirName || !srcFileName || !srcFileSuffix) {
    printf("[getSrcFile] error: one of the args is NULL or 0.\n");
    return NULL;
  }
  char *srcFilePath = getSrcFilePath(srcLocalDirName, srcFileName, srcFileSuffix, verbose);
  if (verbose) {
    printf("About to open source file %s...\n", srcFilePath);
  }
  // Open file.
  fP = fopen(srcFilePath, "w");
  assert(fP);
  if (srcFilePath) {
    jbFree((void**) &srcFilePath);
  }
  return fP;
}

FILE* getResourceFile(char *resourceLocalDirName, char *resourceFileName, char *resourceFileSuffix, U8 verbose) {
  FILE  *fP = NULL;
  if (!resourceLocalDirName || !resourceFileName || !resourceFileSuffix) {
    printf("[getResourceFile] error: one of the args is NULL or 0.\n");
    return NULL;
  }
  char *resourceFilePath = getResourceFilePath(resourceLocalDirName, resourceFileName, resourceFileSuffix, verbose);
  if (verbose) {
    printf("About to open source file %s...\n", resourceFilePath);
  }
  // Open file.
  fP = fopen(resourceFilePath, "r");
  assert(fP);
  if (resourceFilePath) {
    jbFree((void**) &resourceFilePath);
  }
  return fP;
}
