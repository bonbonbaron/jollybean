#include "tau.h"
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
  U32 nEntities;
  U32 nMutationsPerEntity;
  XA *xP;
  Map *bMP;  // map of "b" mutations
  Map *cMP;  // map of "c" mutations
  Map *sharedMPMP;
  System *sP;
} Tau;

static void _validateEntityPosition(Tau *tau, Entity entity, Key expectedIdx) {
  Key *idxP = mapGet(tau->sP->e2cIdxMP, entity);
  assert (idxP && *idxP == expectedIdx);
  assert (tau->sP->cIdx2eA[*idxP] == entity);
}

// We have to make these constants so the compiler doesn't cry about the array initializers not having constant sizes.
#define N_ENTITIES (100)
#define N_MUTATIONS_PER_ENTITY (5)
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
  xIniSys(sAP, tau->nEntities, NULL);
  REQUIRE_EQ(sAP->compSz, sizeof(XAComp));
  REQUIRE_EQ(frayGetNElems_((void**) sAP->cF), tau->nEntities);
  // These will store the entities' immutable subcomponent data since xAddEntityData needs a pointer to them.
  int aA[N_ENTITIES] = {0};
  double dA[N_ENTITIES] = {0};

  // Make the shared outer map. This maps datatypes to inner maps. We'll only store one map in there.
  tau->sharedMPMP = mapNew(MAP_POINTER, sizeof(Map*), tau->nEntities);

  // Populate all the entities' subcomponents.
  for (Entity entity = 1; entity <= tau->nEntities; ++entity) {
    XAComp xAComp = {0};

    // ************ MUTATIONS **************
    // Make a mutation map for the current entity.
    Map *currEntitysMutationMP = NULL;
    currEntitysMutationMP = mapNew(RAW_DATA, sizeof(XAMutation), tau->nMutationsPerEntity);
    // Pre-populate the mutation maps with arbitrary values.
    for (Key i = 1; i <= tau->nMutationsPerEntity; ++i) {
      XAMutation currMutation = {
        .bb = i * B_COEFF,  // mutation key times 2
        .cc = i * C_COEFF   // mutation key times 3
      };
      mapSet(currEntitysMutationMP, i, &currMutation);
    }
    // Now that you've populated this entity's mutation map, add it to the system.
    xAddMutationMap(&tau->xP->system, entity, currEntitysMutationMP);

    // ************ SHARES **************
    // TODO make a share map for *intP here.
    // Make the shared inner map. This maps entities to actual, raw data.
    Map* sharedMP = mapNew(RAW_DATA, sizeof(Map*), tau->nMutationsPerEntity);
    // Map the inner share map to key value "1" in the outer shared map.
    mapSet(tau->sharedMPMP, entity, &sharedMP);
    // Give the shared map to the system. (This particular system wants a pointer to the inner shared map.)
    tau->xP->system.getShare(&tau->xP->system, tau->sharedMPMP);

    // Make arrays of the immutable subcomponents. 
    // The subcomponent intializers consume only pointers to the original data,
    // so we have to store them in arrays here.
    aA[entity - 1] = entity * A_COEFF;
    dA[entity - 1] = entity * D_COEFF;
    xAddEntityData(&tau->xP->system, entity, 0x40 | XA_TYPE, &aA[entity - 1]);
    xAddEntityData(&tau->xP->system, entity, 0x80 | XA_TYPE, &dA[entity - 1]);
  }
  // Make the system make full components from all the subcomponents.
  tau->xP->system.postprocessComps(&tau->xP->system);
  tau->sP = &tau->xP->system;
}

TEST_F_TEARDOWN(Tau) {
  mapOfNestedMapsDel(&tau->sharedMPMP);
  xClr(&tau->xP->system);
}

void validateProcessedMutation( Entity entity, Key mutationKey, XAComp *cP ) {
  CHECK_EQ(cP->a, ( entity * A_COEFF ) * ( mutationKey * B_COEFF + 1 ) - 1 );
  CHECK_EQ(cP->b, mutationKey * B_COEFF + 1);
  CHECK_TRUE(cP->c == mutationKey * C_COEFF - 1);
  CHECK_EQ(cP->d, cP->a + 1 );
}

void validateUnprocessedMutation( Entity entity, Key mutationKey, XAComp *cP ) {
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
  mailboxWrite(tau->sP->mailboxF, 1, 10, ACTIVATE, 0, NULL);
  mailboxWrite(tau->sP->mailboxF, 1, 50, ACTIVATE, 0, NULL);

  // Run the system so it reads its inbox letters from the above.
  xRun(tau->sP);

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
  mailboxWrite(tau->sP->mailboxF, 1, 10, MUTATE_AND_ACTIVATE, 1, NULL);
  mailboxWrite(tau->sP->mailboxF, 1, 50, MUTATE_AND_ACTIVATE, 1, NULL);

  // Run the system so it reads its inbox letters from the above.
  xRun(tau->sP);

  XAComp *cP = tau->sP->cF; 
  
  validateProcessedMutation( 10, 1, cP++ );
  validateProcessedMutation( 50, 1, cP );
  CHECK_FALSE(_frayElemIsActive(tau->sP->cF, 3));
}

