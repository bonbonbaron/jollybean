#include "xGeneric.h"
#include "genericSysGenes.h"

#define nElems_(x_) ( sizeof( x_ ) / sizeof( x_[0] ) )

// Raw gene bodies - Only mutations need a separate declaration.
GenericMutableShortChar 
mutBody0a = {
  .s = 1234,
  .c = 100
},
  mutBody0b = {
    .s = 1235,
    .c = 101
  },
  mutBody0c = {
    .s = 1236,
    .c = 102
  },
  mutBody1a = {
    .s = 123,
    .c = 10
  },
  mutBody1b = {
    .s = 124,
    .c = 11
  };

// Bundle above gene bodies into genes
GenericImmutableGene 
imm0Gene = {
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
    .body = 2
  };

// Immutables can be raw data beneath headers, but  mutables ought to use data pointers for reusability.
Mutation mutations0A[] = {
  {
    .key = 0,
    .mutationBodyP = &mutBody0a
  },
  {
    .key = 1,
    .mutationBodyP = &mutBody0b
  },
  {
    .key = 2,
    .mutationBodyP = &mutBody0c
  }
};

Mutation mutations1A[] = {
  {
    .key = 0,
    .mutationBodyP = &mutBody1a
  },
  {
    .key = 1,
    .mutationBodyP = &mutBody1b
  }
};

MutableGene mut0Gene = {
  .hdr = {
    .class = MUTABLE,
    .u.type = GENERIC
#ifndef NDEBUG
      ,
    .size = sizeof(mutations0A),
    .typeName = "GenericMutation"
#endif
  },
  .n = nElems_(mutations0A),
  .mutationA = mutations0A
},

  mut1Gene = {
    .hdr = {
      .class = MUTABLE,
      .u.type = GENERIC
#ifndef NDEBUG
        ,
      .size = sizeof(mutations1A),
      .typeName = "GenericMutation"
#endif
    },
    .n = nElems_(mutations1A),
    .mutationA = mutations1A
  };

int nMuts0 = sizeof( mutations0A ) / sizeof( mutations0A[0] );
int nMuts1 = sizeof( mutations1A ) / sizeof( mutations1A[0] );

// Intracomposite 0's header array
struct GeneHdr* comp0HdrA[] = { &imm0Gene.hdr, &mut0Gene.hdr };
struct GeneHdr* comp1HdrA[] = { &imm1Gene.hdr, &mut1Gene.hdr  };

// IntraCompositeGene for generic system
IntraCompositeGene intra0 = {
  .hdr = {
    .class = INTRACOMPOSITE,
    .u.type = GENERIC
#ifndef NDEBUG
      ,
    .size = sizeof(XGenericComp),
    .typeName = "XGeneric"
#endif
  },
  .n = nElems_(comp0HdrA),
  .geneHdrPA = comp0HdrA
},

  intra1 = {
    .hdr = {
      .class = INTRACOMPOSITE,
      .u.type = GENERIC
#ifndef NDEBUG
        ,
      .size = sizeof(XGenericComp),
      .typeName = "XGeneric"
#endif
    },
    .n = nElems_(comp1HdrA),
    .geneHdrPA = comp1HdrA
  };

