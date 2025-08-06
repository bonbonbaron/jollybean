#include "tau.h"
#include "x/xRender.h"

// Images we're testing xRender with
extern Image redImg;
extern Image blehImg;
extern Image heckImg;

static Entity Entity1_Red_Guy = 1;
static Entity Entity2_Tan_Circle = 2;
static Entity Entity3_Brown_Rect = 3;

Image* imgA[] = {
  &redImg,   // Entity1_Red_Guy
  &blehImg,  // Entity2_Tan_Circle
  &heckImg   // Entity3_Brown_Rect
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

// from xCollision.c
inline static Bln _collided(const register Rect_ *r1P, const register Rect_ *r2P) {
  return r1P->y        < r2P->y + r2P->h &&  
         r1P->y + r1P->h > r2P->y        && 
         r1P->x        < r2P->x + r2P->w &&
         r1P->x + r1P->w > r2P->x;        
}

// Make this unit test dumb; we're not worried about performance here.
// n^2 testing is fine.
static void checkForCollisions( Tau* tau ) {
  XRender* xP = tau->xP;
  System* sP = &xP->system;
  XRenderComp* cF = xP->system.cF;
  for ( Key i = 0; i < N_LAYERS_SUPPORTED; ++i ) {
    List* listP = &xP->layerListA[ i ];
    if ( listP->flags & LIST_HAS_ELEMS ) {
      XRenderComp* cP = &cF[ listP->head ];
      XRenderComp* cEndP = &cF[ listP->tail ];
      goto SKIP_FIRST_LISTHDR_INCREMENT;
      // render each component on the current layer
      for ( ; cP != cEndP ; ) {
        cP = &cF[ cP->hdr.next ];  // Putting incrementer here so it happens after the for-loop check, not before.
SKIP_FIRST_LISTHDR_INCREMENT:
        if ( cP != cEndP ) {
          XRenderComp* cNextP = &cF[ cP->hdr.next ];
          goto SKIP_FIRST_COMPARISON_INCREMENT;
          while ( cNextP != cEndP ) {
            cNextP = &cF[ cNextP->hdr.next ];  // Putting incrementer here so it happens after the for-loop check, not before.
SKIP_FIRST_COMPARISON_INCREMENT:
            if ( _collided( cP->dstRectP, cNextP->dstRectP ) ) {
              Entity e1 = xGetEntityByVoidComponentPtr( sP, cP );
              Entity e2 = xGetEntityByVoidComponentPtr( sP, cNextP );
              mailboxWrite( tau->xP->system.mailboxF, RENDER, e1, MSG_SOFT_COLLISION_DETECTED, e2, NULL );
            }
          }
        }
      }
    }
  }
}

static void sendElevationMsg( Tau* tau, Entity entity, S32 deltaZ ) {
  if ( deltaZ < 0 ) {
    mailboxWrite( tau->xP->system.mailboxF, RENDER, entity, MSG_MOVE_DOWN_A_LAYER, 0, NULL );
  }
  else if ( deltaZ > 0 ) {
    mailboxWrite( tau->xP->system.mailboxF, RENDER, entity, MSG_MOVE_UP_A_LAYER, 0, NULL );
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

static void elevateAndSend( Tau* tau, Entity entity, S32 deltaZ ) {
  elevateEntity( tau, entity, deltaZ );
  sendElevationMsg( tau, entity, deltaZ );
}

static void runAndCheckZOrder( Tau* tau ) {
  checkForCollisions( tau );
  xRun(&tau->xP->system);  
  System *sP = &tau->xP->system;
  XRender* xP = tau->xP;
  XRenderComp* cF = xP->system.cF;

  for ( Key i = 0; i < N_LAYERS_SUPPORTED; ++i ) {
    List* listP = &xP->layerListA[ i ];
    if ( listP->flags & LIST_HAS_ELEMS ) {
      XRenderComp* cP = &cF[ listP->head ];
      XRenderComp* cEndP = &cF[ listP->tail ];
      goto SKIP_FIRST_LISTHDR_INCREMENT;
      // render each component on the current layer
      while ( cP != cEndP ) {
        cP = &cF[ cP->hdr.next ];  // Putting incrementer here so it happens after the for-loop check, not before.
SKIP_FIRST_LISTHDR_INCREMENT:
        if ( cP != cEndP ) {
          XRenderComp* cNextP = &cF[ cP->hdr.next ];
          goto SKIP_FIRST_COMPARISON_INCREMENT;
          while ( cNextP != cEndP ) {
SKIP_FIRST_COMPARISON_INCREMENT:
            cNextP = &cF[ cNextP->hdr.next ];  // Putting incrementer here so it happens after the for-loop check, not before.
            if ( _collided( cP->dstRectP, cNextP->dstRectP ) ) {
              REQUIRE_LE( cP->dstRectP->y + cP->dstRectP->h, cNextP->dstRectP->y + cNextP->dstRectP->h );
            }
            // else, if not collided, then z-height ordering doesn't matter WITHIN THIS LAYER
          }
        }
      }
    }
  }
}

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

#if 0
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
  moveEntity( tau, 1, 0, -1 );
  for (int i = 0; i < N_FRAMES; ++i) {
    runAndCheckZOrder( tau );
  }
}

TEST_F(Tau, moveUpWhileOneIsActivated) {
  xActivateComponentByEntity( tau->sP, 1 );
  moveEntity( tau, 1, 0, -1 );
  for (int i = 0; i < N_FRAMES; ++i) {
    runAndCheckZOrder( tau );
  }
}

TEST_F(Tau, moveUpWhileMultipleAreActivated) {
  // All three entities are at height 2 because of setup function.
  xActivateComponentByEntity( tau->sP, Entity1_Red_Guy );
  xActivateComponentByEntity( tau->sP, Entity2_Tan_Circle );
  xActivateComponentByEntity( tau->sP, Entity3_Brown_Rect );
  // Put entity 2 below the other two.
  moveEntity( tau, Entity2_Tan_Circle, 0, 25 );
  for (int i = 0; i < N_FRAMES; ++i) {
    /* Make entity 2 scoot up each frame. 
     * It should fall beneath each entity as its bottom 
     * boundary crosses theirs. */
    moveEntity( tau, Entity2_Tan_Circle, 0, -1 );
    SDL_Delay(100);
    runAndCheckZOrder( tau );
  }
}

#endif
TEST_F(Tau, moveDownWhileMultipleAreActivated) {
  xActivateComponentByEntity( tau->sP, Entity1_Red_Guy );
  xActivateComponentByEntity( tau->sP, Entity2_Tan_Circle );
  xActivateComponentByEntity( tau->sP, Entity3_Brown_Rect );
  moveEntity( tau, Entity2_Tan_Circle, 0, 25 );
  for (int i = 0; i < N_FRAMES; ++i) {
    moveEntity( tau, Entity1_Red_Guy, 0, 1 );
    SDL_Delay(100);
    runAndCheckZOrder( tau );
  }
}

#if 0
TEST_F(Tau, elevateWhileOneIsActivated) {
  elevateAndSend( tau, 1, -1 );
  xActivateComponentByEntity( tau->sP, 1 );
  for (int i = 0; i < N_FRAMES; ++i) {
    runAndCheckZOrder( tau );
  }
  memReport();
}
#endif
