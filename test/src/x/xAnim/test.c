#include "tau/tau.h"
#include "xAnim.h"

// We have to make these constants so the compiler doesn't cry about the array initializers not having constant sizes.
#define N_ENTITIES (100)
#define N_MUTATIONS_PER_ENTITY (5)
#define N_FRAMES_PER_STRIP (5)
#define requireSuccess_ REQUIRE_EQ(tau->e, SUCCESS)
#define forEachEntity_(nEntities_) for (Entity entity = 1; entity <= nEntities_; ++entity)

#define RECT_X_COEFF (10)
#define RECT_Y_COEFF (20)
#define RECT_W_COEFF (30)
#define RECT_H_COEFF (40)
#define SRC_RECT_X (1)
#define SRC_RECT_Y (2)
#define SRC_RECT_W (3)
#define SRC_RECT_H (4)
#define DST_RECT_X (5)
#define DST_RECT_Y (6)
#define DST_RECT_W (7)
#define DST_RECT_H (8)
#define OFFSET_X (5)
#define OFFSET_Y (10)

#define REPEAT_FLAG  (0x01)
#define PINGPONG_LIM (0x03)
#define DURATION (10)

TAU_MAIN()

typedef struct Tau {
  Error e;
  XAnim *xP;
  U32 nEntities;
  U32 nFramesPerStrip;
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
  tau->nFramesPerStrip = N_FRAMES_PER_STRIP;
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
      tau->e = arrayNew((void**) &currAnimFrameA, sizeof(AnimFrame), tau->nFramesPerStrip);
      // populate the animation frames for this strip
      for (int j = 0; j < tau->nFramesPerStrip; ++j) {
        AnimFrame currFrame = {
          .rect = {  
            .x = j * RECT_X_COEFF,  // 10  ( 0 for first frame )
            .y = j * RECT_Y_COEFF,  // 20  ( 0 for first frame )
            .w = entity * RECT_W_COEFF,  // 30 
            .h = entity * RECT_H_COEFF   // 40 
          },
          . duration = DURATION  
        };
        currAnimFrameA[j] = currFrame;
      }
      requireSuccess_;
      AnimStrip currAnimStrip = {
        .nFrames = tau->nFramesPerStrip,  // fixed number of frames
        .flags = 0,   // flags are only used to tell when the rect's been offset already
        .repeat = i & REPEAT_FLAG,  // odd-numbered 
        .pingPong = i < PINGPONG_LIM,   // pingpong for the first two strips
        .frameA = currAnimFrameA
      };
      tau->e = mapSet(currEntitysMutationMP, i, &currAnimStrip);
      requireSuccess_;
    }
    // Now that you've populated this entity's mutation map, add it to the system.
    tau->e = xAddMutationMap(&tau->xP->system, entity, currEntitysMutationMP);
    requireSuccess_;

    // ************ SHARES **************
    // TODO make a share map for *intP here.  // What was this for?
    tau->e = mapNew(&tau->shareMPMP, MAP_POINTER, sizeof(Map*), 3);
    requireSuccess_;
    // Make the share inner map. This maps entities to actual, raw data.
    Map *sharedSrcRectMP = NULL;  // Make the share inner map. Maps entities to shared rectangles.
    Map *sharedDstRectMP = NULL;  // Make the share inner map. Maps entities to shared rectangles.
    Map *sharedOffsetMP = NULL;  // Make the share inner map. Maps entities to shared rectangles.
    tau->e = mapNew(&sharedSrcRectMP, RAW_DATA, sizeof(Rect_), tau->nEntities);
    requireSuccess_;
    tau->e = mapNew(&sharedDstRectMP, RAW_DATA, sizeof(Rect_), tau->nEntities);
    requireSuccess_;
    tau->e = mapNew(&sharedOffsetMP, RAW_DATA, sizeof(RectOffset), tau->nEntities);
    requireSuccess_;
    // Now populate the entities' shared rectangles.
    // TODO I don't think this code is well-founded. You don't know what your source rect is until you mutate.
    forEachEntity_(tau->nEntities) {
      Rect_ currSrcRect = { 0 };  // the post-mutate function will change this value before animating
      tau->e = mapSet(sharedSrcRectMP, entity, &currSrcRect);
      requireSuccess_;
      Rect_ currDstRect = { 0 };  // the post-mutate function will change this value before animating
      tau->e = mapSet(sharedDstRectMP, entity, &currDstRect);
      requireSuccess_;
      RectOffset currRectOffset = {
        .x = OFFSET_X,  //  5
        .y = OFFSET_Y   // 10
      };
      tau->e = mapSet(sharedOffsetMP, entity, &currRectOffset);
      requireSuccess_;
    }
    // Map the inner share map to key value "0" in the outer shared map.
    tau->e = mapSet(tau->shareMPMP, SRC_RECT, &sharedSrcRectMP);
    requireSuccess_;
    tau->e = mapSet(tau->shareMPMP, DST_RECT, &sharedDstRectMP);
    requireSuccess_;
    tau->e = mapSet(tau->shareMPMP, RECT_OFFSET, &sharedOffsetMP);
    requireSuccess_;
    // Give the shared map to the system. (This particular system wants a pointer to the inner shared map.)
    tau->e = tau->xP->system.getShare(&tau->xP->system, tau->shareMPMP);
    requireSuccess_;

    // Make arrays of the immutable subcomponents. 
    // We're passing in NULL as the component data so the system adds empty components.
    // The postprocess function call hereafter will take care of the mutable velocities and shared rect pointers.
    tau->e = xAddEntityData(&tau->xP->system, entity, ANIMATION, NULL);
    // Finally, tell the animation system to update all its rects with their offsets.
    for ( Entity entity = 1; entity < tau->nEntities; ++entity ) {
      tau->e = mailboxWrite( tau->sP->mailboxF, ANIMATION, entity, UPDATE_RECT, 0 );
      requireSuccess_;
    }
    requireSuccess_;
  }
}

