#ifndef GENE_
#define GENE_

#define geneName_(name_) name_##Gene
#define compositeName_(name_) name_##Composite

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

typedef struct {
  // TODO finish designing so we know what to do about  mutable and immutable exclusives.
  U32 *nExclusivesA;          // determines each subsystem's number of components 
  U32 *nSharesA;              // determines share-maps' # of elements
  U32  nDistinctShareTypes;   // determines number of maps of shared elements
  U32 *nCompositesA;          // determines number of composites in case we need to know
  U32  nDistinctMedia;        // determines # of strip data to inflate/unpack/assemble
} GeneHisto;

/************************/
/******** GENE  *********/
/************************/

typedef enum {ROOT, SUBTREE, COMPOSITE, MEDIA, SHARED, VARIANT, EXCLUSIVE_IMMUTABLE, EXCLUSIVE_MUTABLE, IMPLICIT} GeneClass;

struct _Gene;

typedef struct {
  U8 nGenes;
  U8 type;
  struct _Gene **genePA;   // pointers prevent multiple entities with same genes from reinitializing them
} Composite;

typedef struct SysLevelGeneHisto {
  Key sysId;
  Key n;
} SysLevelGeneHisto;

typedef struct _Gene {
	U8 class;
  union {
    struct unitary {
      U8 systemId;     // system ID this gene belongs to 
      U8 size;         // sizeof destination component type (so we can memcpy the right size into the ECS target system/sharedPool/BB)
      Key key;         // key that lets you mutate a seed's gene to this one; 0 for immutable
      void *dataP;     // the location of the gene's actual data
    } unitary;
    struct variant {        // Variants allow you to "copy" a subtree many times while reading the tree only once. 
      struct _Gene* geneP;  // Each variation's slight difference is expressed in variations A.
      Composite variationsA;
    } variant;
    Composite composite;
    struct subtree {   // Each subtree starts a new entity.
      Composite composite;
      Key nGenes;
    } subtree;  // Only use root for a scene's top level.
    struct root {
      Composite composite;
      Key nGeneTypes;
      Key* geneTypeHistoA;  // has a histo of the entire genome so we don't have to calculate it at runtime
    } root;  // Only use root for a scene's top level.
  } u;
} Gene;

/* Implicit genes are shared genes that start out empty, which would be a 
   waste to store in ROM (not to mention annoying to have to remember to store).
   Better to create them on startup. */
typedef struct {
  U8 type;
  U8 size;
} ImplicitGene;

typedef struct {
  U32 nGenes;
  ImplicitGene *listA;
} ImplicitGenesList;

#endif
