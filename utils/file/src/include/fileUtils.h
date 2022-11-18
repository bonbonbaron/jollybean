#ifndef FILE_UTILS_
#define FILE_UTILS_
#include "data.h"
#include "assert.h"
#include "string.h"

#ifdef WINDOWS_
#define FILE_SEP '\\'
#else
#define FILE_SEP '/'
#endif

void parseEntityName(char *filepathP, char *extension, U32 *entityNameIdxP, U32 *entityNameLenP);
void writeRawData8(FILE *fP, U8 *byteA, U32 nBytes);
void writeRawData16(FILE *fP, U16 *byteA, U16 nBytes);
void writeRawData32(FILE *fP, U32 *byteA, U32 nBytes);
#endif
