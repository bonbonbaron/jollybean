#ifndef BIOME_
#define BIOME_
#include "jb.h"

#define Biome_(name_, ...) Biome name_ = {\
  .nEntities = nArgs_(Genome*, __VA_ARGS__),\
  .genomePA = {__VA_ARGS__}\
}

typedef struct {
  U32 nEntities;
  Seed *seedPA[];
} Biome;

#endif
