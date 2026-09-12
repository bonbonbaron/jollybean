#ifndef IMM_SYS_GENES_H
#define IMM_SYS_GENES_H

#include "gene/gene.h"

// Bodies
typedef int GenericImmutableInt;
typedef struct GenericMutableShortChar {
  short s;
  char c;
} GenericMutableShortChar;

// Genes
// This gene only populates the immutable int, leaves out the rest.
typedef struct GenericImmutableGene {
  GeneHdr hdr;
  GenericImmutableInt body;
} GenericImmutableGene;

// This gene only populates the mutable short-char, leaves out the rest.
typedef struct GenericMutableGene {
  GeneHdr hdr;
  GenericMutableShortChar body;
} GenericMutableGene;

#endif // IMM_SYS_GENES_H
