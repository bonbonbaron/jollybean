#include "tau/tau.h"
#include "xA.h"

/* STRUCTURE OF AN XA COMPONENT:
 *
 * The component has four subcomponents.
 *   A and D are immutable subcomponents;
 *   B and C are mutable.
 *
 * Therefore, we'll store a one-to-one mapping from each entity to its A and D subcomponents,
 * and then we'll store a one-to-many mapping from each entity to its B and C subcomponents.
 *
 * B and C will also undergo postmutations to fully test that feature of X.
 *
 */

TAU_MAIN()

typedef struct Tau {
  Error e;
  U32 nEntities;
  U32 nMutationsPerEntity;
  XA *xP;
  Map *bMP;  // map of "b" mutations
  Map *cMP;  // map of "c" mutations
  Map *shareMPMP;
  System *sP;
} Tau;

static Error _validateEntityPosition(Tau *tau, Entity entity, Key expectedIdx) {
  Key *idxP = mapGet(tau->sP->e2cIdxMP, entity);
  if (*idxP != expectedIdx) {
    printf("error: entity %d is in index %d, expected to be at %d.\n", entity, *idxP, expectedIdx);
    return 1;
  }
  if (tau->sP->cIdx2eA[*idxP] != entity) {
    printf("error: index %d has entity %d, expected entity to be %d\n", *idxP, tau->sP->cIdx2eA[*idxP], entity);
    return 1;
  }
  return 0;
}

// We have to make these constants so the compiler doesn't cry about the array initializers not having constant sizes.
#define N_ENTITIES (100)
#define N_MUTATIONS_PER_ENTITY (5)
#define requireSuccess_ REQUIRE_EQ(tau->e, SUCCESS)
#define forEachEntity_(nEntities_) for (Entity entity = 1; entity <= nEntities_; ++entity)

#define A_COEFF (1)
#define B_COEFF (2)
#define C_COEFF (3)
#define D_COEFF (4)

TEST_F_SETUP(Tau) {
  extern XA xA;
  tau->xP = &xA;
  tau->nEntities = N_ENTITIES;
  tau->nMutationsPerEntity = N_MUTATIONS_PER_ENTITY;
  tau->e = xIniSys(sAP, tau->nEntities, NULL);
  requireSuccess_;
  REQUIRE_EQ(sAP->compSz, sizeof(XAComp));
  REQUIRE_EQ(frayGetNElems_((void**) sAP->cF), tau->nEntities);
  // These will store the entities' immutable subcomponent data since xAddEntityData needs a pointer to them.
  int aA[N_ENTITIES] = {0};
  double dA[N_ENTITIES] = {0};

  // Populate all the entities' subcomponents.
  for (Entity entity = 1; entity <= tau->nEntities; ++entity) {
    XAComp xAComp = {0};

    // ************ MUTATIONS **************
    // Make a mutation map for the current entity.
    Map *currEntitysMutationMP = NULL;
    tau->e = mapNew(&currEntitysMutationMP, RAW_DATA, sizeof(XAMutation), tau->nMutationsPerEntity);
    requireSuccess_;
    // Pre-populate the mutation maps with arbitrary values.
    for (Key i = 1; !tau->e && i <= tau->nMutationsPerEntity; ++i) {
      XAMutation currMutation = {
        .bb = i * B_COEFF,  // mutation key times 2
        .cc = i * C_COEFF   // mutation key times 3
      };
      tau->e = mapSet(currEntitysMutationMP, i, &currMutation);
      requireSuccess_;
    }
    // Now that you've populated this entity's mutation map, add it to the system.
    tau->e = xAddMutationMap(&tau->xP->system, entity, currEntitysMutationMP);
    requireSuccess_;

    // ************ SHARES **************
    // TODO make a share map for *intP here.
    Map *shareMP = NULL;
    // Make the share outer map. This maps datatypes to inner maps. We'll only store one map in there.
    tau->e = mapNew(&tau->shareMPMP, MAP_POINTER, sizeof(Map*), 1);
    requireSuccess_;
    // Make the share inner map. This maps entities to actual, raw data.
    tau->e = mapNew(&shareMP, RAW_DATA, sizeof(Map*), tau->nMutationsPerEntity);
    requireSuccess_;
    // Map the inner share map to key value "1" in the outer shared map.
    tau->e = mapSet(tau->shareMPMP, 1, &shareMP);
    // Give the shared map to the system. (This particular system wants a pointer to the inner shared map.)
    tau->e = tau->xP->system.getShare(&tau->xP->system, tau->shareMPMP);

    // Make arrays of the immutable subcomponents. 
    // The subcomponent intializers consume only pointers to the original data,
    // so we have to store them in arrays here.
    aA[entity - 1] = entity * A_COEFF;
    dA[entity - 1] = entity * D_COEFF;
    tau->e = xAddEntityData(&tau->xP->system, entity, 0x40 | XA_TYPE, &aA[entity - 1]);
    requireSuccess_;
    tau->e = xAddEntityData(&tau->xP->system, entity, 0x80 | XA_TYPE, &dA[entity - 1]);
    requireSuccess_;
  }
  // Make the system make full components from all the subcomponents.
  tau->e = tau->xP->system.postprocessComps(&tau->xP->system);
  requireSuccess_;
  tau->sP = &tau->xP->system;
}

