#ifndef XMAIN_
#define XMAIN_
#define MASTER_ (1)
#include "x.h"
#include "strip.h"
#include "interface.h"
#include "fray.h"

#define geneName_(name_) name_##Gene
#define genomeName_(name_) name_##Genome
#define compositeName_(name_) name_##Composite

#define Genome_(name_, ...) \
  Gene* name_##GenomeGenePA[] = {\
    __VA_ARGS__\
  };\
  Genome genomeName_(name_) = {\
    .nGenes = nArgs_(Gene*, __VA_ARGS__),\
    .genePA = name_##GenomeGenePA\
  };

#define UnitaryGene_(name_, geneClass_, geneType_, key_, dataType_)\
  Gene geneName_(name_) = {\
    .geneClass = geneClass_,\
    .u.unitary = {\
      .dataP = &name_,\
      .key = key_,\
      .size = sizeof(dataType_),\
      .type = geneType_\
    }\
  };

#define CompositeGene_(name_, ...)\
  /* Genome that's gonna act like a composite gene */\
  Gene* name_##Components[] = {\
    __VA_ARGS__\
  };\
  /* Composite gene */\
  Gene geneName_(name_) = {\
    .geneClass = COMPOSITE_GENE,\
    .u.composite = {\
      .genePA = name_##Components,\
      .nGenes = nArgs_(Gene*, __VA_ARGS__)\
    }\
  };


#define Biome_(name_, ...) Biome name_ = {\
  .nEntities = nArgs_(Spawn, __VA_ARGS__),\
  .spawnA = {__VA_ARGS__}\
}

// Media genes are inflated, unpacked, and assembled into original data.
typedef enum {SCENE_START, SCENE_CHANGE, SCENE_STOP} SceneAction;

typedef U32 MediaGeneHistoElem;

typedef struct {
  U32 *nMutationsPerSpawnAA;  // X dimension: entity #s. Y dimension: system #s.
  // TODO finish designing so we know what to do about  mutable and immutable exclusives.
  U32 *nExclusivesA;          // determines each subsystem's number of components 
  U32 *nSharesA;              // determines share-maps' # of elements
  U32  nDistinctShareTypes;   // determines number of maps of shared elements
  U32 *nCompositesA;          // determines number of composites in case we need to know
  U32  nDistinctMedia;        // determines # of strip data to inflate/unpack/assemble
} GeneHisto;

/**************************/
/******** GENOME  *********/
/**************************/
struct _Gene;

typedef struct {
  U8 nGenes;
  U8 type;
	struct _Gene **genePA;   // pointers prevent multiple entities with same genes from reinitializing them
} Genome;

typedef Genome Composite;  // We can lego block genomes together to form more complex entites.

typedef struct _Gene {
	U8 geneClass;    // exclusive, shared, blackboard, media, or composite gene
  union {
    struct {
      U8 type;         // lower 6 bits hold system this belongs to, upper 2 is component subtype
      U8 size;         // sizeof destination component type (so we can memcpy the right size into the ECS target system/sharedPool/BB)
      Key key;         // key that lets you mutate a seed's gene to this one; 0 for immutable
      void *dataP;     // the location of the gene's actual data
    } unitary;
    Composite composite;
  } u;
} Gene;

/* Implicit genes are shared genes that start out empty, which would be a 
   waste to store in ROM (not to mention annoying to have to remember).
   Better to automate them. */
typedef struct {
  U8 type;
  U8 size;
} ImplicitGene;

typedef struct {
  U32 nGenes;
  ImplicitGene *listA;
} ImplicitGenesList;

/************************/
/******** BIOME *********/
/************************/
typedef struct {
  Key keyhole;
  Position position;
} PositionNode;

typedef struct {
  U8 nEntitiesPossible;  // total number of positions in position node array
  U8 nEntitiesToSpawn;  // number of position nodes whose keyholes the *keyP fits
  Genome *genomeP;
  Key *keyP;   // if an entity can spawn in one of multiple places, this determines which one.
  Map **geneMutationMPA;  // array of pointers to maps of gene mutations; ptrs are distro'd to entities
  PositionNode *positionNodeA;  // all possible places these seeds can spawn
} Spawn;

typedef struct {
  Key nEntitiesToSpawn;  // number of individual entities to spawn
  Key nSpawns;  // number of spawns (a spawn can hold one or more entities, each with a uniqe position)
  Spawn *spawnA;
} Biome;

typedef enum {QUIT_ALL, SWITCH_BIOME} MasterMsgType;
typedef System* XMasterComp;  // master component is a pointer to a child system

typedef struct {
	U8 nXSystemsMax;
	U8 nXSystems;
	Biome *biomeP;
	System **xSysPA;  // This array contains all the systems this app will ever need.
  System *behaviorSysP;
} XMasterIniSysPrms;

typedef struct {
	System system;
	Map *sharedMPMP;         // map of maps of shared "components"
	Biome *biomeP;
  Gui   *guiP;
} XMaster;

typedef struct {} XMasterMutation;

extern XMaster xMaster;
extern System *sMasterP;


Error xMasterPostprocessComps(System *sP);  // If components are composites, piece them together here.
Error xIni(System **sPA, U16 nSystems, U8 nSystemsMax, Biome *biomeP);
Error xMasterIniSys(System *sP, void *sParamsP);
Error xMasterPostMutate(System *sP, void *cP);
XIniSubcompFuncDef_(Master);
Error xMasterIni(XMaster *xMasterSysP, System **sPA, U16 nXSystems, Key nXSystemsMax, Biome *biomeP);
Error xMasterGetShare(System *sP, Map *shareMMP);
Error xMasterNewShareMap(Map **sharedGenesMPP, GeneHisto *geneHistoP);
void  xMasterDelShareMap(Map **sharedGenesMPMPP);
Error xMasterProcessMessage(System *sP, Message *msgP);
Error xMasterClr(System *sP);
Error xMasterRun(System *sP);
#endif
