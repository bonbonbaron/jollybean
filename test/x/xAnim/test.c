#include "tau/tau.h"
#include "xAnim.h"

// We have to make these constants so the compiler doesn't cry about the array initializers not having constant sizes.
#define N_ENTITIES (100)
#define N_MUTATIONS_PER_ENTITY (5)
#define requireSuccess_ REQUIRE_EQ(tau->e, SUCCESS)
#define forEachEntity_(nEntities_) for (Entity entity = 1; entity <= nEntities_; ++entity)

TAU_MAIN()

typedef struct Tau {
  Error e;
  XAnim *xP;
  U32 nEntities;
  U32 nFramesPerEntity;
  U32 nMutationsPerEntity;
  Map *shareMPMP;
  System *sP;
  XAnimComp *animCompF;
} Tau;

TEST_F_SETUP(Tau) {
  extern XAnim xAnim;
  tau->xP = &xAnim;
  tau->sP = &tau->xP->system;
  tau->nEntities = N_ENTITIES;
  tau->nMutationsPerEntity = N_MUTATIONS_PER_ENTITY;
  tau->nFramesPerEntity = 5;
  tau->e = xIniSys(tau->sP, tau->nEntities, NULL);
  tau->animCompF = tau->sP->cF;
  requireSuccess_;
  REQUIRE_EQ(tau->sP->compSz, sizeof(XAnimComp));
  REQUIRE_EQ(xGetNComps(tau->sP), tau->nEntities);

  // Populate all the entities' subcomponents.
  for (Entity entity = 1; entity <= tau->nEntities; ++entity) {
    // ************ MUTATIONS **************
    // Make a mutation map for the current entity.
    Map *currEntitysMutationMP = NULL;
    tau->e = mapNew(&currEntitysMutationMP, RAW_DATA, sizeof(XAnimMutation), tau->nMutationsPerEntity);
    requireSuccess_;
    // Pre-populate the mutation maps with arbitrary values.
    for (Key i = 1; !tau->e && i <= tau->nMutationsPerEntity; ++i) {
      AnimFrame *currAnimFrameA;
      tau->e = arrayNew((void**) &currAnimFrameA, sizeof(AnimFrame), tau->nFramesPerEntity);
      // populate the animation frames for this strip
      for (int j = 0; j < tau->nFramesPerEntity; ++j) {
        AnimFrame currFrame = {
          .rect = {
            .x = (entity + j) * 123 & 0x1fff,
            .y = (entity + j) * 234 & 0xf2ff,
            .w = (entity + j) * 345 & 0xff4f,
            .h = (entity + j) * 456 & 0xfff8
          },
          . duration = (j * 2)
        };
        currAnimFrameA[j] = currFrame;
      }
      requireSuccess_;
      AnimStrip currAnimStrip = {
        .nFrames = tau->nFramesPerEntity,
        .flags = 0,
        .repeat = ((i & 3) == 3),
        .pingPong = i & 1, 
        .frameA = currAnimFrameA
      };
      tau->e = mapSet(currEntitysMutationMP, i, &currAnimStrip);
      requireSuccess_;
    }
    // Now that you've populated this entity's mutation map, add it to the system.
    tau->e = xAddMutationMap(&tau->xP->system, entity, currEntitysMutationMP);
    requireSuccess_;

    // ************ SHARES **************
    // TODO make a share map for *intP here.
    tau->e = mapNew(&tau->shareMPMP, MAP_POINTER, sizeof(Map*), 1);
    requireSuccess_;
    // Make the share inner map. This maps entities to actual, raw data.
    Map *sharedSrcRectMP = NULL;  // Make the share inner map. Maps entities to shared rectangles.
    Map *sharedDstRectMP = NULL;  // Make the share inner map. Maps entities to shared rectangles.
    tau->e = mapNew(&sharedSrcRectMP, RAW_DATA, sizeof(Rect_), tau->nEntities);
    requireSuccess_;
    tau->e = mapNew(&sharedDstRectMP, RAW_DATA, sizeof(Rect_), tau->nEntities);
    requireSuccess_;
    // Now populate the entities' shared rectangles.
    forEachEntity_(tau->nEntities) {
      Rect_ currSrcRect = {
        .x = entity + 0,
        .y = entity + 1,
        .w = entity + 2,
        .h = entity + 3
      };
      tau->e = mapSet(sharedSrcRectMP, entity, &currSrcRect);
      requireSuccess_;
      Rect_ currDstRect = {
        .x = entity + 0,
        .y = entity + 1,
        .w = entity + 2,
        .h = entity + 3
      };
      tau->e = mapSet(sharedDstRectMP, entity, &currDstRect);
      requireSuccess_;
    }
    // Map the inner share map to key value "0" in the outer shared map.
    tau->e = mapSet(tau->shareMPMP, SRC_RECT, &sharedSrcRectMP);
    requireSuccess_;
    tau->e = mapSet(tau->shareMPMP, DST_RECT, &sharedDstRectMP);
    requireSuccess_;
    // Give the shared map to the system. (This particular system wants a pointer to the inner shared map.)
    tau->e = tau->xP->system.getShare(&tau->xP->system, tau->shareMPMP);
    requireSuccess_;

    // Make arrays of the immutable subcomponents. 
    // We're passing in NULL as the component data so the system adds empty components.
    // The postprocess function call hereafter will take care of the mutable velocities and shared rect pointers.
    tau->e = xAddEntityData(&tau->xP->system, entity, ANIMATION, NULL);
    requireSuccess_;
  }
}

TEST_F_TEARDOWN(Tau) {
  xClr(&tau->xP->system);
}

#if 1
TEST_F(Tau, xAnimRun) {
  // Request system to set entity 5's velocity to the one mapped to arbitrary key, 1.
  tau->e = mailboxWrite(tau->sP->mailboxF, ANIMATION, 5, MUTATE_AND_ACTIVATE, 1);
  CHECK_EQ(tau->e, SUCCESS);
  // Request system to set entity 6's velocity to the one mapped to arbitrary key, 2.
  tau->e = mailboxWrite(tau->sP->mailboxF, ANIMATION, 6, MUTATE_AND_ACTIVATE, 2);
  CHECK_EQ(tau->e, SUCCESS);
  // Have key mutate and activate those dudes' motion components and then run.
  tau->e = xRun(tau->sP);
  requireSuccess_;
  // Make sure entity 5's velocity is right.
  CHECK_EQ(tau->animCompF[0].velocity.x,  1);
  CHECK_EQ(tau->animCompF[0].velocity.y, -2);
  CHECK_EQ(tau->animCompF[0].dstRectP->x, 6);
  CHECK_EQ(tau->animCompF[0].dstRectP->y, 4);
  // Make sure entity 6's velocity is right.
  CHECK_EQ(tau->animCompF[1].velocity.x,  2);
  CHECK_EQ(tau->animCompF[1].velocity.y, -4);
  CHECK_EQ(tau->animCompF[1].dstRectP->x, 8);
  CHECK_EQ(tau->animCompF[1].dstRectP->y, 3);
}
#endif
