#ifndef GENE_
#define GENE_

#include "data/strip.h"

// Gene histo
typedef struct GeneHisto {
  U32 *nExclusivesA;          // determines each subsystem's number of components 
  U32  nDistinctMedia;        // determines # of strip data to inflate/unpack/assemble
} GeneHisto;

// Used to distinguish header pointers
// IMPLICIT: "What da hail is dis?" Rather than being stored, it's data created as a *side effect* of consuming a gene. May be  unnecessary. 
typedef enum GeneClass { ROOT, SUBTREE, INTERCOMPOSITE, INTRACOMPOSITE, VARIANT, IMMUTABLE, MUTABLE, N_GENE_CLASSES } GeneClass;

// There is no "Gene" struct, strictly speaking.
// The "Gene" is the thing that proceeds after GeneHdr; it's not a void pointer.
// At system load time, the system takes/uses everything but the header.
// This design is better since the compiler will detect whether we're really pointing at a GeneHdr or not.
typedef struct GeneHdr {  // breaks down to 1 byte with -fshort-enums compiler flag
  const U8 class;  // For mutations, this is in the Exclusiveint MutableGene's header.
  // This union is useful for the following:
  //  1. Exclusives, which will use sysId to know where to go.
  //  2. Composites, which will use nGenes to know how many genes to use
  //  3. Variants, whose subtree (composite) and composite will use the same.
  //  4. 
  union {
    const U8  type;
    const U8  n;
  } u;
#ifndef NDEBUG
  const U8 size;  // when debugging size of expected type, this is handy       
  const char* typeName;  // checks both at CREATION time and RUNtime; string's stored in gene type's header.
#endif
} GeneHdr;

// NOTE: There is no explicit media gene. Anything needing inflation will be registered by each system.

// Similar to GeneHdr, but the difference in some fields' names warrants a distinction.
typedef struct Mutation {
  Key key;
  void* mutationBodyP;
} Mutation;

// Exclusive mutable gene
typedef struct MutableGene {
  GeneHdr hdr;  // header will hold system ID. Otherwise we won't know its destination. N is separate.
  Key n;
  Mutation *mutationA;   // pointers prevent multiple entities with same genes from reinitializing them
} MutableGene;

// Composite genes:
//   INTERcomposites have (sub-)components distributed across multiple systems. These use u.n.
//   INTRAcomposites have sub-components distributed only to one system. These use u.type with an extra n.
typedef struct InterCompositeGene {  // Same information, different effect (see gene.c)
  GeneHdr hdr;  // let the header hold the count, and each individual element's header below will hold its sysId
  GeneHdr **geneHdrPA;   // pointers prevent multiple entities with same genes from reinitializing them
} InterCompositeGene;

typedef struct IntraCompositeGene {  // Same information, different effect (see gene.c)
  GeneHdr hdr;  // let the header hold the count, and each individual element's header below will hold its sysId
  Key n;        // since hdr uses u.type to indicate system, this field is required. 
  GeneHdr **geneHdrPA;   // pointers prevent multiple entities with same genes from reinitializing them
} IntraCompositeGene;

// Subtree gene
typedef InterCompositeGene Subtree;  // u.class = SUBTREE, then u.n = # of intercomposites
  
// Variant gene
typedef struct VariantGene { // Variants allow you to read a tree once and copy it mulitple times with 
  GeneHdr hdr;
  Subtree subtree;       // small variations, indicated in variations A. Those get tacked on.
  InterCompositeGene variations;  // They can either add a new gene or override an existing one.
} VariantGene;  

// Root gene
typedef struct RootGene {
  GeneHdr hdr;
  GeneHisto histo;    // histo of the entire genome so we don't have to calculate it at runtime
  Subtree **subtreePA;   // pointers prevent multiple entities with same genes from reinitializing them
} RootGene;

/* Implicit genes supply genes that're universal to all instances of a genome/subtree.
 * e.g. if every single townsperson randomly walks then randomly waits, I don't want to
 * waste space (or worse, having to remember) providing that Personality for every Townsperson.
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
