#include "tau.h"
#include "xGeneric.h"
#include "gene/gene.h"
#include "genericSysGenes.h"

#define nElems_(x_) ( sizeof( x_ ) / sizeof( x_[0] ) )

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
  .n = nElems_(mutations1A),
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
    .n = nElems_(mutations2A),
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
  .n = nElems_(comp1HdrA),
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
    .n = nElems_(comp2HdrA),
    .geneHdrPA = comp2HdrA
  };

struct GeneHdr* geneHdr1PA[] = { &intra1.hdr };
struct GeneHdr* geneHdr2PA[] = { &intra2.hdr };
struct GeneHdr* geneHdr3PA[] = { &mut1Gene.hdr };  // entity with just mutable
struct GeneHdr* geneHdr4PA[] = { &imm1Gene.hdr };  // entity with just immutable

Subtree entity1Tree = {
  .hdr = {
    .class = SUBTREE,
    .u.n = nElems_(geneHdr1PA)
  },
  .geneHdrPA = geneHdr1PA
},
  entity2Tree = {
    .hdr = {
      .class = SUBTREE,
      .u.n = nElems_( geneHdr2PA ),
    },
    .geneHdrPA = geneHdr2PA
  },
  entity3Tree = {
    .hdr = {
      .class = SUBTREE,
      .u.n = nElems_( geneHdr3PA ),
    },
    .geneHdrPA = geneHdr3PA
  },
  entity4Tree = {
    .hdr = {
      .class = SUBTREE,
      .u.n = nElems_( geneHdr4PA ),
    },
    .geneHdrPA = geneHdr4PA
  };



Subtree* subtreePA[] = { &entity1Tree, &entity2Tree, &entity3Tree, &entity4Tree };

extern XGeneric xGeneric;  // generic system
const System* sysPA[] = { &xGeneric.system };
#define NSYSTEMS ( nElems_( sysPA ) )  // because C compilers cry if you size an array by a variable, even if it's const
const Key NENTITIES = nElems_( subtreePA );
U32 nExclusivesA[ NSYSTEMS ] = { NENTITIES };

RootGene root = {
  .hdr = {
    .class = ROOT,
    .u.n = NENTITIES,
#ifndef NDEBUG
    .size = sizeof(RootGene),
    .typeName = "RootGene"
#endif
  },
  .histo = {
    .nExclusivesA = nExclusivesA,
    .nDistinctMedia = 0
  },
  .subtreePA = subtreePA
};


typedef struct Tau {
  XGeneric *xP;
} Tau;

// This simulates what may happen in the finished engine.

TEST_F_SETUP(Tau) {
  xIni( sysPA, NSYSTEMS, &root );

  // TODO revamp mailboxes to no longer use addresses 
  tau->xP = &xGeneric;
  mailboxWrite( tau->xP->system.mailboxF, GENERIC, 1, MUTATE_AND_ACTIVATE, 1, NULL );
  mailboxWrite( tau->xP->system.mailboxF, GENERIC, 2, MUTATE_AND_ACTIVATE, 1, NULL );
  mailboxWrite( tau->xP->system.mailboxF, GENERIC, 3, MUTATE_AND_ACTIVATE, 1, NULL );
  mailboxWrite( tau->xP->system.mailboxF, GENERIC, 4, ACTIVATE, 0, NULL );

  xRun( &tau->xP->system );
}

TEST_F_TEARDOWN(Tau) {}  // tau forces us to declare this

TEST_F( Tau, Intracomposites ) {
  XGenericComp* cP = (XGenericComp*) xGetCompPByEntity( &tau->xP->system, 1);
  REQUIRE_TRUE( cP != NULL );
  CHECK_EQ( cP->immutable, 1 );
  CHECK_EQ( cP->mutableCompositePc1, 1234 );
  CHECK_TRUE( cP->mutableCompositePc2 == 100 );
  cP = (XGenericComp*) xGetCompPByEntity( &tau->xP->system, 2);
  REQUIRE_TRUE( cP != NULL );
  // TODO figure out why the below are all failing.
  CHECK_EQ( cP->immutable, 2 );
  CHECK_EQ( cP->mutableCompositePc1, 123 );
  CHECK_TRUE( cP->mutableCompositePc2 == 10 );
}

static void _testMutations( Tau* tau, Mutation* mutationA, const U32 nMutations, Entity entity ) {
  XGenericComp* cP;
  Mutation* mutationP = mutationA;
  Mutation* mutationEndP = mutationP + nMutations;
  for ( ; mutationP < mutationEndP ; ++mutationP ) {
    mailboxWrite( tau->xP->system.mailboxF, GENERIC, entity, MUTATE_AND_ACTIVATE, mutationP->key, NULL );
    xRun( &tau->xP->system );
    cP = (XGenericComp*) xGetCompPByEntity( &tau->xP->system, entity);
    REQUIRE_TRUE( cP != NULL );
    CHECK_EQ( cP->mutableCompositePc1, ( (GenericMutableShortChar*) mutationP->mutationBodyP)->s );
    CHECK_TRUE( cP->mutableCompositePc2 == ( (GenericMutableShortChar*) mutationP->mutationBodyP)->c );
  }
}


TEST_F( Tau, Mutations ) {
  _testMutations( tau, mutations1A, nElems_( mutations1A ), 1 );
  _testMutations( tau, mutations2A, nElems_( mutations2A ), 2 );
  _testMutations( tau, mutations1A, nElems_( mutations2A ), 3 );  // entity 3 uses entity 1's mutations
}

// I go ahead and test immutables of entities derived from both intracomposite genes and strictly immutable.
TEST_F( Tau, Immutables ) {
  // Entity 1
  XGenericComp* cP = (XGenericComp*) xGetCompPByEntity( &tau->xP->system, 1);
  CHECK_EQ( cP->immutable, imm1Gene.body );
  // Entity 2
  cP = (XGenericComp*) xGetCompPByEntity( &tau->xP->system, 2);
  CHECK_EQ( cP->immutable, imm2Gene.body );
  // Skipping entity 3, who lacks immutables
  // Entity 4
  cP = (XGenericComp*) xGetCompPByEntity( &tau->xP->system, 4);
  CHECK_EQ( cP->immutable, imm1Gene.body );
}
