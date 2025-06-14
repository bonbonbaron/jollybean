#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>
#include "data/inflatable.h"
#include "data/array.h"

Inflatable* inflatableNew(void* voidA) {
  assert(voidA);  // pray to god it's actually an array... welcome to C
  Inflatable* infP;
  U32 szDataOrig = arrayGetNElems(voidA) * arrayGetElemSz(voidA);
  unsigned long szDataCompressed  = (szDataOrig * 1.1) + 12;
  U8 *dataOrigP = (U8*) voidA;
  U8 *dataCompressed = (U8*)memAdd( szDataCompressed, MAIN );
  int z_result = compress(dataCompressed, &szDataCompressed, dataOrigP, szDataOrig);    

  switch(z_result) {
    case Z_OK:
      break;
    case Z_MEM_ERROR:
    case Z_BUF_ERROR:
    default:
      assert(0); // this means some other strange error happened
      return NULL;
      break;
  }

  infP = memAdd( sizeof(Inflatable), MAIN );

  infP->compressedLen   = szDataCompressed;
  infP->inflatedLen     = szDataOrig;
  infP->inflatedDataP   = NULL;
  infP->compressedDataA = dataCompressed;

  return infP;
}

void inflatableWrite(Inflatable *infP, char *filepathA, char *inflatableNameA) {
  assert (infP && !filepathA && inflatableNameA);

  unsigned int len = 0;
  FILE *fOutP = fopen(filepathA, "w");

  if (fOutP) {
    int counter = 0;
    fprintf(fOutP, "#include \"botox/botox.h\"\n\n");
    fprintf(fOutP, "Inflatable %s = {\n", inflatableNameA);
    fprintf(fOutP, "\t.compressedLen   = %ld,\n", infP->compressedLen);
    fprintf(fOutP, "\t.inflatedDataP   = NULL,\n");
    fprintf(fOutP, "\t.inflatedLen     = %ld,\n", infP->inflatedLen);
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
    // Inflatable itself
    fprintf(fP, "Inflatable %s = {\n", inflatableNameA);
    fprintf(fP, "\t.compressedLen  = %ld,\n", infP->compressedLen);
    fprintf(fP, "\t.inflatedDataP  = NULL,\n");
    fprintf(fP, "\t.inflatedLen    = %ld,\n", infP->inflatedLen);
    fprintf(fP, "\t.compressedDataA = %sInfCompressedDataA\n", inflatableNameA);
    fprintf(fP, "};\n\n");
  }
}
