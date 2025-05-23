#ifndef FILE_UTILS_
#define FILE_UTILS_
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "data/common.h"
#include "data/mem.h"

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
char* parseName(char *filepathP, char *extension, U8 verbose);
void writeRawData8(FILE *fP, U8 *byteA, U32 nBytes);
void writeRawData16(FILE *fP, U16 *byteA, U16 nBytes);
void writeRawData32(FILE *fP, U32 *byteA, U32 nBytes);
char* getSrcFilePath(char *srcLocalDirName, char *srcFileName, char *srcFileSuffix, U8 verbose);
char* getBuildFilePath(char *buildLocalDirName, char *buildFileName, char *buildFileSuffix, U8 verbose);
char* getResourceFilePath(char *resourceLocalDirName, char *resourceFileName, char *resourceFileSuffix, U8 verbose);
FILE* getSrcFile(char *srcLocalDirName, char *srcFileName, char *srcFileSuffix, U8 verbose);
FILE* getBuildFile(char *buildLocalDirName, char *buildFileName, char *buildFileSuffix, U8 verbose);
FILE* getResourceFile(char *resourceLocalDirName, char *resourceFileName, char *resourceFileSuffix, U8 verbose);

extern char JB_DIR_NAME[];
extern char SRC_DIR_NAME[];
extern char DST_DIR_NAME[];
#endif