TEST_F( Tau, mutateAndDeactivate) {
  // Write a couple activation messages to the system.
  mailboxWrite(tau->sP->mailboxF, 1, 10, MUTATE_AND_DEACTIVATE, 1, NULL);
  mailboxWrite(tau->sP->mailboxF, 1, 50, MUTATE_AND_DEACTIVATE, 1, NULL);

  // Run the system so it reads its inbox letters from the above.
  xRun(tau->sP);

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
  mailboxWrite(tau->sP->mailboxF, 1, 10, ACTIVATE, 0, NULL);
  // deactivate entity 10
  mailboxWrite(tau->sP->mailboxF, 1, 10, DEACTIVATE, 0, NULL);

  // RUn it.
  xRun(tau->sP);

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
    mailboxWrite(tau->sP->mailboxF, 1, entity, ACTIVATE, 0, NULL);
  }
  // Pause the middle one
  mailboxWrite(tau->sP->mailboxF, 1, 2, PAUSE, 0, NULL);
  xRun(tau->sP);
  // Make sure the ordering is correct in the entity-to-component array.
  _validateEntityPosition(tau, 1, 0);
  _validateEntityPosition(tau, 2, 2);
  _validateEntityPosition(tau, 3, 1);
}

TEST_F(Tau, xUnpauseComponentByEntity) {
  // Repeat the same test as the above so we can unpause the paused element.
  // Activate the first 3 entities
  for (Entity entity = 1; entity <= 3; ++entity) {
    mailboxWrite(tau->sP->mailboxF, 1, entity, ACTIVATE, 0, NULL);
  }
  // Pause the middle one
  mailboxWrite(tau->sP->mailboxF, 1, 2, PAUSE, 0, NULL);
  xRun(tau->sP);
  // Make sure the ordering is correct in the entity-to-component array.
  // Entity 1's still in index 0
  _validateEntityPosition(tau, 1, 0);
  _validateEntityPosition(tau, 2, 2);
  _validateEntityPosition(tau, 3, 1);
  // Unpause entity 2
  mailboxWrite(tau->sP->mailboxF, 1, 2, UNPAUSE, 0, NULL);
  xRun(tau->sP);
  // Entity 2's still in index 2
  _validateEntityPosition(tau, 2, 2);
}

TEST_F(Tau, xGetNComps) {
  CHECK_EQ(xGetNComps(tau->sP), tau->nEntities);
}

TEST_F(Tau, xMutateComponent) {
  // Request mutation.
  mailboxWrite(tau->sP->mailboxF, 1, 5, MUTATE, 7, NULL);  // will this work w/ only 5 mutations?
  XAComp *cP = (XAComp*) xGetCompPByEntity(tau->sP, 5);
  validateUnprocessedUnmutated( 5, cP );
  // Mutate it.
  xRun(tau->sP);
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
    xQueuePause(tau->sP, voidCP);
  }
  // Pause them.
  xRun(tau->sP);
  // Check that they're paused.
  for (int i = 1; i <= 3; ++i) {
    _validateEntityPosition(tau, i * 10 + 1, i - 1);
    // Make sure it's paused in the fray.
    CHECK_TRUE(_frayElemIsPaused(tau->sP->cF, i - 1));
  }
}

TEST_F(Tau, xQueueDeactivate) {
  // Queue entities 11, 21, and 31 for deactivating.
  for (int i = 1; i <= 3; ++i) {
    void *voidCP = (void*) &((XAComp*) tau->sP->cF)[i * 10];
    xQueueDeactivate(tau->sP, voidCP);
  }
  // Deactivate them.
  xRun(tau->sP);
  // Check that they're deactivated.
  for (int i = 1; i <= 3; ++i) {
    _validateEntityPosition(tau, i * 10 + 1, i * 10);
    // Make sure it's deactivated in the fray.
    CHECK_TRUE(!_frayElemIsActive(tau->sP->cF, i * 10));
  }
}

TEST_F(Tau, ActivateAcrossPause) {
  // First let's set up a 3-activated and 3-paused situation before we run our test.
  // Activate 3.
  for (Entity entity = 1; entity <= 3; ++entity) {
    mailboxWrite(tau->sP->mailboxF, 1, entity, ACTIVATE, 0, NULL);
  }
  // Pause 3.
  for (Entity entity = 4; entity <= 6; ++entity) {
    mailboxWrite(tau->sP->mailboxF, 1, entity, PAUSE, 0, NULL);
  }
  // Process requests.
  xRun(tau->sP);
  // Now activate 2 more across the pauses.
  for (Entity entity = 7; entity <= 8; ++entity) {
    mailboxWrite(tau->sP->mailboxF, 1, entity, ACTIVATE, 0, NULL);
  }
  // Process requests.
  xRun(tau->sP);
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
  mailboxWrite(tau->sP->mailboxF, 1, 1, 20, 0, NULL);  // 20 is the command to make our default procMsg() run.
  xRun(tau->sP);
}
