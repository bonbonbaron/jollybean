#include "tau.h"
#include "xAnim.h"

// We have to make these constants so the compiler doesn't cry about the array initializers not having constant sizes.
#define N_ENTITIES (100)
#define N_MUTATIONS_PER_ENTITY (5)
#define N_FRAMES_PER_STRIP (5)

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

#define GOING_FORWARD FALSE
#define GOING_BACKWARD TRUE
#define IS_ANIMATED TRUE
#define NOT_ANIMATED FALSE
TAU_MAIN()

typedef struct Tau {
  XAnim *xP;
  U32 nEntities;
  U32 nFramesPerStrip;
  U32 nMutationsPerEntity;
  Map *shareMPMP;
  System *sP;
  XAnimComp *animCompF;
  AnimFrame** frameAF;
} Tau;

TEST_F_SETUP(Tau) {
  extern XAnim xAnim;
  tau->xP = &xAnim;
  tau->sP = &tau->xP->system;
  tau->nEntities = N_ENTITIES;
  tau->nMutationsPerEntity = N_MUTATIONS_PER_ENTITY;
  tau->nFramesPerStrip = N_FRAMES_PER_STRIP;
  xIniSys(tau->sP, tau->nEntities, NULL);
  tau->animCompF = tau->sP->cF;
  tau->frameAF = frayNew( sizeof( AnimFrame* ), tau->nEntities * tau->nMutationsPerEntity );

  REQUIRE_EQ(tau->sP->compSz, sizeof(XAnimComp));
  REQUIRE_EQ(xGetNComps(tau->sP), tau->nEntities);

  // Populate all the entities' subcomponents.
  forEachEntity_( tau->nEntities ) {
    // ************ MUTATIONS **************
    // Make a mutation map for the current entity.
    Map *currEntitysMutationMP =  mapNew(RAW_DATA, sizeof(XAnimMutation), tau->nMutationsPerEntity);

    // Pre-populate the mutation maps with arbitrary values.
    for (Key i = 1; i <= tau->nMutationsPerEntity; ++i) {
      AnimFrame *currAnimFrameA = arrayNew(sizeof(AnimFrame), tau->nFramesPerStrip);
      frayAdd( tau->frameAF, &currAnimFrameA, NULL );

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
      AnimStrip currAnimStrip = {
        .nFrames = tau->nFramesPerStrip,  // fixed number of frames
        .flags = 0,   // flags are only used to tell when the rect's been offset already
        .repeat = i & REPEAT_FLAG,  // odd-numbered strips do repeats
        .pingPong = i < PINGPONG_LIM,   // strips numbered less than 3 do pingpong
        .frameA = currAnimFrameA
      };
      // Stick the animation strip into the current entity's map of animation strips.
      mapSet(currEntitysMutationMP, i, &currAnimStrip);

    }
    // Now that you've populated this entity's mutation map, add it to the system.
    xAddMutationMap(&tau->xP->system, entity, currEntitysMutationMP);

  }

  // ************ SHARES **************
  tau->shareMPMP = mapNew(MAP_POINTER, sizeof(Map*), 3);

  // Make the share inner map. This maps entities to actual, raw data.
  Map* sharedSrcRectMP = mapNew(RAW_DATA, sizeof(Rect_), tau->nEntities);
  Map* sharedDstRectMP = mapNew(RAW_DATA, sizeof(Rect_), tau->nEntities);
  Map* sharedOffsetMP = mapNew(RAW_DATA, sizeof(RectOffset), tau->nEntities);

  // Now populate the entities' shared rectangles.
  // You don't know what your source rect is until you mutate.
  forEachEntity_( tau->nEntities ) {
    Rect_ currSrcRect = { 0 };  // the post-mutate function will change this value before animating
    mapSet(sharedSrcRectMP, entity, &currSrcRect);

    Rect_ currDstRect = { 0 };  // the post-mutate function will change this value before animating
    mapSet(sharedDstRectMP, entity, &currDstRect);

    RectOffset currRectOffset = {
      .x = OFFSET_X,  //  5
      .y = OFFSET_Y   // 10
    };
    mapSet(sharedOffsetMP, entity, &currRectOffset);

  }
  // If ordering is sacred, then we need fixed functions that'll encapsulate this sacred ordering for us.
  // Now add the entity to the actual system itself.
  forEachEntity_( tau->nEntities ) {
    xAddEntityData(&tau->xP->system, entity, ANIMATION, NULL);

  }
  // Map the inner share map to key value "0" in the outer shared map.
  mapSet(tau->shareMPMP, SRC_RECT, &sharedSrcRectMP);

  mapSet(tau->shareMPMP, DST_RECT, &sharedDstRectMP);

  mapSet(tau->shareMPMP, RECT_OFFSET, &sharedOffsetMP);

  // Give the shared map to the system. (This particular system wants a pointer to the inner shared map.)
  tau->xP->system.getShare(&tau->xP->system, tau->shareMPMP);

  // Make arrays of the immutable subcomponents. 
  // We're passing in NULL as the component data so the system adds empty components.
  // The postprocess function call hereafter will take care of the mutable velocities and shared rect pointers.
  // Finally, tell the animation system to update all its rects with their offsets.
  forEachEntity_( tau->nEntities ) {
    mailboxWrite( tau->sP->mailboxF, ANIMATION, entity, UPDATE_RECT, 0, NULL );

  }
  // Go ahead and run the system to flush out the rectangle-update messages. 
  // We need to leave room for the test cases' mailbox writes.
  xRun( tau->sP );

  

  // PREPARE TEST SAMPLES
  // *********************************
  // Entity 6 strip 1: repeat + pingpong
#define ENTITY_WITH_PINGPONG_REPEAT (6)
  mailboxWrite(tau->sP->mailboxF, ANIMATION, ENTITY_WITH_PINGPONG_REPEAT, MUTATE_AND_ACTIVATE, 1, NULL);

  // *********************************
  // Entity 1 strip 2: pingpong only
#define ENTITY_WITH_PINGPONG (1)
  mailboxWrite(tau->sP->mailboxF, ANIMATION, ENTITY_WITH_PINGPONG, MUTATE_AND_ACTIVATE, 2, NULL);

  // *********************************
  // Entity 88 strip 3: repeat only
#define ENTITY_WITH_REPEAT (88)  // TODO change back to 88 after you find and fix bug
  mailboxWrite(tau->sP->mailboxF, ANIMATION, ENTITY_WITH_REPEAT, MUTATE_AND_ACTIVATE, 3, NULL);

  // *********************************
  // Entity 30 strip 4: neither repeat nor pingpong
#define ENTITY_WITH_NOTHING (30)
  mailboxWrite(tau->sP->mailboxF, ANIMATION, ENTITY_WITH_NOTHING, MUTATE_AND_ACTIVATE, 4, NULL);


  // Don't run system here. Instead, let each test case decide whether to advance by timestep, frame, or full strip.
}

