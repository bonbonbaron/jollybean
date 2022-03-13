#ifndef GENOME_
#define GENOME_
#include "jb.h"

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

#endif
