#ifndef GENOME_
#define GENOME_
#include "jb.h"

#define Genome_(name_, ...) Genome name_ = {\
  .nGenes = nArgs_(XHeader*, __VA_ARGS__),\
  .genePA = {__VA_ARGS__}\
}

typedef struct {
  U32 nGenes;
  XHeader *genePA[];
} Genome;

extern Genome bigger;
#endif
