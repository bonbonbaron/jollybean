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

struct GeneHdr* geneHdrPA[] = { &intra1.hdr, &intra2.hdr };

U32 nExclusivesA[ N_SYSTEM_TYPES ] = {0};

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
  .histo = {
    .nExclusivesA = nExclusivesA,
    .nDistinctMedia = 0
  },
  .geneHdrPA = geneHdrPA
};

extern XGeneric xGeneric;  // generic system

typedef struct Tau {
  XGeneric *xP;
} Tau;

TEST_F_SETUP(Tau) {
  shareIni();
  nExclusivesA[GENERIC] = 2;
  shareSetSystem( &xGeneric.system );
  tau->xP = &xGeneric;
  shareIni();  // Let's test gene's ability to distribute across two systems.
  distributeGenes( &root );
}

TEST_F( Tau, FirstOneLezSeeHowItGoez ) {
  REQUIRE_TRUE( 1 );
}

TEST_F_TEARDOWN(Tau) {
  memRst( GENERAL );
}
