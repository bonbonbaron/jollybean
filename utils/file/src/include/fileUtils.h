#ifndef FILE_UTILS_
#define FILE_UTILS_
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "data.h"

#ifdef WINDOWS_
#define SEP "\\"
#else
#define SEP "/"
#endif

/// Under Windows (VC) unistd.h does not exist. To make it work it is necessary to define:
#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#endif

//FILE* accessFile(char *fp);
void parseName(char *filepathP, char *extension, U32 *entityNameIdxP, U32 *entityNameLenP);
void writeRawData8(FILE *fP, U8 *byteA, U32 nBytes);
void writeRawData16(FILE *fP, U16 *byteA, U16 nBytes);
void writeRawData32(FILE *fP, U32 *byteA, U32 nBytes);
Error getSrcFilePath(char **srcFilePath, char *srcLocalDirName, char *srcFileName, char *srcFileSuffix);
Error getBuildFilePath(char **buildFilePath, char *buildLocalDirName, char *buildFileName, char *buildFileSuffix);

extern char JB_DIR_NAME[];
extern char SRC_DIR_NAME[];
extern char DST_DIR_NAME[];
#endif