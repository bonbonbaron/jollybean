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

# define requireSuccess_ REQUIRE_EQ(tau->e, SUCCESS)
typedef struct Tau {
  Error e;
  U32 nEntities;
  U32 nMutationsPerEntity;
  XA *xP;
  Map *bMP;  // map of "b" mutations
  Map *cMP;  // map of "c" mutations
} Tau;

// We have to make these constants so the compiler doesn't cry about the array initializers not having constant sizes.
#define N_ENTITIES (100)
#define N_MUTATIONS_PER_ENTITY (5)
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
        .bb = i * 3,
        .cc = i + 4
      };
      tau->e = mapSet(currEntitysMutationMP, i, &currMutation);
      requireSuccess_;
    }
    // Now that you've populated this entity's mutation map, add it to the system.
    // Error xAddMutationMap(System *sP, Entity entity, Map *mutationMP) {
    tau->e = xAddMutationMap(&tau->xP->system, entity, currEntitysMutationMP);
    requireSuccess_;

    // ************ SHARES **************
    // TODO make a share map for *intP here.
    Map *shareMPMP = NULL;
    Map *shareMP = NULL;
    // Make the share outer map. This maps datatypes to inner maps. We'll only store one map in there.
    tau->e = mapNew(&shareMPMP, MAP_POINTER, sizeof(Map*), 1);
    requireSuccess_;
    // Make the share inner map. This maps entities to actual, raw data.
    tau->e = mapNew(&shareMP, RAW_DATA, sizeof(Map*), tau->nMutationsPerEntity);
    requireSuccess_;
    // Map the inner share map to key value "1" in the outer shared map.
    tau->e = mapSet(shareMPMP, 1, &shareMP);
    // Give the shared map to the system. (This particular system wants a pointer to the inner shared map.)
    tau->e = tau->xP->system.getShare(&tau->xP->system, shareMPMP);

    // Make arrays of the immutable subcomponents. 
    // The subcomponent intializers consume only pointers to the original data,
    // so we have to store them in arrays here.
    aA[entity] = entity * 1;
    dA[entity] = entity * 4.0;
    tau->e = xAddEntityData(&tau->xP->system, entity, 0x40 | XA_TYPE, &aA[entity]);
    requireSuccess_;
    tau->e = xAddEntityData(&tau->xP->system, entity, 0x80 | XA_TYPE, &dA[entity]);
    requireSuccess_;
  }
  // Make the system make full components from all the subcomponents.
  tau->e = tau->xP->system.postprocessComps(&tau->xP->system);
  requireSuccess_;
}

TEST_F_TEARDOWN(Tau) {
  xClr(&tau->xP->system);
}
//--------------------------------------------

TEST_F(Tau, xGetCompPByEntity) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xGetEntityByCompIdx) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xActivateComponentByEntity) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xDeactivateComponentByEntity) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xPauseComponentByEntity) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xUnpauseComponentByEntity) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, xActivateComponentByIdx) {
  REQUIRE_TRUE(1);
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

TEST_F(Tau, xRun) {
  REQUIRE_TRUE(1);
}