TEST_F_TEARDOWN(Tau) {
  xClr(&tau->xP->system);
}

void validateProcessedMutation( Entity entity, Key mutationKey, XAComp *cP ) {
  CHECK_EQ(cP->a, ( entity * A_COEFF ) * ( mutationKey * B_COEFF + 1 ) - 1 );
  CHECK_EQ(cP->b, mutationKey * B_COEFF + 1);
  CHECK_TRUE(cP->c == mutationKey * C_COEFF - 1);
  CHECK_EQ(cP->d, cP->a + 1 );
}

void validateUnprocessedMutation( Entity entity, Key mutationKey, XAComp *cP ) {
  printf("entity %d\n", entity);
  CHECK_EQ(cP->a, ( entity * A_COEFF ) * ( cP->b + 1 ) );
  CHECK_EQ(cP->b, mutationKey * B_COEFF );
  CHECK_TRUE(cP->c == mutationKey * C_COEFF );
  CHECK_EQ(cP->d, ( entity * D_COEFF ) * ( cP->c - 2 ) );
}

void validateUnprocessedUnmutated( Entity entity, XAComp *cP ) {
  CHECK_EQ(cP->a, ( entity * A_COEFF ) );
  CHECK_EQ(cP->b, 0 );
  CHECK_TRUE(cP->c == 0 );
  CHECK_EQ(cP->d, entity * D_COEFF );
}

void validateProcessedUnmutated( Entity entity, XAComp *cP ) {
  CHECK_EQ(cP->a, ( entity * A_COEFF ) - 1 );
  CHECK_EQ(cP->b, 1 );
  CHECK_TRUE(cP->c == 255 );
  CHECK_EQ(cP->d, cP->a + 1 );
}

//--------------------------------------------
// Actual tests
//--------------------------------------------

TEST_F(Tau, xGetCompPByEntity) {
  // Make sure you can get every component properly by its entity ID.
  forEachEntity_(tau->nEntities) {
    XAComp *cP = (XAComp*) xGetCompPByEntity(tau->sP, entity);
    validateUnprocessedUnmutated( entity, cP );
  }
}

TEST_F(Tau, xGetEntityByCompIdx) {
  // Make sure you can get every entity properly by its component index.
  forEachEntity_(tau->nEntities) {
    Entity _entity = xGetEntityByCompIdx(tau->sP, entity - 1);
    CHECK_EQ(_entity, entity);
  }
}

TEST_F(Tau, xActivateComponentByEntity) {
  // Write a couple activation messages to the system.
  tau->e = mailboxWrite(tau->sP->mailboxF, 1, 10, ACTIVATE, 0);
  requireSuccess_;
  tau->e = mailboxWrite(tau->sP->mailboxF, 1, 50, ACTIVATE, 0);
  requireSuccess_;

  // Run the system so it reads its inbox letters from the above.
  tau->e = xRun(tau->sP);
  requireSuccess_;

  XAComp *cP = tau->sP->cF; 
  XAComp *cEndP = cP + *_frayGetFirstEmptyIdxP(tau->sP->cF);
  for (; cP < cEndP; ++cP) {
    // Make sure the two activated components are moved to the front of the fray AND marked as active.
    Entity entity = xGetEntityByVoidComponentPtr( tau->sP, cP );
    if ( entity == 10 || entity == 50 ) {
      validateProcessedUnmutated( entity, cP );
    }
    else {
      validateUnprocessedUnmutated( entity, cP );
    }
  }
}

TEST_F( Tau, mutateAndActivate) {
  // Write a couple activation messages to the system.
  tau->e = mailboxWrite(tau->sP->mailboxF, 1, 10, MUTATE_AND_ACTIVATE, 1);
  requireSuccess_;
  tau->e = mailboxWrite(tau->sP->mailboxF, 1, 50, MUTATE_AND_ACTIVATE, 1);
  requireSuccess_;

  // Run the system so it reads its inbox letters from the above.
  tau->e = xRun(tau->sP);
  requireSuccess_;

  XAComp *cP = tau->sP->cF; 
  
  validateProcessedMutation( 10, 1, cP++ );
  validateProcessedMutation( 50, 1, cP );
  CHECK_FALSE(_frayElemIsActive(tau->sP->cF, 3));
}

