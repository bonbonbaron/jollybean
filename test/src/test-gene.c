#include "tau.h"
#include "xGeneric.h"
#include "share.h"
#include "gene.h"
#include "genericSysGenes.h"

TAU_MAIN();

#if 0
typedef struct GeneHdr {  // breaks down to 1 byte with -fshort-enums compiler flag
  const U8 class;  // For mutations, this is in the ExclusiveMutableGene's header.
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

// for now, just make a dummy root gene here to grok what's going on.
typedef struct RootGene {
  GeneHdr hdr;
  GeneHisto histo;    // histo of the entire genome so we don't have to calculate it at runtime
  GeneHdr **geneHdrPA;   // pointers prevent multiple entities with same genes from reinitializing them
} RootGene;
//
// Composite gene
typedef struct CompositeGene {  // Same information, different effect (see gene.c)
  GeneHdr hdr;  // let the header hold the count, and each individual element's header below will hold its sysId
  GeneHdr **geneHdrPA;   // pointers prevent multiple entities with same genes from reinitializing them
} CompositeGene;
#endif

// Now the deal is, I want to push just one thing into the system per entity, not a bunch of things. 
// Ideally.
// Composites are processed by looping through the array.
// See how painful this is shaping up to be is HIGHLY justifying the gene tool.

// Raw gene bodies
GenericImmutableInt 
genImm1 = 1,
        genImm2 = 2;
GenericMutableShortChar 
genMutBody1a = {
  .s = 1234,
  .c = 100
},
  genMutBody1b = {
    .s = 1235,
    .c = 101
  },
  genMutBody1c = {
    .s = 1236,
    .c = 102
  },
  genMutBody2a = {
    .s = 123,
    .c = 10
  },
  genMutBody2b = {
    .s = 124,
    .c = 11
  };

// Bundle above gene bodies into genes
GenericImmutableGene 
genImm1Gene = {
  .hdr = {
    .class = IMMUTABLE,
    .u.type = GENERIC
#ifndef NEDBUG
      ,
    .size = sizeof(GenericImmutableInt),
    .typeName = "GenericImmutableInt"
#endif
  },
  .geneBodyP = &genImm1
},
  genImm2Gene = {
    .hdr = {
      .class = IMMUTABLE,
      .u.type = GENERIC
#ifndef NEDBUG
        ,
      .size = sizeof(GenericImmutableInt),
      .typeName = "GenericImmutableInt"
#endif
    },
    .geneBodyP = &genImm2
  };

// Immutables can be raw data beneath headers, but  mutables ought to use data pointers for reusability.
Mutation mutations1A[] = {
  {
    .key = 1,
    .mutationBodyP = &genMutBody1a
  },
  {
    .key = 2,
    .mutationBodyP = &genMutBody1b
  },
  {
    .key = 3,
    .mutationBodyP = &genMutBody1c
  }
};

Mutation mutations2A[] = {
  {
    .key = 1,
    .mutationBodyP = &genMutBody2a
  },
  {
    .key = 2,
    .mutationBodyP = &genMutBody2b
  }
};

MutableGene mutGene1 = {
  .hdr = {
    .class = MUTABLE,
    .u.type = GENERIC
#ifndef NDEBUG
      ,
    .size = sizeof(mutations1A),
    .typeName = "GenericMutation"
#endif
  },
  .n = sizeof(mutations1A) / sizeof(mutations1A[0]),
  .mutationA = mutations1A
};

MutableGene mutGene2 = {
  .hdr = {
    .class = MUTABLE,
    .u.type = GENERIC
#ifndef NDEBUG
      ,
    .size = sizeof(mutations2A),
    .typeName = "GenericMutation"
#endif
  },
  .n = sizeof(mutations2A) / sizeof(mutations2A[0]),
  .mutationA = mutations2A
};


// Gotta make the mutations before I make the genes. Seems clunky. Maybe redesign this step.

// Composite 1's header array
// TODO put the immutable gene and mutable gene together here
struct GeneHdr* comp1HdrA[] = { };
struct GeneHdr* comp2HdrA[] = { };

// Composite for generic system
CompositeGene comp1 = {
  .hdr = {
    .class = COMPOSITE,
    .u.n = 2
#ifndef NDEBUG
      ,
    .size = sizeof(XGenericComp),
    .typeName = "XGeneric"
#endif
  },
  .geneHdrPA = comp1HdrA
};

RootGene root = {
  .hdr = {
    .class = ROOT,
    .u.n = 1
#ifndef NDEBUG
      ,
    .size = sizeof(RootGene),
    .typeName = "RootGene"
#endif
  },
  //GeneHisto histo;    // histo of the entire genome so we don't have to calculate it at runtime
  //GeneHdr **geneHdrPA;   // pointers prevent multiple entities with same genes from reinitializing them

};

GeneHisto histo;    // histo of the entire genome so we don't have to calculate it at runtime
struct GeneHdr **geneHdrPA;   // pointers prevent multiple entities with same genes from reinitializing them

extern XGeneric xGeneric;  // generic system

typedef struct Tau {
  XGeneric *xP;
} Tau;

TEST_F_SETUP(Tau) {
  tau->xP = &xGeneric;
  shareIni( 2 );  // Let's test gene's ability to distribute across two systems.
  distributeGenes( &root );
}

TEST_F_TEARDOWN(Tau) {
  memRst( GENERAL );
}