TEST_F_TEARDOWN(Tau) {
  xClr(&tau->xP->system);
}

void checkAnimComp( Tau *tau, Entity entity, U32 currDuration, U32 currFrameIdx, Bln isOffset, Bln isGoingBackwards ) {
  XAnimComp* cP = (XAnimComp*) xGetCompPByEntity(tau->sP, entity);
  // If source rect is all zeros, we haven't begun any animation for this entity yet.
  if ( cP->srcRectP->w == 0 ) {
    printf("checking fresh stuff\n");
    CHECK_EQ( cP->srcRectP->x, 0);
    CHECK_EQ( cP->srcRectP->y, 0);
    CHECK_EQ( cP->srcRectP->w, 0);
    CHECK_EQ( cP->srcRectP->h, 0);
    CHECK_EQ( cP->dstRectP->x, 0);
    CHECK_EQ( cP->dstRectP->y, 0);
    CHECK_EQ( cP->dstRectP->w, 0);
    CHECK_EQ( cP->dstRectP->h, 0);
    // Check non-rect stuff too.
    CHECK_EQ( cP->incrDecrement, 0);
    CHECK_EQ( cP->currFrameIdx, 0);
    CHECK_EQ( cP->incrDecrement, 0);
    CHECK_EQ( cP->timeLeft, 0);
    CHECK_EQ( cP->currStrip.nFrames, 0 );
  }
  // Otherwise, if we've started animating for this entity, we expect a post-mutation of the src/dstRect.
  else {
    printf("checking changed stuff\n");
    CHECK_EQ( cP->srcRectP->x, RECT_X_COEFF * ( cP->currFrameIdx ) + OFFSET_X );
    CHECK_EQ( cP->srcRectP->y, RECT_Y_COEFF * ( cP->currFrameIdx ) + OFFSET_Y );
    CHECK_EQ( cP->srcRectP->w, entity * RECT_W_COEFF );
    CHECK_EQ( cP->srcRectP->h, entity * RECT_H_COEFF );
    CHECK_EQ( cP->dstRectP->x, entity + DST_RECT_X);
    CHECK_EQ( cP->dstRectP->y, entity + DST_RECT_Y);
    CHECK_EQ( cP->dstRectP->w, entity * RECT_W_COEFF );
    CHECK_EQ( cP->dstRectP->h, entity * RECT_H_COEFF );
    // Check non-rect stuff too.
    CHECK_EQ( cP->incrDecrement, isGoingBackwards ? -1 : 1 );
    CHECK_EQ( cP->currFrameIdx, currFrameIdx);
    CHECK_EQ( cP->timeLeft, currDuration );
    CHECK_EQ( cP->currStrip.nFrames, tau->nFramesPerStrip );
  }
}

void advanceOneFrame( Tau* tau ) {
  for (int i = 0; i < tau->nFramesPerStrip; ++i) {
    tau->e = xRun(tau->sP);
    requireSuccess_;
  }
}

TEST_F(Tau, xAnimMutate) {
  // First, make sure entity 6's component still has its original values.
  checkAnimComp( tau, 6, 0, 0, FALSE, FALSE );
  // Request system to set entity 5's animation to the one mapped to arbitrary key, 1.
  tau->e = mailboxWrite(tau->sP->mailboxF, ANIMATION, 6, MUTATE_AND_ACTIVATE, 1);
  // TODO this may be a bug: I don't see the offset happening in the test results.
  CHECK_EQ(tau->e, SUCCESS);
  tau->e = xRun(tau->sP);
  CHECK_EQ(tau->e, SUCCESS);
  checkAnimComp( tau, 6, DURATION - 1, 1, TRUE, FALSE );
}

