#ifndef MED_SYS_GENES_H
#define MED_SYS_GENES_H

#include "gene/gene.h"

// Genes
// This gene only populates the mutable short-char, leaves out the rest.
typedef struct MediaImageGene {
  GeneHdr hdr;
  Image* imgP;
} MediaImageGene;

#endif // MED_SYS_GENES_H