TEST_F_TEARDOWN(Tau) {
  xClr(&tau->xP->system);
  for ( int i = 0; i < arrayGetNElems( tau->frameAF ); ++i ) {
    arrayDel( (void**) &tau->frameAF[i] );
  }
  frayDel( (void**) &tau->frameAF );
  mapOfNestedMapsDel( &tau->shareMPMP );
}

void checkAnimComp( Tau *tau, Entity entity, U32 currDuration, U32 expCurrFrameIdx, Bln isGoingBackwards ) {
  XAnimComp* cP = (XAnimComp*) xGetCompPByEntity(tau->sP, entity);
  if ( !cP ) {
    REQUIRE_EQ( 1, 0 );  // This means the entity doesn't have a component in this system.
  }
  // If source rect is all zeros, we haven't begun any animation for this entity yet.
  if ( cP->incrDecrement == 0) {  // if this is 0, that implies we haven't mutated the anim comp to anything yet.
    CHECK_TRUE( cP->srcRectP != NULL );  // despite no mutation, getShare() should've populated shared rect pointers
    CHECK_TRUE( cP->dstRectP != NULL );  // despite no mutation, getShare() should've populated shared rect pointers
    CHECK_EQ( cP->incrDecrement, 0);
    CHECK_EQ( cP->currFrameIdx, 0);
    CHECK_EQ( cP->incrDecrement, 0);
    CHECK_EQ( cP->timeLeft, 0);
    CHECK_EQ( cP->currStrip.nFrames, 0 );
  }
  // Otherwise, if we've started animating for this entity, we expect a post-mutation of the src/dstRect.
  else {
    REQUIRE_TRUE( cP != NULL );
    REQUIRE_TRUE( cP->srcRectP != NULL );
    REQUIRE_TRUE( cP->dstRectP != NULL );
    CHECK_EQ( cP->srcRectP->x, RECT_X_COEFF * ( cP->currFrameIdx ) + OFFSET_X );
    CHECK_EQ( cP->srcRectP->y, RECT_Y_COEFF * ( cP->currFrameIdx ) + OFFSET_Y );
    CHECK_EQ( cP->srcRectP->w, entity * RECT_W_COEFF );
    CHECK_EQ( cP->srcRectP->h, entity * RECT_H_COEFF );
    CHECK_EQ( cP->dstRectP->x, 0 );
    CHECK_EQ( cP->dstRectP->y, 0 );
    CHECK_EQ( cP->dstRectP->w, cP->srcRectP->w );
    CHECK_EQ( cP->dstRectP->h, cP->srcRectP->h );
    // Check non-rect stuff too.
    CHECK_EQ( cP->incrDecrement, isGoingBackwards ? -1 : 1 );
    CHECK_EQ( cP->currFrameIdx, expCurrFrameIdx);
    CHECK_EQ( cP->timeLeft, currDuration );
    CHECK_EQ( cP->currStrip.nFrames, tau->nFramesPerStrip );
  }
}

