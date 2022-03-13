#ifndef BIOME_
#define BIOME_
#include "ecs.h"
#include "wild.h"

#define Biome_(name_, ...) Biome name_ = {\
  .nEntities = nArgs_(Genome*, __VA_ARGS__),\
  .genomePA = {__VA_ARGS__}\
}

#define Genome_(name_, ...) Genome name_ = {\
  .nGenes = nArgs_(GeneHeader*, __VA_ARGS__),\
  .genePA = {__VA_ARGS__}\
}

typedef enum {ECS_COMPONENT, ECS_SHARED, BLACKBOARD} GeneClass;

// Note: this header is for each individual gene.
typedef struct {
	U8 geneClass;
	U8 type;
	U8 size;
	U8 _padding_;
} GeneHeader;

typedef struct {
  U8 nGenes;
	GeneHeader *genePA[];
} Genome;

typedef struct {
	Genome *genomeP;
	Personality *reactionMP; 
} Seed;

typedef struct {
  U32 nEntities;
  Seed *seedPA[];
} Biome;

#endif
