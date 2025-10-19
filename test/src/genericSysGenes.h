#ifndef IMM_SYS_GENES_H
#define IMM_SYS_GENES_H

#include "gene.h"

typedef int GenericImmutableInt;
typedef struct GenericMutableShortChar {
  short s;
  char c;
} GenericMutableShortChar;

typedef struct GenericImmutableGene {
  GeneHdr hdr;
  GenericImmutableInt* geneBodyP;
} GenericImmutableGene;

#endif // IMM_SYS_GENES_H