void advanceOneTimestep( Tau* tau ) {
  xRun(tau->sP);
}

void advanceOneFrame( Tau* tau ) {
  for (int i = 0; i < DURATION; ++i) {
    advanceOneTimestep( tau );
  }
}

void advanceOneStrip( Tau* tau ) {
  for (int i = 0; i < tau->nFramesPerStrip; ++i) {
    advanceOneFrame( tau );
  }
}

void checkIfEntityIsAnimated( Tau* tau, Entity entity, Bln expectAnimated ) {
  REQUIRE_TRUE ( tau->sP != NULL );
  XAnimComp* cP = (XAnimComp*) xGetCompPByEntity( tau->sP, entity );
  REQUIRE_TRUE ( cP != NULL );
  U32 compIdx = cP - (XAnimComp*) tau->sP->cF;
  CHECK_EQ( _frayElemIsActive(tau->sP->cF, compIdx ), expectAnimated );
}

// Test cases for repeating ping pong
TEST_F(Tau, xAnim_repeatingPingpongStrip_incrementOneTimestep) {
  advanceOneTimestep( tau );
  checkAnimComp( tau, ENTITY_WITH_PINGPONG_REPEAT, DURATION - 1, 0, GOING_FORWARD );
}

TEST_F(Tau, xAnim_repeatingPingpongStrip_incrementOneFullFrame) {
  advanceOneFrame( tau );
  checkAnimComp( tau, ENTITY_WITH_PINGPONG_REPEAT, DURATION, 1, GOING_FORWARD );
}

TEST_F(Tau, xAnim_repeatingPingpongStrip_incrementOneFullStrip) {
  advanceOneStrip( tau );
  checkAnimComp( tau, ENTITY_WITH_PINGPONG_REPEAT, DURATION, 3, GOING_BACKWARD );
}

TEST_F(Tau, xAnim_repeatingPingpongStrip_fullPingPong) {
  advanceOneStrip( tau );
  checkIfEntityIsAnimated( tau, ENTITY_WITH_PINGPONG_REPEAT, IS_ANIMATED );
  checkAnimComp( tau, ENTITY_WITH_PINGPONG_REPEAT, DURATION, 3, GOING_BACKWARD );
  advanceOneStrip( tau );
  checkAnimComp( tau, ENTITY_WITH_PINGPONG_REPEAT, DURATION, 2, GOING_FORWARD );
}

// Test cases for pingpong
TEST_F(Tau, xAnim_pingPongStrip_incrementOneTimestep) {
  advanceOneTimestep( tau );
  checkAnimComp( tau, ENTITY_WITH_PINGPONG, DURATION - 1, 0, GOING_FORWARD );
}

