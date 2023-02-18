#ifndef BIOME_H
#define BIOME_H
#include "bt.h"
#include "interface.h"

#define Biome_(name_, ...) Biome name_ = {\
  .nEntities = nArgs_(Spawn, __VA_ARGS__),\
  .spawnA = {__VA_ARGS__}\
}

#define Genome_(name_, ...) Genome genomeName_(name_) = {\
  .nGenes = nArgs_(Gene*, __VA_ARGS__),\
  .genePA = {__VA_ARGS__}\
}

#define Gene_(name_, geneClass_, type_, typeEnum_, switchU_, rawDataName_) Gene geneName_(name_) = {\
  .geneClass = geneClass_,\
  .type = typeEnum_,\
  .size = sizeof(type_),\
  .dataP = &rawDataName_,\
}

#define Personality_(name_, quirkA_) \
Personality personalityName_(name_) = {\
  .nQuirks = sizeof(quirkA_) / sizeof(quirkA_[0]),\
  .quirkA = quirkA_\
}

#define Seed_(name_, genomeName__, personalityName__) Seed seedName_(name_) = {\
  .genomeP = &genomeName_(genomeName__),\
  .personalityP = &personalityName_(personalityName__)\
}

#define geneName_(name_) name_##Gene
#define genomeName_(name_) name_##Genome
#define personalityName_(name_) name_##Personality
#define seedName_(name_) name_##Seed

// Media genes are inflated, unpacked, and assembled into original data.
typedef enum {EXCLUSIVE_GENE, SHARED_GENE, BB_GENE, MEDIA_GENE, COMPOSITE_GENE} GeneClass;
typedef enum {SCENE_START, SCENE_CHANGE, SCENE_STOP} SceneAction;

typedef struct {
  Key size;  // size of system's component
  Key count; 
  Key geneType;
  Key geneClass;
} XHistoElem;

typedef U32 BbGeneHistoElem;

typedef struct {
  Key *histoSpawnMutations;  // first dimension: entity #s. second dimension: system #s.
  XHistoElem *histoXElemA;  // determines each subsystem's number of components and share-maps' # of elements
  BbGeneHistoElem *histoBbElemA;  // determines each entity's number of blackboard elements
  Key nDistinctShareds;  // determines number of maps of shared elements
} GeneHisto;

/**************************/
/******** GENOME  *********/
/**************************/
typedef struct {
	U8 geneClass;    // exclusive, shared, blackboard, media, or composite gene
	U8 type;         // upper 6 bits hold system this belongs to, lower 2 is component subtype
	U8 size;         // sizeof destination component type (so we can memcpy the right size into the ECS target system/sharedPool/BB)
  Key key;         // key that lets you mutate a seed's gene to this one; 0 for immutable
  void *dataP;     // the destination data of the gene 
} Gene;

typedef struct {
  U8 nGenes;
	Gene *genePA[];   // pointers prevent multiple entities with same genes from reinitializing them
} Genome;

/******************************/
/******** PERSONALITY *********/
/******************************/
// A quirk is a prioritized mapping between a trigger and behavior tree.
typedef struct {
  Key    triggerKey;
  U8     priority;
  BTree *btP;
} Quirk;

typedef struct {
	Key nQuirks;
	Quirk *quirkA;
} Personality;

/***********************/
/******** SEED *********/
/***********************/
typedef struct {
	Genome *genomeP;
  Personality *personalityP;
} Seed;

typedef struct {
  S32 x;
  S32 y;
} Position;

typedef struct {
  Key keyhole;
  Position position;
} PositionNode;

typedef struct {
  U8 nEntitiesPossible;  // total number of positions in position node array
  U8 nEntitiesToSpawn;  // number of position nodes whose keyholes the *keyP fits
  Seed *seedP;
  Key *keyP;   // if an entity can spawn in one of multiple places, this determines which one.
  Map **geneMutationMPA;  // array of pointers to maps of gene mutations; ptrs are distro'd to entities
  PositionNode *positionNodeA;  // all possible places these seeds can spawn
} Spawn;

/************************/
/******** BIOME *********/
/************************/
// Entities are enumerated by seed index in the biome.
typedef struct {
  Key nEntitiesToSpawn;  // number of individual entities to spawn
  Key nSpawns;  // number of spawns (a spawn can hold one or more entities, each with a uniqe position)
  Spawn *spawnA;
} Biome;

// Union of quirk, blackboard, and tree status.
typedef struct {
	U8           priority;       // priority of this activity to compare to any other requested one
	BTree       *btP;        // current tree from system's switch
  Map         *bbMP;           // blackboard
  BTreeStatus *btStatP;        // status of tree's nodes for this activity's owner (entity)
} Activity;  

void hivemindDel(Map **hivemindMPP);
void activityMPMPDel(Map **activityMPMPP);
Error biomeExtract(Biome *biomeP, System *masterSysP, System *goSysP, Map **sharedGenesMPP, Window_ *windowP, Renderer_ *rendererP, Map **hivemindMPP, Key nSystemsMax);

#endif
