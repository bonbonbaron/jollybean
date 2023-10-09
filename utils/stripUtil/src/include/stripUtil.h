#ifndef STRIP_H_
#define STRIP_H_
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include "mem.h"
#include "inflatable.h"
#include "infUtil.h"
#include "strip.h"
#include "fileUtils.h"

Error stripNew(U8 *srcA, const U32 nUsedBytesPerUnpackedStrip, const U8 bitsPerPackedByte, StripDataS **sdPP, U32 flags, U8 verbose);
void stripDel(StripDataS **sdPP);
Error writeStripDataInFile(FILE *fP, U8 verbose, char *objNameA, StripDataS *sdP);
Error writeStripData(char *objNameA, char *OBJ_TYPE, U8 verbose, StripDataS *sdP);
Error stripIni(StripDataS *sdP);
void printStripData(StripDataS *sdP);  // for debugging

#endif
