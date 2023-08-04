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

// We have to make these constants so the compiler doesn't cry about the array initializers not having constant sizes.
#define N_ENTITIES (100)
#define N_MUTATIONS_PER_ENTITY (5)
#define requireSuccess_ REQUIRE_EQ(tau->e, SUCCESS)
#define forEachEntity_(nEntities_) for (Entity entity = 1; entity <= nEntities_; ++entity)

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
        .bb = i * 2,
        .cc = i * 3
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
    aA[entity - 1] = entity * 1;
    dA[entity - 1] = entity * 4.0;
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

//--------------------------------------------
// Actual tests
//--------------------------------------------

TEST_F(Tau, xGetCompPByEntity) {
  forEachEntity_(tau->nEntities) {
    XAComp *cP = (XAComp*) xGetCompPByEntity(tau->sP, entity);
    CHECK_EQ(cP->a, entity * 1);
    CHECK_EQ(cP->b, entity * 0);
    CHECK_EQ(cP->c, entity * 0);
    CHECK_EQ(cP->d, entity * 4.0);
  }
}

TEST_F(Tau, xGetEntityByCompIdx) {
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
    U32 componentIdx = cP - (XAComp*) tau->sP->cF;
    // Make sure the two activated components are moved to the front of the fray AND marked as active.
    switch (componentIdx) {
      case 0:
        CHECK_EQ(cP->a, 9);
        CHECK_EQ(cP->d, 10.0);
        CHECK_TRUE(_frayElemIsActive(tau->sP->cF, componentIdx));
        break;
      case 1:
        CHECK_EQ(cP->a, 49);
        CHECK_EQ(cP->d, 50.0);
        CHECK_TRUE(_frayElemIsActive(tau->sP->cF, componentIdx));
        break;
      // Make sure all the rest are inactive.
      default:
        CHECK_FALSE(_frayElemIsActive(tau->sP->cF, componentIdx));
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
  XAComp *cEndP = cP + *_frayGetFirstEmptyIdxP(tau->sP->cF);
  for (; cP < cEndP; ++cP) {
    U32 componentIdx = cP - (XAComp*) tau->sP->cF;
    // Make sure the two activated components are moved to the front of the fray AND marked as active.
    switch (componentIdx) {
      case 0:
        CHECK_EQ(cP->a, 9);
        CHECK_EQ(cP->b, 9);
        CHECK_EQ(cP->c, 9);
        CHECK_EQ(cP->d, 10.0);
        CHECK_TRUE(_frayElemIsActive(tau->sP->cF, componentIdx));
        break;
      case 1:
        CHECK_EQ(cP->a, 49);
        CHECK_EQ(cP->b, 9);
        CHECK_EQ(cP->c, 9);
        CHECK_EQ(cP->d, 50.0);
        CHECK_TRUE(_frayElemIsActive(tau->sP->cF, componentIdx));
        break;
      // Make sure all the rest are inactive.
      default:
        CHECK_FALSE(_frayElemIsActive(tau->sP->cF, componentIdx));
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
  CHECK_EQ(cP->a, 10);
  CHECK_EQ(cP->d, 40.0);
  // make sure it's also inactive
  CHECK_FALSE(_frayElemIsActive(tau->sP->cF, 0));
}

TEST_F(Tau, xPauseComponentByEntity) {
  // Activate the first 3 entities
  for (Entity entity = 1; entity <= 3; ++entity) {
    tau->e = xActivateComponentByEntity(tau->sP, entity);
    requireSuccess_;
  }
  // Pause the middle one
  tau->e = xPauseComponentByEntity(tau->sP, 2);
  requireSuccess_;
  // Make sure the ordering is correct in the entity-to-component array.
  // Entity 1's still in index 0
  Key *idxP = mapGet(tau->sP->e2cIdxMP, 1);
  CHECK_EQ(*idxP, 0);
  CHECK_EQ(tau->sP->cIdx2eA[*idxP], 1);
  // Entity 2's now in index 2
  idxP = mapGet(tau->sP->e2cIdxMP, 2);
  CHECK_EQ(*idxP, 2);
  CHECK_EQ(tau->sP->cIdx2eA[*idxP], 2);
  // Entity 3's now in index 1
  idxP = mapGet(tau->sP->e2cIdxMP, 3);
  CHECK_EQ(*idxP, 1); 
  CHECK_EQ(tau->sP->cIdx2eA[*idxP], 3);
}

TEST_F(Tau, xUnpauseComponentByEntity) {
  // Repeat the same test as the above so we can unpause the paused element.
  // Activate the first 3 entities
  for (Entity entity = 1; entity <= 3; ++entity) {
    tau->e = xActivateComponentByEntity(tau->sP, entity);
    requireSuccess_;
  }
  // Pause the middle one
  tau->e = xPauseComponentByEntity(tau->sP, 2);
  requireSuccess_;
  // Make sure the ordering is correct in the entity-to-component array.
  // Entity 1's still in index 0
  Key *idxP = mapGet(tau->sP->e2cIdxMP, 1);
  CHECK_EQ(*idxP, 0);
  CHECK_EQ(tau->sP->cIdx2eA[*idxP], 1);
  // Entity 2's now in index 2
  idxP = mapGet(tau->sP->e2cIdxMP, 2);
  CHECK_EQ(*idxP, 2);
  CHECK_EQ(tau->sP->cIdx2eA[*idxP], 2);
  // Entity 3's now in index 1
  idxP = mapGet(tau->sP->e2cIdxMP, 3);
  CHECK_EQ(*idxP, 1); 
  CHECK_EQ(tau->sP->cIdx2eA[*idxP], 3);
  // Unpause
  
}

TEST_F(Tau, xActivateComponentByIdx) {
  // Unlike previous tests, let's see what happens if we pause an element first.
  tau->e = xPauseComponentByEntity(tau->sP, 5);
  requireSuccess_;
}

TEST_F(Tau, xDeactivateComponentByIdx) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xGetNComps) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xMutateComponent) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xGetEntityByVoidComponentPtr) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xQueuePause) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xQueueDeactivate) {
  REQUIRE_TRUE(1);
}

// TODO make tests that perform the above in the inbox

TEST_F(Tau, xRun) {
  REQUIRE_TRUE(1);
}
