#ifndef XMAIN_
#define XMAIN_
#include "xGo.h"
#include "jbInterface.h"

#define Biome_(name_, ...) Biome name_ = {\
  .nEntities = nArgs_(Genome*, __VA_ARGS__),\
  .genomePA = {__VA_ARGS__}\
}

#define Genome_(name_, ...) Genome name_ = {\
  .nGenes = nArgs_(Gene*, __VA_ARGS__),\
  .genePA = {__VA_ARGS__}\
}

typedef enum {ECS_SIMPLE_COMPONENT, ECS_MAPPED_COMPONENT, ECS_SHARED, BLACKBOARD} GeneClass;

// Note: this header is attached to each individual gene.
typedef struct {
	U8 geneClass;
	U8 type;
	U8 size;
	U8 padding;
  void *dataP;
} Gene;

typedef struct {
  U8 nGenes;
	Gene *genePA[];
} Genome;

typedef struct {
	Genome *genomeP;
  BBSeed *bbSeedP;
	Personality *personalityP; 
} Seed;

typedef struct {
  U32 nEntities;
  Seed *seedPA[];
} Biome;

typedef System XMainCompData;
typedef struct {
  XHeader xHeader;
  XMainCompData data;
} XMainComp;

typedef struct {
	U8 nXSystemsMax;
	U8 nXSystems;
	Biome *biomeP;
	System **xSysPA;  // This array contains all the systems this app will ever need.
  System *behaviorSysP;
} XMainIniSysPrms;

typedef struct {
	System system;
	Map *sharedMMP;         // map of maps of shared "components"
	Biome *biomeP;
	Window_ *windowP;
	Renderer_ *rendererP;
} XMain;

Error xIni(System **sPA, U16 nSystems, U8 nSystemsMax, Biome *biomeP);
Error xMainIniSys(System *sP, void *sParamsP);
Error xMainIniComp(System *sP, void *dataP);
Error xMainIni(XMain **xMainSysPP, System **sPA, U16 nSystems, U8 nSystemsMax, Biome *biomeP);
Error xMainGetShare(System *sP, Map *shareMMP);
Error xMainProcessMessage(System *sP, Message *msgP);
Error xMainClr(System *sP);
#endif
