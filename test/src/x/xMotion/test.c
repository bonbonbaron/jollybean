#include "tau.h"
#include "xMotion.h"


// We have to make these constants so the compiler doesn't cry about the array initializers not having constant sizes.
#define N_ENTITIES (100)
#define N_MUTATIONS_PER_ENTITY (5)

#define forEachEntity_(nEntities_) for (Entity entity = 1; entity <= nEntities_; ++entity)

TAU_MAIN()

typedef struct Tau {
  XMotion *xP;
  U32 nEntities;
  U32 nMutationsPerEntity;
  Map *shareMPMP;
  System *sP;
  XMotionComp *motionCompF;
} Tau;

TEST_F_SETUP(Tau) {
  extern XMotion xMotion;
  tau->xP = &xMotion;
  tau->sP = &tau->xP->system;
  tau->nEntities = N_ENTITIES;
  tau->nMutationsPerEntity = N_MUTATIONS_PER_ENTITY;
  xIniSys(tau->sP, tau->nEntities, NULL);
  tau->motionCompF = tau->sP->cF;

  REQUIRE_EQ(tau->sP->compSz, sizeof(XMotionComp));
  REQUIRE_EQ(xGetNComps(tau->sP), tau->nEntities);

  tau->shareMPMP = mapNew( MAP_POINTER, sizeof(Map*), 1);
  // Map the inner share map to key value "MOTION" in the outer shared map.
  // ************ SHARES **************
  // Make the share inner map. This maps entities to actual, raw data.
  Map* shareMP = mapNew(RAW_DATA, sizeof(Rect_), tau->nEntities);
  mapSet(tau->shareMPMP, MOTION, &shareMP);

  // Populate all the entities' subcomponents.
  for (Entity entity = 1; entity <= tau->nEntities; ++entity) {
    // ************ MUTATIONS **************
    // Make a mutation map for the current entity.
    Map *currEntitysMutationMP = NULL;
    currEntitysMutationMP = mapNew(RAW_DATA, sizeof(XMotionMutation), tau->nMutationsPerEntity);

    // Pre-populate the mutation maps with arbitrary values.
    for (Key i = 1; i <= tau->nMutationsPerEntity; ++i) {
      Velocity currVelocity = {
        .x = i *  1,
        .y = i * -2
      };
      mapSet(currEntitysMutationMP, i, &currVelocity);
    }

    // Now that you've populated this entity's mutation map, add it to the system.
    xAddMutationMap(&tau->xP->system, entity, currEntitysMutationMP);

    // Now populate the entities' shared rectangles.
    Rect_ currRect = {
      .x = entity + 0,
      .y = entity + 1,
      .w = entity + 2,
      .h = entity + 3
    };
    mapSet(shareMP, entity, &currRect);
    // Make arrays of the immutable subcomponents. 
    // We're passing in NULL as the component data so the system adds empty components.
    // The postprocess function call hereafter will take care of the mutable velocities and shared rect pointers.
    xAddEntityData(&tau->xP->system, entity, MOTION, NULL);
  }

  // Give the shared map to the system. (This particular system wants a pointer to the inner shared map.)
  tau->xP->system.getShare(&tau->xP->system, tau->shareMPMP);

  // Make the system make full components from all the subcomponents.
  tau->xP->system.postprocessComps(&tau->xP->system);
}

TEST_F_TEARDOWN(Tau) {
  xClr(&tau->xP->system);
}

TEST_F(Tau, xMotionRun) {
  // Request system to set entity 5's velocity to the one mapped to arbitrary key, 1.
  mailboxWrite(tau->sP->mailboxF, MOTION, 5, MUTATE_AND_ACTIVATE, 1, NULL);
  // Request system to set entity 6's velocity to the one mapped to arbitrary key, 2.
  mailboxWrite(tau->sP->mailboxF, MOTION, 6, MUTATE_AND_ACTIVATE, 2, NULL);
  // Have key mutate and activate those dudes' motion components and then run.
  xRun(tau->sP);
  // Make sure entity 5's velocity is right.
  CHECK_EQ(tau->motionCompF[0].velocity.x,  1);
  CHECK_EQ(tau->motionCompF[0].velocity.y, -2);
  CHECK_EQ(tau->motionCompF[0].dstRectP->x, 6);
  CHECK_EQ(tau->motionCompF[0].dstRectP->y, 4);
  // Make sure entity 6's velocity is right.
  CHECK_EQ(tau->motionCompF[1].velocity.x,  2);
  CHECK_EQ(tau->motionCompF[1].velocity.y, -4);
  CHECK_EQ(tau->motionCompF[1].dstRectP->x, 8);
  CHECK_EQ(tau->motionCompF[1].dstRectP->y, 3);
}
