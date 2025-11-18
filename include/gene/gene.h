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
typedef enum GeneClass { ROOT, INTERCOMPOSITE, INTRACOMPOSITE, IMMUTABLE, MUTABLE, N_GENE_CLASSES } GeneClass;

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

typedef struct ImmutableGene {
  GeneHdr hdr;
  void* bodyP;
} ImmutableGene;

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
//   INTERcomposites have (sub-)components distributed across multiple systems. 
//                   These use u.n for count.
//   INTRAcomposites have sub-components distributed only to one system. 
//                   These use geneP->u.type and geneP->n for count.
typedef struct InterCompositeGene {  // Same information, different effect (see gene.c)
  GeneHdr hdr;  // let the header hold the count, and each individual element's header below will hold its sysId
  GeneHdr **geneHdrPA;   // pointers prevent multiple entities with same genes from reinitializing them
} InterCompositeGene;

typedef struct IntraCompositeGene {  // Same information, different effect (see gene.c)
  GeneHdr hdr;  // let the header hold the count, and each individual element's header below will hold its sysId
  Key n;        // since hdr uses u.type to indicate system, this field is required. 
  GeneHdr **geneHdrPA;   // pointers prevent multiple entities with same genes from reinitializing them
} IntraCompositeGene;

// TODO
//  1. Allow overrides of immutables with mutatables.
//  2. Allow overrides of mutables with immutables.
//  3. Allow overrides of immutables with intracomposites.
//  4. Allow overrides of mutables with intracomposites.
//  5. Allow overrides of intracomposites with mutables.
//  6. Allow overrides of intracomposites with immutables.
// Derivative gene
#if 0
typedef struct OverridingTweak {
  GeneHdr additivePA;  // These genes DON'T exist in parent; they override them.
} OverridingTweak;
typedef struct SubstitutingTweak {
  GeneHdr substitutivePA;  // These genes override ones existing in parent (aka pivot).
} SubstitutingTweak;
typedef struct OvrdSubstTweak {
  GeneHdr additivePA;  // These genes DON'T exist in parent; they override them.
  GeneHdr substitutivePA;  // These genes override ones existing in parent (aka pivot).
} OvrdSubstTweak;

typedef struct DerivativeGene { // Variant genes allow you to reuse a genome with small changes without wasting space.
  GeneHdr hdr;  // u.n will be number of variations (and hence the number of entities produced from this)
  GeneHdr* pivotGeneP;       // small variations, indicated in variations A. Those get tacked on.
  Tweak* tweakP;  // Each set of tweaks makes one new entity.
} DerivativeGene;

typedef struct DerivativeVectorGene {
  GeneHdr hdr;  // u.n will be number of variations (and hence the number of entities produced from this)
  GeneHdr* pivotGeneP;       // small variations, indicated in variations A. Those get tacked on.
  Tweak** tweakPA;  // Each set of tweaks makes one new entity.
} DerivativeVectorGene;
#endif

// Root gene
typedef struct RootGene {
  GeneHdr hdr;
  GeneHisto histo;    // histo of the entire genome so we don't have to calculate it at runtime
  GeneHdr **genomePA;   // pointers prevent multiple entities with same genes from reinitializing them
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