TEST_F(Tau, xAnim_pingPongStrip_incrementOneFullFrame) {
  checkAnimComp( tau, ENTITY_WITH_PINGPONG, 0, 0, GOING_FORWARD );
  mailboxWrite(tau->sP->mailboxF, ANIMATION, ENTITY_WITH_PINGPONG, MUTATE_AND_ACTIVATE, 1, NULL);
  advanceOneFrame( tau );
  checkAnimComp( tau, ENTITY_WITH_PINGPONG, DURATION, 1, GOING_FORWARD );
}

TEST_F(Tau, xAnim_pingPongStrip_incrementOneFullStrip) {
  advanceOneStrip( tau );
  checkIfEntityIsAnimated( tau, ENTITY_WITH_PINGPONG, IS_ANIMATED );
  checkAnimComp( tau, ENTITY_WITH_PINGPONG, DURATION, 3, GOING_BACKWARD );
}

TEST_F(Tau, xAnim_pingPongStrip_fullPingPong) {
  advanceOneStrip( tau );
  checkIfEntityIsAnimated( tau, ENTITY_WITH_PINGPONG, IS_ANIMATED );
  checkAnimComp( tau, ENTITY_WITH_PINGPONG, DURATION, 3, GOING_BACKWARD );
  advanceOneStrip( tau );
  checkAnimComp( tau, ENTITY_WITH_PINGPONG, 0, 0, GOING_FORWARD );
}

// Test cases for repeating
TEST_F(Tau, xAnim_repeatingStrip_incrementOneTimestep) {
  advanceOneTimestep( tau );
  checkAnimComp( tau, ENTITY_WITH_REPEAT, DURATION - 1, 0, GOING_FORWARD );
}

TEST_F(Tau, xAnim_repeatingStrip_incrementOneFullFrame) {
  advanceOneFrame( tau );
  checkAnimComp( tau, ENTITY_WITH_REPEAT, DURATION, 1, GOING_FORWARD );
}

TEST_F(Tau, xAnim_repeatingStrip_incrementOneFullStrip) {
  advanceOneStrip( tau );
  checkIfEntityIsAnimated( tau, ENTITY_WITH_REPEAT, IS_ANIMATED );
  checkAnimComp( tau, ENTITY_WITH_REPEAT, DURATION, 0, GOING_FORWARD );
}

TEST_F(Tau, xAnim_repeatingStrip_fullPingPong) {
  advanceOneStrip( tau );
  checkIfEntityIsAnimated( tau, ENTITY_WITH_REPEAT, IS_ANIMATED );
  checkAnimComp( tau, ENTITY_WITH_REPEAT, DURATION, 0, GOING_FORWARD );
  advanceOneStrip( tau );
  checkIfEntityIsAnimated( tau, ENTITY_WITH_REPEAT, IS_ANIMATED );
  checkAnimComp( tau, ENTITY_WITH_REPEAT, DURATION, 0, GOING_FORWARD );
}

// Test cases for neither pingpong nor repeating
TEST_F(Tau, xAnim_vanillaStrip_incrementOneTimestep) {
  advanceOneTimestep( tau );
  checkAnimComp( tau, ENTITY_WITH_NOTHING, DURATION - 1, 0, GOING_FORWARD );
}

TEST_F(Tau, xAnim_vanillaStrip_incrementOneFullFrame) {
  advanceOneFrame( tau );
  checkAnimComp( tau, ENTITY_WITH_NOTHING, DURATION, 1, GOING_FORWARD );
}

TEST_F(Tau, xAnim_vanillaStrip_incrementOneFullStrip) {
  advanceOneStrip( tau );
  checkIfEntityIsAnimated( tau, ENTITY_WITH_NOTHING, NOT_ANIMATED );
  checkAnimComp( tau, ENTITY_WITH_NOTHING, 0, tau->nFramesPerStrip - 1, GOING_FORWARD );
}

TEST_F(Tau, xAnim_vanillaStrip_fullPingPong) {
  advanceOneStrip( tau );
  checkIfEntityIsAnimated( tau, ENTITY_WITH_NOTHING, NOT_ANIMATED );
  checkAnimComp( tau, ENTITY_WITH_NOTHING, 0, tau->nFramesPerStrip - 1, GOING_FORWARD );
  advanceOneStrip( tau );
  checkIfEntityIsAnimated( tau, ENTITY_WITH_NOTHING, NOT_ANIMATED );
  checkAnimComp( tau, ENTITY_WITH_NOTHING, 0, tau->nFramesPerStrip - 1, GOING_FORWARD );
}

