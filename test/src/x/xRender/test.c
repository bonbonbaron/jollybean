#include "tau/tau.h"
#if 0   // Don't use this for now, we gotta make sure it works first.
#define USE_HEADLESS_INTERFACE
#endif
#include "xRender.h"
// Images we're testing xRender with
#include "blehColormap.h"
#include "blehColorPalette.h"
#include "heckColormap.h"
#include "heckColorPalette.h"
#include "redColormap.h"
#include "redColorPalette.h"

Image imgA[] = {
  {
    .state = 0,
    .sortedRectIdx = 0,
    .cmP = &redColormap,
    .cpP = &redColorPalette
  },
  {
    .state = 0,
    .sortedRectIdx = 0,
    .cmP = &blehColormap,
    .cpP = &blehColorPalette
  },
  {
    .state = 0,
    .sortedRectIdx = 0,
    .cmP = &heckColormap,
    .cpP = &heckColorPalette
  }
};


// We have to make these constants so the compiler doesn't cry about the array initializers not having constant sizes.
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

TEST_F_SETUP(Tau) {
  extern XRender xRender;
  tau->xP = &xRender;
  tau->sP = &tau->xP->system;
  // Make a new GUI.
  tau->xP->guiP = guiNew();

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
  tau->shareMPMP = mapNew( MAP_POINTER, sizeof(Map*), 4);

  // Make the share inner map. This maps entities to actual, raw data.
  Map* sharedSrcRectMP = mapNew( RAW_DATA, sizeof(Rect_), tau->nEntities);
  Map* sharedDstRectMP = mapNew( RAW_DATA, sizeof(Rect_), tau->nEntities);
  Map* sharedOffsetMP  = mapNew( RAW_DATA, sizeof(RectOffset), tau->nEntities);
  Map* sharedGuiMP     = mapNew( NONMAP_POINTER, sizeof(Gui*), 1);

  // Set gui pointer in its map... TODO wouldn't it be nice to have SDL or whatever just be owned by the rendering system?
  mapSet(sharedGuiMP, GUI_KEY_, &tau->xP->guiP);

  // Now populate the entities' shared rectangles.
  // You don't know what your source rect is until you mutate.
  forEachEntity_( tau->nEntities ) {
    Rect_ currSrcRect = { 0 };  // the post-mutate function will change this value before rendering
    mapSet(sharedSrcRectMP, entity, &currSrcRect);

    Rect_ currDstRect = { 0 };  // the post-mutate function will change this value before renderating
    mapSet(sharedDstRectMP, entity, &currDstRect);

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
  mapSet(tau->shareMPMP, GUI_KEY_, &sharedGuiMP);
  // Give the shared map to the system. (This particular system wants a pointer to the inner shared map.)
  tau->xP->system.getShare(&tau->xP->system, tau->shareMPMP);

  // Don't run system here. Instead, let each test case decide whether to advance by timestep, frame, or full strip.
}

TEST_F_TEARDOWN(Tau) {
  guiDel( &tau->xP->guiP );
  mapOfNestedMapsDel(&tau->shareMPMP );
  xClr( &tau->xP->system );
}

TEST_F(Tau, xRenderRun) {
  for (int i = 0; i < 50; ++i) {
    xRun(&tau->xP->system);
  }
}

TEST_F(Tau, somethingfornow) {
  // ====================================================
  // Repeat things done in multithreadingg test for setup
  // ====================================================
  Colormap** cmPF = frayNew( sizeof(Colormap*), tau->nEntities);
  StripDataS **sdPA = arrayNew(sizeof(StripDataS*), tau->nEntities);
  for (U32 i = 0; i < tau->nEntities; ++i) {
    frayAdd(cmPF, &imgA[i].cmP, NULL);
    sdPA[i] = cmPF[i]->sdP;
    stripIni(sdPA[i]);
  }

  // ====================================================
  // Clean up
  // ====================================================
  for (U32 i = 0; i < tau->nEntities; ++i) {
    stripClr(sdPA[i]);
  }
  arrayDel((void**) &sdPA);
  frayDel((void**) &cmPF);
  // arrayDel((void**) &atlasPixelA);  TODO what was this for? Did i accidentally delete something?
}
