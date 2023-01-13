#ifndef STRIP_H_
#define STRIP_H_
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include "inflatable.h"
#include "fileUtils.h"
#include <stdlib.h>
#include "data.h"

Error stripNew(U8 *srcA, U8 verbose, U8 bpu, U32 nUnits, StripsetS **ssPP, StripmapS **smPP);
void stripDel(StripsetS *ssP, StripmapS *smP);
Error writeStripDataInFile(FILE *fP, U8 verbose, char *objNameA, StripsetS *ssP, StripmapS *smP);
Error writeStripData(char *objNameA, char *OBJ_TYPE, U8 verbose, StripsetS *ssP, StripmapS *smP);
Error unpackStripset(StripsetS *ssP, StripmapS *smP, U8 **outputPP);

#endif
