#ifndef IMM_SYS_GENES_H
#define IMM_SYS_GENES_H

#include "gene.h"

typedef struct GenericImmutableGene {
  GeneHdr hdr = {
    .class = IMMUTABLE,
    .u.type = GENERIC,
    .size = sizeof(ImmutableGene),
    .name = "ImmutableGene"
  },
  int i;
} ImmutableGene;

typedef struct GenericMutableGene {
  GeneHdr hdr = {
    .class = IMMUTABLE,
    .u.type = GENERIC,
    .size = sizeof(ImmutableGene),
    .name = "ImmutableGene"
  },
  int i;
} ImmutableGene;
    

#endif // IMM_SYS_GENES_H