TEST_F( Tau, mutateAndDeactivate) {
  // Write a couple activation messages to the system.
  tau->e = mailboxWrite(tau->sP->mailboxF, 1, 10, MUTATE_AND_DEACTIVATE, 1);
  requireSuccess_;
  tau->e = mailboxWrite(tau->sP->mailboxF, 1, 50, MUTATE_AND_DEACTIVATE, 1);
  requireSuccess_;

  // Run the system so it reads its inbox letters from the above.
  tau->e = xRun(tau->sP);
  requireSuccess_;

  XAComp *cP = tau->sP->cF; 
  XAComp *cEndP = cP + *_frayGetFirstEmptyIdxP(tau->sP->cF);
  for (; cP < cEndP; ++cP) {
    Entity entity = xGetEntityByVoidComponentPtr( tau->sP, cP );
    if ( entity == 10 || entity == 50 ) {
      validateUnprocessedMutation( entity, 1, cP );
    }
    else {
      validateUnprocessedUnmutated( entity, cP );
    }
  }
}

TEST_F(Tau, xDeactivateComponentByEntity) {
  FrayChanges fc = {0};
  // make sure first elem ain't active
  CHECK_FALSE(_frayElemIsActive(tau->sP->cF, 0));
  // activate entity 10
  tau->e = mailboxWrite(tau->sP->mailboxF, 1, 10, ACTIVATE, 0);
  requireSuccess_;
  // deactivate entity 10
  tau->e = mailboxWrite(tau->sP->mailboxF, 1, 10, DEACTIVATE, 0);
  requireSuccess_;

  // RUn it.
  tau->e = xRun(tau->sP);
  requireSuccess_;

  // make sure teh first element is the one we moved there
  // BEcause we deactivated it rihgt after activation, it didn't have a chance to run.
  XAComp *cP = (XAComp*) tau->sP->cF;
  Entity entity = xGetEntityByVoidComponentPtr( tau->sP, cP );
  CHECK_EQ(cP->a, entity * A_COEFF );
  CHECK_EQ(cP->b, 0 );
  CHECK_TRUE(cP->c == 0 );
  CHECK_EQ(cP->d, entity * D_COEFF );
  // make sure it's also inactive
  CHECK_FALSE(_frayElemIsActive(tau->sP->cF, 0));
}

TEST_F(Tau, xPauseComponentByEntity) {
  // Activate the first 3 entities
  for (Entity entity = 1; entity <= 3; ++entity) {
    tau->e = mailboxWrite(tau->sP->mailboxF, 1, entity, ACTIVATE, 0);
    requireSuccess_;
  }
  // Pause the middle one
  tau->e = mailboxWrite(tau->sP->mailboxF, 1, 2, PAUSE, 0);
  requireSuccess_;
  tau->e = xRun(tau->sP);
  requireSuccess_;
  // Make sure the ordering is correct in the entity-to-component array.
  CHECK_EQ(_validateEntityPosition(tau, 1, 0), SUCCESS);
  CHECK_EQ(_validateEntityPosition(tau, 2, 2), SUCCESS);
  CHECK_EQ(_validateEntityPosition(tau, 3, 1), SUCCESS);
}

TEST_F(Tau, xUnpauseComponentByEntity) {
  // Repeat the same test as the above so we can unpause the paused element.
  // Activate the first 3 entities
  for (Entity entity = 1; entity <= 3; ++entity) {
    tau->e = mailboxWrite(tau->sP->mailboxF, 1, entity, ACTIVATE, 0);
    requireSuccess_;
  }
  // Pause the middle one
  tau->e = mailboxWrite(tau->sP->mailboxF, 1, 2, PAUSE, 0);
  tau->e = xRun(tau->sP);
  requireSuccess_;
  // Make sure the ordering is correct in the entity-to-component array.
  // Entity 1's still in index 0
  CHECK_EQ(_validateEntityPosition(tau, 1, 0), SUCCESS);
  CHECK_EQ(_validateEntityPosition(tau, 2, 2), SUCCESS);
  CHECK_EQ(_validateEntityPosition(tau, 3, 1), SUCCESS);
  // Unpause entity 2
  tau->e = mailboxWrite(tau->sP->mailboxF, 1, 2, UNPAUSE, 0);
  tau->e = xRun(tau->sP);
  requireSuccess_;
  // Entity 2's still in index 2
  CHECK_EQ(_validateEntityPosition(tau, 2, 2), SUCCESS);
}

