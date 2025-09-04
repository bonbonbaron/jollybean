#ifndef GENE_
#define GENE_

// Gene histo
typedef struct GeneHisto {
  // TODO finish designing so we know what to do about  mutable and immutable exclusives.
  U32 *nExclusivesA;          // determines each subsystem's number of components 
  // /*MAYBE UNNEEDED*/ U32 *nSharesA;              // determines share-maps' # of elements
  // /*MAYBE UNNEEDED*/ U32  nDistinctShareTypes;   // determines number of maps of shared elements
  // /*MAYBE UNNEEDED*/ U32 *nCompositesA;          // determines number of composites in case we need to know
  U32  nDistinctMedia;        // determines # of strip data to inflate/unpack/assemble
} GeneHisto;

// Used to distinguish header pointers
typedef enum GeneClass { ROOT, SUBTREE, COMPOSITE, MEDIA, SHARED, VARIANT, EXCLUSIVE_IMMUTABLE, EXCLUSIVE_MUTABLE, IMPLICIT, BLACKBOARD } GeneClass;

// There is no "Gene" struct, strictly speaking.
// The "Gene" is the thing that proceeds after GeneHdr; it's not a void pointer.
// At system load time, the system takes/uses everything but the header.
// This design is better since the compiler will detect whether we're really pointing at a GeneHdr or not.
typedef struct GeneHdr {  // breaks down to 1 byte with -fshort-enums compiler flag
  const GeneClass class;
  // This union is useful for the following:
  //  1. Exclusives, which will use sysId to know where to go.
  //  2. Composites, which will use nGenes to know how many genes to use
  //  3. Variants, whose subtree (composite) and composite will use the same.
  //  4. 
  union {
    const SystemId  sysId;
    const U8 n;
  } u;
#ifndef NDEBUG
  const U8 size;  // when debugging size of expected type, this is handy       
  const char* typeName;  // checks both at CREATION time and RUNtime; string's stored in gene type's header.
#endif
} GeneHdr;

// Exclusive mutable gene
// Be sure to assert at tool-time that all mutables  have the same SystemId.
typedef struct ExMutGene {
  GeneHdr hdr;  // let the header hold the count, and each individual element's header below will hold its sysId
  GeneHdr **geneHdrPA;   // pointers prevent multiple entities with same genes from reinitializing them
} ExMutGene;

// Composite gene
typedef struct CompositeGene {
  GeneHdr hdr;
  GeneHdr **geneHdrPA;   // pointers prevent multiple entities with same genes from reinitializing them
} CompositeGene;

// Subtree gene
typedef CompositeGene SubtreeGene;  // same information, with added effect of incrementing entity
  
// Variant gene
typedef struct VariantGene {          // Variants allow you to read a tree once and copy it mulitple times with 
  GeneHdr hdr;
  SubtreeGene subtree;       // small variations, indicated in variations A. Those get tacked on.
  CompositeGene variations;  // They can either add a new gene or override an existing one.
} VariantGene;  

// Root gene
typedef struct RootGene {
  GeneHdr hdr;
  GeneHisto histo;    // histo of the entire genome so we don't have to calculate it at runtime
  GeneHdr **geneHdrPA;   // pointers prevent multiple entities with same genes from reinitializing them
} RootGene;

/* Implicit genes supply genes that're universal to all instances of a genome/subtree.
 * e.g. if every single townsperson randomly walks then randomly waits, I don't want to
 * waste space providing that Personality for every Townsperson instance.
 * Instead, I want to give each his 
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
