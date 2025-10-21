#include "tau.h"
#include "xGeneric.h"
#include "share.h"
#include "gene.h"
#include "genericSysGenes.h"

TAU_MAIN();

// Raw gene bodies - Only mutations need a separate declaration.
GenericMutableShortChar 
mutBody1a = {
  .s = 1234,
  .c = 100
},
  mutBody1b = {
    .s = 1235,
    .c = 101
  },
  mutBody1c = {
    .s = 1236,
    .c = 102
  },
  mutBody2a = {
    .s = 123,
    .c = 10
  },
  mutBody2b = {
    .s = 124,
    .c = 11
  };

// Bundle above gene bodies into genes
GenericImmutableGene 
imm1Gene = {
  .hdr = {
    .class = IMMUTABLE,
    .u.type = GENERIC
#ifndef NEDBUG
      ,
    .size = sizeof(GenericImmutableInt),
    .typeName = "GenericImmutableInt"
#endif
  },
  .body = 1,
},
  imm2Gene = {
    .hdr = {
      .class = IMMUTABLE,
      .u.type = GENERIC
#ifndef NEDBUG
        ,
      .size = sizeof(GenericImmutableInt),
      .typeName = "GenericImmutableInt"
#endif
    },
    .body = 2
  };

// Immutables can be raw data beneath headers, but  mutables ought to use data pointers for reusability.
Mutation mutations1A[] = {
  {
    .key = 1,
    .mutationBodyP = &mutBody1a
  },
  {
    .key = 2,
    .mutationBodyP = &mutBody1b
  },
  {
    .key = 3,
    .mutationBodyP = &mutBody1c
  }
};

Mutation mutations2A[] = {
  {
    .key = 1,
    .mutationBodyP = &mutBody2a
  },
  {
    .key = 2,
    .mutationBodyP = &mutBody2b
  }
};

MutableGene mut1Gene = {
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
},

  mut2Gene = {
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

// Intercomposite 1's header array
// TODO put the immutable gene and mutable gene together here
// I think i have a better understanding of how this should work out:
//    The top-level ONLY should have type = system.
//    Each sub-level will have class (esp. mutables) and sub-types.
//    Sub-types, denoted by type, will tell the system what 
//    However, the problemw ith that is that there's only ONE type that can go into a system if you
//    hard-code the genes. It's almost like, for most systems, it should stay composite.
//    Unless we can make a ready-made gene.c helper function.
struct GeneHdr* comp1HdrA[] = { &imm1Gene.hdr, &mut1Gene.hdr };
struct GeneHdr* comp2HdrA[] = { &imm2Gene.hdr, &mut2Gene.hdr  };

// IntraCompositeGene for generic system
IntraCompositeGene intra1 = {
  .hdr = {
    .class = INTRACOMPOSITE,
    .u.type = GENERIC
#ifndef NDEBUG
      ,
    .size = sizeof(XGenericComp),
    .typeName = "XGeneric"
#endif
  },
  .geneHdrPA = comp1HdrA
},

  intra2 = {
    .hdr = {
      .class = INTRACOMPOSITE,
      .u.type = GENERIC
#ifndef NDEBUG
        ,
      .size = sizeof(XGenericComp),
      .typeName = "XGeneric"
#endif
    },
    .geneHdrPA = comp1HdrA
  };

GeneHdr* geneHdrPA[] = { intra1, intra2 };

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
  .geneHdrPA = geneHdrPA

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
