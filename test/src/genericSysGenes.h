#ifndef IMM_SYS_GENES_H
#define IMM_SYS_GENES_H

#include "gene.h"

typedef struct GenericImmutableGene {
  GeneHdr hdr;
  int i;
} GenericImmutableGene;

typedef struct GenericMutableGene {
  GeneHdr hdr;
  int i;
} GenericMutableGene;
    

#endif // IMM_SYS_GENES_H