TEST_F(Tau, xGetNComps) {
  CHECK_EQ(xGetNComps(tau->sP), tau->nEntities);
}

TEST_F(Tau, xMutateComponent) {
  // Request mutation.
  tau->e = mailboxWrite(tau->sP->mailboxF, 1, 5, MUTATE, 7);  // will this work w/ only 5 mutations?
  requireSuccess_;
  XAComp *cP = (XAComp*) xGetCompPByEntity(tau->sP, 5);
  validateUnprocessedUnmutated( 5, cP );
  // Mutate it.
  tau->e = xRun(tau->sP);
  requireSuccess_;
  cP = (XAComp*) xGetCompPByEntity(tau->sP, 5);
  // Reason we treat it as unmutated is because
  // we don't have a mutation for key 7.
  validateUnprocessedUnmutated( 5, cP );
}

TEST_F(Tau, xGetEntityByVoidComponentPtr) {
  Entity entity = xGetEntityByVoidComponentPtr(tau->sP, (void*) &((XAComp*) tau->sP->cF)[8]);  // Talk about ugly.
  CHECK_EQ(entity, 9);
}

TEST_F(Tau, xQueuePause) {
  // Queue entities 11, 21, and 31 for pausing.
  for (int i = 1; i <= 3; ++i) {
    void *voidCP = (void*) &((XAComp*) tau->sP->cF)[i * 10];
    tau->e = xQueuePause(tau->sP, voidCP);
    requireSuccess_;
  }
  // Pause them.
  tau->e = xRun(tau->sP);
  // Check that they're paused.
  for (int i = 1; i <= 3; ++i) {
    CHECK_EQ(_validateEntityPosition(tau, i * 10 + 1, i - 1), SUCCESS);
    // Make sure it's paused in the fray.
    CHECK_TRUE(_frayElemIsPaused(tau->sP->cF, i - 1));
  }
}

TEST_F(Tau, xQueueDeactivate) {
  // Queue entities 11, 21, and 31 for deactivating.
  for (int i = 1; i <= 3; ++i) {
    void *voidCP = (void*) &((XAComp*) tau->sP->cF)[i * 10];
    tau->e = xQueueDeactivate(tau->sP, voidCP);
    requireSuccess_;
  }
  // Deactivate them.
  tau->e = xRun(tau->sP);
  // Check that they're deactivated.
  for (int i = 1; i <= 3; ++i) {
    CHECK_EQ(_validateEntityPosition(tau, i * 10 + 1, i * 10), SUCCESS);
    // Make sure it's deactivated in the fray.
    CHECK_TRUE(!_frayElemIsActive(tau->sP->cF, i * 10));
  }
}

TEST_F(Tau, ActivateAcrossPause) {
  // First let's set up a 3-activated and 3-paused situation before we run our test.
  // Activate 3.
  for (Entity entity = 1; entity <= 3; ++entity) {
    tau->e = mailboxWrite(tau->sP->mailboxF, 1, entity, ACTIVATE, 0);
    requireSuccess_;
  }
  // Pause 3.
  for (Entity entity = 4; entity <= 6; ++entity) {
    tau->e = mailboxWrite(tau->sP->mailboxF, 1, entity, PAUSE, 0);
    requireSuccess_;
  }
  // Process requests.
  tau->e = xRun(tau->sP);
  requireSuccess_;
  // Now activate 2 more across the pauses.
  for (Entity entity = 7; entity <= 8; ++entity) {
    tau->e = mailboxWrite(tau->sP->mailboxF, 1, entity, ACTIVATE, 0);
    requireSuccess_;
  }
  // Process requests.
  tau->e = xRun(tau->sP);
  requireSuccess_;
  CHECK_EQ(xGetEntityByCompIdx(tau->sP, 0), 1);
  CHECK_EQ(xGetEntityByCompIdx(tau->sP, 1), 2);
  CHECK_EQ(xGetEntityByCompIdx(tau->sP, 2), 3);
  CHECK_EQ(xGetEntityByCompIdx(tau->sP, 3), 7);
  CHECK_EQ(xGetEntityByCompIdx(tau->sP, 4), 8);
  CHECK_EQ(xGetEntityByCompIdx(tau->sP, 5), 6);
  CHECK_EQ(xGetEntityByCompIdx(tau->sP, 6), 4);
  CHECK_EQ(xGetEntityByCompIdx(tau->sP, 7), 5);
}

TEST_F(Tau, miscProcesMessage) {
  tau->e = mailboxWrite(tau->sP->mailboxF, 1, 1, 20, 0);  // 20 is the command to make our default procMsg() run.
  requireSuccess_;
  tau->e = xRun(tau->sP);
  requireSuccess_;
}
