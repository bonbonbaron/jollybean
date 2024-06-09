#include "tau/tau.h"
#define USE_HEADLESS_INTERFACE
#include "xRender.h"
// Images we're testing xRender with
#include "blehColormap.h"
#include "blehColorPalette.h"
#include "heckColormap.h"
#include "heckColorPalette.h"
#include "redColormap.h"
#include "redColorPalette.h"


// We have to make these constants so the compiler doesn't cry about the array initializers not having constant sizes.
#define N_ENTITIES (100)
#define N_MUTATIONS_PER_ENTITY (5)

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

TAU_MAIN();

typedef struct Tau {
  XRender *xP;
  U32 nEntities;
  U32 nMutationsPerEntity;
  Map *shareMPMP;
  System *sP;
  XRenderComp *renderCompF;
} Tau;

TEST_F_SETUP(Tau) {
  extern XRender xRender;
  tau->xP = &xRender;
  tau->sP = &tau->xP->system;
  // Make a new GUI.
  tau->xP->guiP = guiNew();

  // Initialize the system basics.
  tau->nEntities = N_ENTITIES;
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
  // 5) body'ing the three test images you have in Makefile (steal from 
  // ************ SHARES **************
  tau->shareMPMP = mapNew( MAP_POINTER, sizeof(Map*), 3);

  // Make the share inner map. This maps entities to actual, raw data.
  Map* sharedSrcRectMP = mapNew( RAW_DATA, sizeof(Rect_), tau->nEntities);
  Map* sharedDstRectMP = mapNew( RAW_DATA, sizeof(Rect_), tau->nEntities);
  Map* sharedOffsetMP  = mapNew( RAW_DATA, sizeof(RectOffset), tau->nEntities);
  Map* sharedGuiMP     = mapNew( NONMAP_POINTER, sizeof(Gui*), 1);

  // Set gui pointer in its map... TODO wouldn't it be nice to have SDL or whatever just be owned by the rendering system?
  mapSet(sharedGuiMP, GUI_KEY_, &tau->guiP);

  // Now populate the entities' shared rectangles.
  // You don't know what your source rect is until you mutate.
  forEachEntity_( tau->nEntities ) {
    Rect_ currSrcRect = { 0 };  // the post-mutate function will change this value before renderating
    mapSet(sharedSrcRectMP, entity, &currSrcRect);

    Rect_ currDstRect = { 0 };  // the post-mutate function will change this value before renderating
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
    xAddEntityData(&tau->xP->system, entity, RENDER, NULL);

  }
  // Map the inner share map to key value "0" in the outer shared map.
  mapSet(tau->shareMPMP, SRC_RECT, &sharedSrcRectMP);
  mapSet(tau->shareMPMP, DST_RECT, &sharedDstRectMP);
  mapSet(tau->shareMPMP, RECT_OFFSET, &sharedOffsetMP);
  // Give the shared map to the system. (This particular system wants a pointer to the inner shared map.)
  tau->xP->system.getShare(&tau->xP->system, tau->shareMPMP);

  xRun( tau->sP );

  

  // PREPARE TEST SAMPLES
  // *********************************
  // Entity 6 strip 1: repeat + pingpong
#define ENTITY_WITH_PINGPONG_REPEAT (6)
  mailboxWrite(tau->sP->mailboxF, ANIMATION, ENTITY_WITH_PINGPONG_REPEAT, MUTATE_AND_ACTIVATE, 1);

  // *********************************
  // Entity 1 strip 2: pingpong only
#define ENTITY_WITH_PINGPONG (1)
  mailboxWrite(tau->sP->mailboxF, ANIMATION, ENTITY_WITH_PINGPONG, MUTATE_AND_ACTIVATE, 2);

  // *********************************
  // Entity 88 strip 3: repeat only
#define ENTITY_WITH_REPEAT (88)  // TODO change back to 88 after you find and fix bug
  mailboxWrite(tau->sP->mailboxF, ANIMATION, ENTITY_WITH_REPEAT, MUTATE_AND_ACTIVATE, 3);

  // *********************************
  // Entity 30 strip 4: neither repeat nor pingpong
#define ENTITY_WITH_NOTHING (30)
  mailboxWrite(tau->sP->mailboxF, ANIMATION, ENTITY_WITH_NOTHING, MUTATE_AND_ACTIVATE, 4);


  // Don't run system here. Instead, let each test case decide whether to advance by timestep, frame, or full strip.
}

TEST_F_TEARDOWN(Tau) {
  guiDel( &tau->xP->guiP );
  mapOfNestedMapsDel(&tau->shareMPMP );
  xClr( &tau->xP->system );
}

TEST_F(Tau, xRenderRun) {
  REQUIRE_TRUE(1);
}
#define USE_HEADLESS_INTERFACE
//#include "previewImg.h"

TEST_F(Tau, somethingfornow) {
  // ====================================================
  // Repeat things done in multithreadingg test for setup
  // ====================================================

  Colormap *cmPA[] = {
    &blehColormap,
    &redColormap,
    &heckColormap
  };

  ColorPalette *cpPA[] = {
    &blehColorPalette,
    &redColorPalette,
    &heckColorPalette
  };

  // TODO Why're we putting it in a fray?
  Colormap** cmPF = frayNew( sizeof(Colormap*), 3);
  for (U32 i = 0; i < 3; ++i) {
    frayAdd(cmPF, &cmPA[i], NULL);
  }

  U32 N_SAMPLES = *_frayGetFirstEmptyIdxP(cmPF);

  // Offset color palette and colormap indices to match future texture atlas destinations
  U8 atlasOffset = 0;
  for (int i = 0; i < N_SAMPLES; ++i) {
    atlasOffset += cpPA[i]->nColors;
  }

  // Extract stripdatas into an array so we can multithread process them all.
  StripDataS **sdPA = arrayNew(sizeof(StripDataS*), N_SAMPLES);

  for (U32 i = 0, iEnd = arrayGetNElems(sdPA); i < iEnd; ++i) {
    sdPA[i] = cmPF[i]->sdP;
  }

  // Inflate colormap inflatables
  for (int i = 0; i < N_SAMPLES; ++i) {
    stripIni(sdPA[i]);
  }

  // ====================================================
  // Clean up
  // ====================================================
  arrayDel((void**) &sdPA);
  frayDel((void**) &cmPF);
  // arrayDel((void**) &atlasPixelA);  TODO what was this for? Did i accidentally delete something?
}
