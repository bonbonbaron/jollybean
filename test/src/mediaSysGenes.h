#ifndef MED_SYS_GENES_H
#define MED_SYS_GENES_H

#include "gene/gene.h"

// Bodies
typedef int MediaImmutableInt;
typedef Image MediaImg;

// Genes
// This gene only populates the immutable int, leaves out the rest.
typedef struct MediaImmutableGene {
  GeneHdr hdr;
  MediaImmutableInt body;
} MediaImmutableGene;

// This gene only populates the mutable short-char, leaves out the rest.
typedef struct MediaMutableGene {
  GeneHdr hdr;
  MediaMutableShortChar body;
} MediaMutableGene;

#endif // MED_SYS_GENES_H
