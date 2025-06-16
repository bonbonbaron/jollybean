#include "tau.h"
#include "x/xRender.h"

// Images we're testing xRender with
extern Image redImg;
extern Image blehImg;
extern Image heckImg;


Image* imgA[] = {
  &redImg,
  &blehImg,
  &heckImg
};

// We have to make these constants so the compiler doesn't cry about the array initializers not having constant sizes.
#define N_FRAMES (50)
#define N_MUTATIONS_PER_ENTITY (5)
#define forEachEntity_(nEntities_) for (Entity entity = 1; entity <= nEntities_; ++entity)

TAU_MAIN();

typedef struct Tau {
  XRender *xP;
  U32 nEntities;
  U32 nMutationsPerEntity;
  Map *shareMPMP;
  System *sP;
  XRenderComp *renderCompF;
} Tau;

static void sendMovementMsg( Tau* tau, Entity entity, S32 y ) {
  if ( y < 0 ) {
    mailboxWrite( tau->xP->system.mailboxF, RENDER, entity, MSG_MOVED_Y_UP, 0, NULL );
  }
  else if ( y > 0 ) {
    mailboxWrite( tau->xP->system.mailboxF, RENDER, entity, MSG_MOVED_Y_DOWN, 0, NULL );
  }
  // else, don't bother writing anything at all. We dgaf if it goes left or right.
}

static void sendElevationMsg( Tau* tau, Entity entity, S32 deltaZ ) {
  if ( deltaZ < 0 ) {
    mailboxWrite( tau->xP->system.mailboxF, RENDER, entity, MSG_MOVED_Z_DOWN, 0, NULL );
  }
  else if ( deltaZ > 0 ) {
    mailboxWrite( tau->xP->system.mailboxF, RENDER, entity, MSG_MOVED_Z_UP, 0, NULL );
  }
  // else, don't bother writing anything at all. We dgaf if it goes left or right.
}

static void elevateEntity( Tau* tau, Entity entity, S32 deltaZ ) {
  Map *sharedZHeightMP = mapGetNestedMapP( tau->shareMPMP, Z_HEIGHT );
  assert( sharedZHeightMP );
  U8* zP = (U8*) mapGet( sharedZHeightMP, entity );
  assert( zP );
  *zP += deltaZ;
}

static void moveEntity( Tau* tau, Entity entity, S32 x, S32 y ) {
  Map *sharedDstRectMP = mapGetNestedMapP( tau->shareMPMP, DST_RECT );
  assert( sharedDstRectMP );
  Rect_* rectP = (Rect_*) mapGet( sharedDstRectMP, entity );
  assert( rectP );
  rectP->x += x;
  rectP->y += y;
}

static void moveAndSend( Tau* tau, Entity entity, S32 x, S32 y ) {
  moveEntity( tau, entity, x, y );
  sendMovementMsg( tau, entity, y );
}

static void elevateAndSend( Tau* tau, Entity entity, S32 deltaZ ) {
  elevateEntity( tau, entity, deltaZ );
  sendElevationMsg( tau, entity, deltaZ );
}

static void runAndCheckZOrder( Tau* tau ) {
  xRun(&tau->xP->system);  
  System *sP = &tau->xP->system;

  XRenderComp* cP = (XRenderComp*) sP->cF;
  XRenderComp* cEndP = cP + *_frayGetFirstInactiveIdxP( sP->cF );  // only care about active ones

  ++cP;  // skip first so you can compare current to previous, only if more than one is active
  for ( ; cP < cEndP; ++cP ) {
     CHECK_GE( *cP->zHeightP, *(cP - 1)->zHeightP );
  }
}

void test( Tau* tau ) {
}


