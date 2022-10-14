#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>
#include "botox/data.h"

Error inflatableNew(void *voidA, Inflatable **inflatablePP) {
  if (!voidA || !inflatablePP)
    return E_BAD_ARGS;

  Error e = SUCCESS;
  U32 szDataOrig = arrayGetNElems(voidA) * arrayGetElemSz(voidA);
  unsigned long szDataCompressed  = (szDataOrig * 1.1) + 12;
  U8 *dataOrigP = (U8*) voidA;
  U8 *dataCompressed = (U8*)malloc( szDataCompressed );
  int z_result = compress(dataCompressed, &szDataCompressed, dataOrigP, szDataOrig);    

  switch(z_result) {
    case Z_OK:
      break;
    case Z_MEM_ERROR:
    case Z_BUF_ERROR:
      e = E_NO_MEMORY;
      break;
    default:
      e = (Error) z_result;
      break;
  }

  if (!e) 
    e = jbAlloc((void**) inflatablePP, sizeof(Inflatable), 1);
  if (!e) {
    (*inflatablePP)->compressedLen   = szDataCompressed;
    (*inflatablePP)->inflatedLen     = szDataOrig;
    (*inflatablePP)->inflatedDataP   = NULL;
    (*inflatablePP)->compressedDataA = dataCompressed;
  }

  return e;
}

Error inflatableWrite(Inflatable *inflatableP, char *filepathA, char *inflatableNameA) {
  if (!inflatableP || !filepathA || !inflatableNameA)
    return E_BAD_ARGS;

  unsigned int len = 0;
  FILE *fOutP = fopen(filepathA, "w");

  int outC;
  if (fOutP) {
    int counter = 0;
    fprintf(fOutP, "#include \"botox/botox.h\"\n\n");
    fprintf(fOutP, "Inflatable sf3Inf = {\n");
    fprintf(fOutP, "\t.compressedLen  = 5774,\n");
    fprintf(fOutP, "\t.inflatedDataP  = NULL,\n");
    fprintf(fOutP, "\t.inflatedLen    = 10500,\n");
    fprintf(fOutP, "\t.compressedData = {\n\t");
    U8 *infByteP = inflatableP->compressedDataA;
    U8 *infByteEndP = infByteP + inflatableP->compressedLen;
    for (; infByteP < infByteEndP; ++infByteP) {
      fprintf(fOutP, "0x%02x, ", outC);
      if (++counter >= 16) {
        fprintf(fOutP, "\n\t");
        counter = 0;
      }
      ++len;
    }
    fprintf(fOutP, "\n\t}\n};");
    fclose(fOutP);
  }

  return 0;
}
