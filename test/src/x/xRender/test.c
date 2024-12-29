#include "tau.h"
#include "x/xRender.h"
// Images we're testing xRender with

extern Image redImg;
extern Image blehImg;
extern Image heckImg;

#define N_FRAMES (50)

Image* imgA[] = {
#if 0
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
#else
  &redImg,
  &blehImg,
  &heckImg
};
#endif

void moveEntity( Entity entity, U32 x, U32 y ) {
}

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
  tau->shareMPMP = mapNew( MAP_POINTER, sizeof(Map*), 5);

  // Make the share inner map. This maps entities to actual, raw data.
  Map* sharedSrcRectMP = mapNew( RAW_DATA, sizeof(Rect_), tau->nEntities);
  Map* sharedDstRectMP = mapNew( RAW_DATA, sizeof(Rect_), tau->nEntities);
  Map* sharedZHeightMP = mapNew( RAW_DATA, sizeof(U8), tau->nEntities);
  // Map* sharedOffsetMP  = mapNew( RAW_DATA, sizeof(RectOffset), tau->nEntities);
  Map* sharedGuiMP     = mapNew( NONMAP_POINTER, sizeof(Gui*), 1);

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
    stripIni( imgA[i]->cmP->sdP );
  }

  // If ordering is sacred, then we need fixed functions that'll encapsulate this sacred ordering for us.
  // Now add the entity to the actual system itself.
  forEachEntity_( tau->nEntities ) {
#if 0
    Image* imgP = &imgA[entity - 1];
    xAddEntityData(&tau->xP->system, entity, RENDER | IMAGE, &imgP );
#else
    xAddEntityData(&tau->xP->system, entity, RENDER | IMAGE, imgA[ entity - 1 ]);
#endif
  }

  tau->sP->postprocessComps( tau->sP );
}

TEST_F_TEARDOWN(Tau) {
  for (int i = 0; i < tau->nEntities; ++i ) {
    stripClr( imgA[i]->cmP->sdP );
    imgA[i]->state = 0;
  }
  mapOfNestedMapsDel(&tau->shareMPMP );
  xClr( &tau->xP->system );
  guiDel( &tau->xP->guiP );
}

TEST_F(Tau, xRenderRun_red) {
  xActivateComponentByEntity( tau->sP, 1 );
  for (int i = 0; i < N_FRAMES; ++i) {
    xRun(&tau->xP->system);
  }
}

TEST_F(Tau, xRenderRun_bleh) {
  xActivateComponentByEntity( tau->sP, 2 );
  for (int i = 0; i < N_FRAMES; ++i) {
    xRun(&tau->xP->system);
  }
}

TEST_F(Tau, xRenderRun_heck) {
  xActivateComponentByEntity( tau->sP, 3 );
  for (int i = 0; i < N_FRAMES; ++i) {
    xRun(&tau->xP->system);
  }
}

TEST_F(Tau, somethingfornow) {
  for (int i = 0; i < N_FRAMES; ++i) {
    xRun(&tau->xP->system);
  }
}

TEST_F(Tau, moveUpWhileDeactivated) {
  for (int i = 0; i < N_FRAMES; ++i) {
    xRun(&tau->xP->system);
  }
}
