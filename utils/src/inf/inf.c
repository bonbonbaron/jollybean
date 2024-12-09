#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>
#include "inflatable.h"
#include "array.h"

Inflatable* inflatableNew(void* voidA) {
  assert(voidA);  // pray to god it's actually an array... welcome to C
  Inflatable* infP;
  U32 szDataOrig = arrayGetNElems(voidA) * arrayGetElemSz(voidA);
  unsigned long szDataCompressed  = (szDataOrig * 1.1) + 12;
  U8 *dataOrigP = (U8*) voidA;
  U8 *dataCompressed = (U8*)malloc( szDataCompressed );
#if 1
  int z_result = compress(dataCompressed, &szDataCompressed, dataOrigP, szDataOrig);    
#else
  int z_result = compress2(dataCompressed, &szDataCompressed, dataOrigP, szDataOrig, 6);    
#endif

  switch(z_result) {
    case Z_OK:
      break;
    case Z_MEM_ERROR:
    case Z_BUF_ERROR:
    default:
      assert(0); // this means some other strange error happened
      break;
  }

  infP = jbAlloc( sizeof(Inflatable), 1);

  infP->compressedLen   = szDataCompressed;
  infP->inflatedLen     = szDataOrig;
  infP->inflatedDataP   = NULL;
  infP->compressedDataA = dataCompressed;
}

// The reason data.c doesn't own this function is because inflatables are permanent in-game.
void inflatableDel(Inflatable **infPP) {
  if (infPP && *infPP) {
    if ((*infPP)->compressedDataA) {
      free((*infPP)->compressedDataA);
      (*infPP)->compressedDataA = NULL;
    }
    if ((*infPP)->inflatedDataP) {
      free((*infPP)->inflatedDataP);
      (*infPP)->inflatedDataP = NULL;
    }
    jbFree((void**) infPP);
  }
}

void inflatableWrite(Inflatable *infP, char *filepathA, char *inflatableNameA) {
  assert (infP && !filepathA && inflatableNameA);

  unsigned int len = 0;
  FILE *fOutP = fopen(filepathA, "w");

  if (fOutP) {
    int counter = 0;
    fprintf(fOutP, "#include \"botox/botox.h\"\n\n");
    fprintf(fOutP, "Inflatable %s = {\n", inflatableNameA);
    fprintf(fOutP, "\t.compressedLen   = %d,\n", infP->compressedLen);
    fprintf(fOutP, "\t.inflatedDataP   = NULL,\n");
    fprintf(fOutP, "\t.inflatedLen     = %d,\n", infP->inflatedLen);
    fprintf(fOutP, "\t.compressedDataA = {\n\t");
    U8 *infByteP = infP->compressedDataA;
    U8 *infByteEndP = infByteP + infP->compressedLen;
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
}

void inflatableAppend(Inflatable *infP, FILE *fP, char *inflatableNameA) {
  assert (infP && fP && inflatableNameA);

  unsigned int len = 0;

  if (fP) {
    int counter = 0;
    // Inflatable's compressed data
    fprintf(fP, "U8 %sInfCompressedDataA[] = {\n", inflatableNameA);
    U8 *infByteP = infP->compressedDataA;
    U8 *infByteEndP = infByteP + infP->compressedLen;
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
    fprintf(fP, "\t.compressedLen  = %d,\n", infP->compressedLen);
    fprintf(fP, "\t.inflatedDataP  = NULL,\n");
    fprintf(fP, "\t.inflatedLen    = %d,\n", infP->inflatedLen);
    fprintf(fP, "\t.compressedDataA = %sInfCompressedDataA\n", inflatableNameA);
    fprintf(fP, "};\n\n");
  }
}
