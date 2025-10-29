#include "tau.h"
#include "gene/gene.h"
// Systems
#include "xGeneric.h"
#include "xMedia.h"
// Gene data structure definitions
#include "genericSysGenes.h"
#include "mediaSysGenes.h"

#define nElems_(x_) ( sizeof( x_ ) / sizeof( x_[0] ) )

extern IntraCompositeGene intra1, intra2;
extern MutableGene mut1Gene, mut2Gene;
extern GenericImmutableGene imm1Gene, imm2Gene;
extern Mutation mutations1A[], mutations2A[];
extern int nMuts1, nMuts2;
extern MediaGene med1Gene, med2Gene;

TAU_MAIN();
extern XGeneric xGeneric;  // generic system
extern XMedia xMedia;  // media system
const System* sysPA[] = { &xGeneric.system, &xMedia.system };
#define NSYSTEMS ( nElems_( sysPA ) )  // because C compilers cry if you size an array by a variable, even if it's const

struct GeneHdr* geneHdr1PA[] = { &intra1.hdr, &med1Gene.hdr };
struct GeneHdr* geneHdr2PA[] = { &intra2.hdr, &med2Gene.hdr };
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
int N_GENERIC_COMPS = sizeof (subtreePA ) / sizeof( subtreePA[0] );
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
  _testMutations( tau, mutations1A, nMuts1, 1 );
  _testMutations( tau, mutations2A, nMuts2, 2 );
  _testMutations( tau, mutations1A, nMuts1, 3 );  // entity 3 uses entity 1's mutations
  // Skipping entity 4, who lacks mutables
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
