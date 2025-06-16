#ifndef STRIP_H_
#define STRIP_H_
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include "data/mem.h"
#include "data/inflatable.h"
#include "inf.h"
#include "data/strip.h"
#include "fileUtils.h"

StripDataS* stripNew(U8 *srcA, const size_t nUsedBytesPerUnpackedStrip, const U8 bitsPerPackedByte, size_t flags, U8 verbose);
void writeStripDataInFile(FILE *fP, U8 verbose, char *objNameA, StripDataS *sdP);
void writeStripData(char *objNameA, char *OBJ_TYPE, U8 verbose, StripDataS *sdP);
void printStripData(StripDataS *sdP);  // for debugging

#endif