TEST_F_SETUP(Tau) {
  extern XRender xRender;
  tau->xP = &xRender;
  tau->sP = &tau->xP->system;
  // Make a new GUI.
  printf("a\n");
  tau->xP->guiP = guiNew();
  printf("b\n");

  // Initialize the system basics.
  tau->nEntities = sizeof( imgA ) / sizeof( imgA[0] );  // for lack of anything better for now
  tau->nMutationsPerEntity = N_MUTATIONS_PER_ENTITY;
  xIniSys(tau->sP, tau->nEntities, NULL);
  tau->renderCompF = tau->sP->cF;
  REQUIRE_EQ(tau->sP->compSz, sizeof(XRenderComp));
  REQUIRE_EQ(xGetNComps(tau->sP), tau->nEntities);

  // FIRST: Gather a list of requirements you need so you don't need to sit here scratching your head too much.
  // 1) source rects
  // 2) dest rects 
  // 3) colormaps
  // 4) color palettes
  // 5) body'ing the three test images you have in Makefile 
  // 6) Send them into the system in the setup stage
  // 7) 
  // ************ SHARES **************
  tau->shareMPMP = mapNew( MAP_POINTER, sizeof(Map*), 5, GENERAL);

  // Make the share inner map. This maps entities to actual, raw data.
  Map* sharedSrcRectMP = mapNew( RAW_DATA, sizeof(Rect_), tau->nEntities, GENERAL);
  Map* sharedDstRectMP = mapNew( RAW_DATA, sizeof(Rect_), tau->nEntities, GENERAL);
  Map* sharedZHeightMP = mapNew( RAW_DATA, sizeof(U8), tau->nEntities, GENERAL);
  // Map* sharedOffsetMP  = mapNew( RAW_DATA, sizeof(RectOffset), tau->nEntities, GENERAL);
  Map* sharedGuiMP     = mapNew( NONMAP_POINTER, sizeof(Gui*), 1, GENERAL);

  // Set gui pointer in its map... 
  // TODO wouldn't it be nice to have SDL or whatever just be owned by the rendering system?
  mapSet(sharedGuiMP, GUI_KEY_, &tau->xP->guiP);

  // Now populate the entities' shared rectangles.
  // You don't know what your source rect is until you mutate.
  const static U8 DEFAULT_Z = 2;
  forEachEntity_( tau->nEntities ) {
    Rect_ currSrcRect = { 0 };  // the post-mutate function will change this value before rendering
    mapSet(sharedSrcRectMP, entity, &currSrcRect);
    Rect_ currDstRect = { 0 };  // the post-mutate function will change this value before renderating
    mapSet(sharedDstRectMP, entity, &currDstRect);
    mapSet(sharedZHeightMP, entity, &DEFAULT_Z );  // start everybody off at Z-height = 2
  }
  // Map the inner share map to key value "0" in the outer shared map.
  mapSet(tau->shareMPMP, SRC_RECT, &sharedSrcRectMP);
  mapSet(tau->shareMPMP, DST_RECT, &sharedDstRectMP);
  mapSet(tau->shareMPMP, Z_HEIGHT, &sharedZHeightMP);
  //
  // mapSet(tau->shareMPMP, RECT_OFFSET, &sharedOffsetMP); 
  mapSet(tau->shareMPMP, GUI_KEY_, &sharedGuiMP);
  // Give the shared map to the system. (This particular system wants a pointer to the inner shared map.)
  tau->xP->system.getShare(&tau->xP->system, tau->shareMPMP);

  // Set the stripsets and stripmaps up since xRender relies on that already being done by xMaster.
  for (int i = 0; i < tau->nEntities; ++i ) {
    stripIni( imgA[i]->cmP->sdP, TEMPORARY );
  }

  // If ordering is sacred, then we need fixed functions that'll encapsulate this sacred ordering for us.
  // Now add the entity to the actual system itself.
  forEachEntity_( tau->nEntities ) {
    xAddEntityData(&tau->xP->system, entity, RENDER | IMAGE, imgA[ entity - 1 ]);
  }

  tau->sP->postprocessComps( tau->sP );
  memRst( TEMPORARY );
}

TEST_F_TEARDOWN(Tau) {
  for (int i = 0; i < tau->nEntities; ++i ) {
    imgA[i]->state = 0;
  }
  guiDel( &tau->xP->guiP );
  memRst( GENERAL );
}

#if 1
TEST_F(Tau, xRenderRun_red) {
  xActivateComponentByEntity( tau->sP, 1 );
  xRun(&tau->xP->system);
}

TEST_F(Tau, xRenderRun_bleh) {
  xActivateComponentByEntity( tau->sP, 2 );
  xRun(&tau->xP->system);
}

TEST_F(Tau, xRenderRun_heck) {
  xActivateComponentByEntity( tau->sP, 3 );
  xRun(&tau->xP->system);
}

TEST_F(Tau, somethingfornow) {
  xRun(&tau->xP->system);
}

TEST_F(Tau, moveUpWhileDeactivated) {
  moveAndSend( tau, 1, 0, -1 );
  runAndCheckZOrder( tau );
}

TEST_F(Tau, moveUpWhileOneIsActivated) {
  xActivateComponentByEntity( tau->sP, 1 );
  moveAndSend( tau, 1, 0, -1 );
  for (int i = 0; i < N_FRAMES; ++i) {
    runAndCheckZOrder( tau );
  }
}

TEST_F(Tau, moveUpWhileMultipleAreActivated) {
  xActivateComponentByEntity( tau->sP, 1 );
  xActivateComponentByEntity( tau->sP, 2 );
  xActivateComponentByEntity( tau->sP, 3 );
  moveAndSend( tau, 3, 0, 10 );
  for (int i = 0; i < N_FRAMES; ++i) {
    moveAndSend( tau, 2, 0, -1 );
    runAndCheckZOrder( tau );
  }
}

TEST_F(Tau, moveDownWhileMultipleAreActivated) {
  xActivateComponentByEntity( tau->sP, 1 );
  xActivateComponentByEntity( tau->sP, 2 );
  xActivateComponentByEntity( tau->sP, 3 );
  moveAndSend( tau, 3, 0, 10 );
  for (int i = 0; i < N_FRAMES; ++i) {
    moveAndSend( tau, 2, 0, 1 );
    runAndCheckZOrder( tau );
  }
}
#endif

TEST_F(Tau, elevateWhileOneIsActivated) {
  elevateAndSend( tau, 1, -1 );
  xActivateComponentByEntity( tau->sP, 1 );
  for (int i = 0; i < N_FRAMES; ++i) {
    runAndCheckZOrder( tau );
  }
  memReport();
}
