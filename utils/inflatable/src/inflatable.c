#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>
#include "data.h"

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

// The reason data.c doesn't own this function is because inflatables are permanent in-game.
void inflatableDel(Inflatable **inflatablePP) {
  if (inflatablePP && *inflatablePP) {
    if ((*inflatablePP)->compressedDataA) {
      free((*inflatablePP)->compressedDataA);
      (*inflatablePP)->compressedDataA = NULL;
    }
    if ((*inflatablePP)->inflatedDataP) {
      free((*inflatablePP)->inflatedDataP);
      (*inflatablePP)->inflatedDataP = NULL;
    }
    free(*inflatablePP);
    *inflatablePP = NULL;
  }
}

Error inflatableWrite(Inflatable *inflatableP, char *filepathA, char *inflatableNameA) {
  if (!inflatableP || !filepathA || !inflatableNameA)
    return E_BAD_ARGS;

  unsigned int len = 0;
  FILE *fOutP = fopen(filepathA, "w");

  if (fOutP) {
    int counter = 0;
    fprintf(fOutP, "#include \"botox/botox.h\"\n\n");
    fprintf(fOutP, "Inflatable %s = {\n", inflatableNameA);
    fprintf(fOutP, "\t.compressedLen   = %d,\n", inflatableP->compressedLen);
    fprintf(fOutP, "\t.inflatedDataP   = NULL,\n");
    fprintf(fOutP, "\t.inflatedLen     = %d,\n", inflatableP->inflatedLen);
    fprintf(fOutP, "\t.compressedDataA = {\n\t");
    U8 *infByteP = inflatableP->compressedDataA;
    U8 *infByteEndP = infByteP + inflatableP->compressedLen;
    for (; infByteP < infByteEndP; ++infByteP) {
      fprintf(fOutP, "0x%02x, ", *infByteP);
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

Error inflatableAppend(Inflatable *inflatableP, FILE *fP, char *inflatableNameA) {
  if (!inflatableP || !fP || !inflatableNameA)
    return E_BAD_ARGS;

  unsigned int len = 0;

  if (fP) {
    int counter = 0;
    // Inflatable's compressed data
    fprintf(fP, "U8 %sInfCompressedDataA[] = {\n", inflatableNameA);
    U8 *infByteP = inflatableP->compressedDataA;
    U8 *infByteEndP = infByteP + inflatableP->compressedLen;
    fprintf(fP, "\t");
    for (; infByteP < infByteEndP; ++infByteP) {
      fprintf(fP, "0x%02x, ", *infByteP);
      if (++counter >= 16) {
        fprintf(fP, "\n\t");
        counter = 0;
      }
      ++len;
    }
    fprintf(fP, "\n};\n\n");
    // Inflatble itself
    fprintf(fP, "Inflatable %s = {\n", inflatableNameA);
    fprintf(fP, "\t.compressedLen  = %d,\n", inflatableP->compressedLen);
    fprintf(fP, "\t.inflatedDataP  = NULL,\n");
    fprintf(fP, "\t.inflatedLen    = %d,\n", inflatableP->inflatedLen);
    fprintf(fP, "\t.compressedDataA = %sInfCompressedDataA\n", inflatableNameA);
    fprintf(fP, "};\n\n");
  }

  return 0;
}
