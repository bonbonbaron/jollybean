#ifndef XMAIN_
#define XMAIN_
#include "x.h"
#include "bt.h"

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

typedef System XMainComp;

typedef struct {
	U8 nSystemsMax;
	U8 nSystems;
	Biome *biomeP;
	System **sysPA;
} dunnoYet;

typedef struct {
	U8 nSystemsMax;
	U8 nSystems;
	Biome *biomeP;
	System **sysPA;
} XMainIniSysPrms;

typedef struct {
	System system;
	Map *sharedMP;         // map of maps of shared "components"
	Biome *biomeP;
} XMainSystem; 

Error xIni(System **sPA, U16 nSystems, U8 nSystemsMax, Biome *biomeP);
Error xMainIniSys(System *sP, void *sParamsP);
Error xMainIni(XMainSystem **xMainSysPP, System **sPA, U16 nSystems, U8 nSystemsMax, Biome *biomeP);
#endif
