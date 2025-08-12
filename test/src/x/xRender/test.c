#include "tau.h"
#include "x/xRender.h"

// Images we're testing xRender with
extern Image redImg;
extern Image blehImg;
extern Image heckImg;

static Entity Entity1_Red_Guy = 1;
static Entity Entity2_Tan_Circle = 2;
static Entity Entity3_Brown_Rect = 3;
static Entity Entity4_Brown_Rect = 4;
static Entity Entity5_Brown_Rect = 5;
static Entity Entity6_Brown_Rect = 6;

static char* unused = "UNUSED";
static char* redName = "red";
static char* tanCircleName = "tan circle";
static char* brownRectName = "brown rect";
static char* brownRectName_cp1 = "brown rect - copy 1";
static char* brownRectName_cp2 = "brown rect - copy 2";
static char* brownRectName_cp3 = "brown rect - copy 3";

char** namePA[] = {
  &unused,
  &redName,
  &tanCircleName,
  &brownRectName,
  &brownRectName_cp1,
  &brownRectName_cp2,
  &brownRectName_cp3
};

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
  U32 nImgs;
  U32 nMutationsPerEntity;
  Map *shareMPMP;
  System *sP;
  XRenderComp *renderCompF;
  Collision *collisionA;  // placeholder for xCollision comps
  List *blobListF;
  U32 blobIdA[256];  // keeps track of each list's new/orig ID
  U32 nCollisionAElems;
  Key blobId;  // local substitute for incrementer that'll be in xCollisionRun()
  Gui *guiP;
  Bln readyToKillGui;
  // Collision collisionA[(sizeof(Key) << 8) - 1];  // local substitute for incrementer that'll be in xCollisionRun()
} Tau;

/* we need to simulate the following:
 *
 *  xCollision finds collisions
 *  xCollision writes to a shared list of blob members for each collision
 *  xRender gets this shared list at start-time
 *  x
 */
// from xCollision.c
inline static Bln _collided( const XRenderComp* c1P, const XRenderComp *c2P ) {
  Rect_ *r1P = c1P->dstRectP, *r2P = c2P->dstRectP;
  return *c1P->zHeightP == *c2P->zHeightP &&
    r1P->y        < r2P->y + r2P->h &&  
    r1P->y + r1P->h > r2P->y        && 
    r1P->x        < r2P->x + r2P->w &&
    r1P->x + r1P->w > r2P->x;        
}

#define blobIdIdxOf_(x_) tau->collisionA[ x_ ].blobIdIdx 
#define blobIdOf_(x_) tau->blobIdA[ blobIdIdxOf_( x_ ) ]
#define blobListOf_(x_) tau->blobListF[ blobIdOf_( x_ ) ]
#define nameOf_(x_) (*namePA[x_])
const ListNodeHeader UNSET_HEADER = {
  .prev = UNSET_,
  .next = UNSET_,
  .listId = UNSET_
};
// Make this unit test dumb; we're not worried about performance here.
// n^2 testing is fine.
static void checkForCollisions( Tau* tau ) {
  U32 frayIdx;
  _frayClr( tau->blobListF );
  memset( tau->collisionA, 0, sizeof( Collision ) * tau->nCollisionAElems );
  memset( tau->blobIdA, 0, sizeof( U32 ) * 256 );
  tau->blobId = UNSET_;  // will increment to 0 on the first blob
  for ( int i = 0; i < arrayGetNElems( tau->collisionA ); ++i ) {
    tau->collisionA[i].blobIdIdx = UNSET_;
  }
  XRender* xP = tau->xP;
  System* sP = &xP->system;
  XRenderComp* cF = xP->system.cF;
  // This iterates for collisions differently since xRender's layer list isn't necessarily updated yet.
  for ( Key i = 0; i < *_frayGetFirstInactiveIdxP( cF ); ++ i ) {
    XRenderComp* cP = &cF[i];
    for ( Key j = i + 1; j < *_frayGetFirstInactiveIdxP( cF ); ++j ) {
      XRenderComp* cNextP = &cF[j];
      if ( _collided( cP, cNextP ) ) {
        Entity e1 = xGetEntityByVoidComponentPtr( sP, cP );
        Entity e2 = xGetEntityByVoidComponentPtr( sP, cNextP );
        printf("found a collision between %d (%s) and %d (%s)\n", e1, nameOf_(e1), e2, nameOf_(e2));
        // If e1's in a blob but not e2, eat e2.
        if ( blobIdIdxOf_( e1 ) != UNSET_ ) {
          if ( blobIdIdxOf_( e2 ) == UNSET_ ) {
            blobIdIdxOf_( e2 ) = blobIdIdxOf_( e1 );
            printf( "%s's blob id: %d, id idx: %d\n", nameOf_(e2), blobIdOf_( e2 ), blobIdIdxOf_(e2));
            Collision* c2P = &tau->collisionA[ e2 ];
            c2P->hdr = UNSET_HEADER;
            c2P->entity = e2;
            c2P->blobIdIdx = blobIdIdxOf_( e1 );
            c2P->bottomYCoord = cNextP->dstRectP->y + cNextP->dstRectP->h;
            listAppend( &blobListOf_( e2 ), &c2P->hdr );
            printf( "1: putting %s into list %d \n", nameOf_(e2), blobIdOf_( e2 ) );
          }
          else if ( blobIdOf_( e1 ) < blobIdOf_( e2 ) ) {
            listMerge( &blobListOf_( e2 ), &blobListOf_( e1 ) );
            printf( "a: merging list %d into list %d \n", blobIdOf_( e2 ), blobIdOf_( e1 ) );
            blobIdOf_( e2 ) = blobIdOf_( e1 );
          }
          else if ( tau->collisionA[ e2 ].blobIdIdx > tau->collisionA[ e1 ].blobIdIdx ) {
            listMerge( &blobListOf_( e1 ), &blobListOf_( e2 ) );
            printf( "b: merging list %d into list %d \n", blobIdOf_( e1 ), blobIdOf_( e2 ) );
            blobIdOf_( e1 ) = blobIdOf_( e2 );
          }
          // else, if they're already in the same blob, ignore (SHOULD NEVER HAPPEN)
        }
        else {  // if e1 is UNSET_
          if ( blobIdIdxOf_( e2 ) == UNSET_ ) { // if  both e1 and e2 are unset
            ++tau->blobId;   // starts at 255 (UNSET_)

            tau->blobIdA[ tau->blobId ] = tau->blobId;  // Initially, a blob ID's index equals itself.
                                                        // On merges, we want all members to know they serve a new king.
            blobIdIdxOf_( e1 ) = tau->blobId;
            blobIdIdxOf_( e2 ) = tau->blobId;
            Collision* c1P = &tau->collisionA[ e1 ];
            Collision* c2P = &tau->collisionA[ e2 ];
            // Put 2nd entity in the blob
            c1P->hdr = UNSET_HEADER;
            c1P->entity = e1;
            c1P->blobIdIdx = tau->blobId;
            c1P->bottomYCoord = cP->dstRectP->y + cP->dstRectP->h;
            // Put 2nd entity in the blob
            c2P->hdr = UNSET_HEADER;
            c2P->entity = e2;
            c2P->blobIdIdx = c1P->blobIdIdx;
            c2P->bottomYCoord = cNextP->dstRectP->y + cNextP->dstRectP->h;
            // Construct list in-place
            // TODO why is red still UNSET_ on its second collision?
            frayAddEmpty( tau->blobListF, &frayIdx );
            printf("making list at tau->blobListF[ %d ]\n", frayIdx);
            listIni( &tau->blobListF[ frayIdx ], blobIdOf_( e1 ), tau->collisionA, tau->blobId == 0 );
            printf( "%d's blob id: %d\n", e1, blobIdOf_( e1 ));
            printf( "%d's blob id idx: %d\n", e1, blobIdIdxOf_( e1 ));
            printf( "%d's blob id: %d\n", e2, blobIdOf_( e2 ));
            printf( "%d's blob id idx: %d\n", e2, blobIdIdxOf_( e2 ));
            listAppend( &blobListOf_( e1 ), &c1P->hdr );
            listAppend( &blobListOf_( e2 ), &c2P->hdr );
            printf( "2: putting %s into list %d \n", nameOf_(e1), blobIdOf_( e1 ) );
            printf( "3: putting %s into list %d \n", nameOf_(e2), blobIdOf_( e2 ) );
          }  // if neither e1 nor e2 is set
             // if  e1's unset but e2's set
          else {
            Collision* c1P = &tau->collisionA[ e1 ];
            c1P->hdr = UNSET_HEADER;
            c1P->entity = e1;
            c1P->blobIdIdx = blobIdIdxOf_( e2 );
            c1P->bottomYCoord = cP->dstRectP->y + cP->dstRectP->h;
            listAppend( &blobListOf_( e2 ), &c1P->hdr );
            printf( "4: putting %s into list %d \n", nameOf_(e1), blobIdOf_( e2 ) );
          }  // if e1's unset but e2's set
        }  // if e1's unset
      }  // if e1 and e2 are collided
    }  // for each RHS dest rectangle
  } // for each LHS dest rectangle
  if ( tau->blobId != UNSET_ ) {
    mailboxWrite( tau->xP->system.mailboxF, RENDER, 0, MSG_SOFT_COLLISIONS_DETECTED, 0, NULL );
  }
}  //checkForCollisions()

static Key elevateEntity( Tau* tau, Entity entity, S32 deltaZ ) {
  assert( entity  );
  Map *sharedZHeightMP = mapGetNestedMapP( tau->shareMPMP, Z_HEIGHT );
  assert( sharedZHeightMP );
  U8* zP = (U8*) mapGet( sharedZHeightMP, entity );
  assert( zP );
  Key oldZHeight = *zP;
  *zP += deltaZ;
  printf("\e[92mnew z-height: %d\n", *zP);
  return oldZHeight;
}

static void moveEntity( Tau* tau, Entity entity, S32 x, S32 y ) {
  assert( entity  );
  Map *sharedDstRectMP = mapGetNestedMapP( tau->shareMPMP, DST_RECT );
  assert( sharedDstRectMP );
  Rect_* rectP = (Rect_*) mapGet( sharedDstRectMP, entity );
  assert( rectP );
  rectP->x += x;
  rectP->y += y;
}

static void elevateAndSend( Tau* tau, Entity entity, S32 deltaZ ) {
  assert( entity  );
  Key oldZHeight = elevateEntity( tau, entity, deltaZ );
  printf("old z height: %d\e[0m\n", oldZHeight);
  mailboxWrite( tau->xP->system.mailboxF, RENDER, entity, MSG_LAYER_CHANGED, oldZHeight, NULL );
}

static void runAndCheckZOrder( Tau* tau ) {
  printf("checking for colls\n");
  checkForCollisions( tau );
  printf("running render system\n");
  xRun(&tau->xP->system);  
  puts("\n");
  System *sP = &tau->xP->system;
  XRender* xP = tau->xP;
  XRenderComp* cF = xP->system.cF;

  for ( Key i = 0; i < N_LAYERS_SUPPORTED; ++i ) {
    List* listP = &xP->layerListA[ i ];
    if ( listP->head != UNSET_ ) {
      XRenderComp* cP = &cF[ listP->head ];
      goto SKIP_FIRST_LISTHDR_INCREMENT2;
      // render each component on the current layer
      while ( cP->hdr.next != UNSET_ ) {
        cP = &cF[ cP->hdr.next ];  // Putting incrementer here so it happens after the for-loop check, not before.
SKIP_FIRST_LISTHDR_INCREMENT2:
        if ( cP->hdr.next != UNSET_ ) {
          XRenderComp* cNextP = &cF[ cP->hdr.next ];
          goto SKIP_FIRST_COMPARISON_INCREMENT2;
          while ( cNextP->hdr.next != UNSET_ ) {
            cNextP = &cF[ cNextP->hdr.next ];  // Putting incrementer here so it happens after the for-loop check, not before.
SKIP_FIRST_COMPARISON_INCREMENT2:
            if ( _collided( cP, cNextP ) ) {
              // If they're colliding, then current rectangle needs to be higher up than the next, which will be drawn over it.
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
  tau->readyToKillGui = FALSE;
  // Make a new GUI.

  tau->nCollisionAElems = 255;
  tau->blobListF = frayNew( sizeof(List), 255, GENERAL );
  tau->collisionA = arrayNew( sizeof(Collision), tau->nCollisionAElems, GENERAL );
  tau->blobId = UNSET_;

  // Initialize the system basics.
  tau->nEntities = sizeof( namePA ) / sizeof( namePA[0] );  // for lack of anything better for now
  tau->nImgs = sizeof( imgA ) / sizeof( imgA[0] );  // for lack of anything better for now
  U32 nEntities;
  tau->nMutationsPerEntity = N_MUTATIONS_PER_ENTITY;
  xIniSys(tau->sP, tau->nEntities, NULL);
  tau->renderCompF = tau->sP->cF;
  REQUIRE_EQ(tau->sP->compSz, sizeof(XRenderComp));
  REQUIRE_EQ(xGetNComps(tau->sP), tau->nEntities);

  // ************ SHARES **************
  tau->shareMPMP = mapNew( MAP_POINTER, sizeof(Map*), 4, GENERAL);

  // Make the share inner map. This maps entities to actual, raw data.
  Map* sharedSrcRectMP = mapNew( RAW_DATA, sizeof(Rect_), tau->nEntities, GENERAL);
  Map* sharedDstRectMP = mapNew( RAW_DATA, sizeof(Rect_), tau->nEntities, GENERAL);
  Map* sharedZHeightMP = mapNew( RAW_DATA, sizeof(U8), tau->nEntities, GENERAL);
  // Map* sharedOffsetMP  = mapNew( RAW_DATA, sizeof(RectOffset), tau->nEntities, GENERAL);
  Map* sharedSingletonMP    = mapNew( NONMAP_POINTER, sizeof(void*), 2, GENERAL);

  // Set gui pointer in its map... 
  // TODO wouldn't it be nice to have SDL or whatever just be owned by the rendering system?
  mapSet(sharedSingletonMP, BLOB_KEY_, &tau->blobListF);

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

  // mapSet(tau->shareMPMP, RECT_OFFSET, &sharedOffsetMP); 
  mapSet(tau->shareMPMP, SINGLETON_TYPE, &sharedSingletonMP);
  // Give the shared map to the system. (This particular system wants a pointer to the inner shared map.)
  tau->xP->system.getShare(&tau->xP->system, tau->shareMPMP);

  // Set the stripsets and stripmaps up since xRender relies on that already being done by xMaster.
  for (int i = 0; i < tau->nImgs; ++i ) {
    stripIni( imgA[i]->cmP->sdP, TEMPORARY );
  }

  // If ordering is sacred, then we need fixed functions that'll encapsulate this sacred ordering for us.
  // Now add the entity to the actual system itself.
  forEachEntity_( tau->nEntities ) {
    if ( entity <= 3 ) {
      xAddEntityData(&tau->xP->system, entity, RENDER | IMAGE, imgA[ entity - 1 ]);
    }
    else {
      xAddEntityData(&tau->xP->system, entity, RENDER | IMAGE, imgA[ 2 ]);
    }
  }

  tau->sP->postprocessComps( tau->sP );
  memRst( TEMPORARY );
}

TEST_F_TEARDOWN(Tau) {
  for (int i = 0; i < tau->nImgs; ++i ) {
    imgA[i]->state = 0;
  }
  if ( tau->readyToKillGui ) {
    mailboxWrite( tau->sP->mailboxF, RENDER, 0, MSG_KILL_GUI, 0, NULL );
  }
  memRst( GENERAL );
}

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

// Failing, likely because we put red in the list before even activating him. In fact, activation alone should put somebody in a list.
TEST_F(Tau, moveUpWhileDeactivated) {
  moveEntity( tau, Entity1_Red_Guy, 0, -1 );
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

// FAILED
TEST_F(Tau, moveUpWhileMultipleAreActivated) {
  // All three entities are at height 2 because of setup function.
  xActivateComponentByEntity( tau->sP, Entity1_Red_Guy );
  xActivateComponentByEntity( tau->sP, Entity2_Tan_Circle );
  xActivateComponentByEntity( tau->sP, Entity3_Brown_Rect );
  xActivateComponentByEntity( tau->sP, Entity4_Brown_Rect );
  // Put entity 2 below the other two.
  moveEntity( tau, Entity2_Tan_Circle, 0, 25 );
  moveEntity( tau, Entity4_Brown_Rect, 100, 50 ); // isolate this one
  for (int i = 0; i < 80; ++i) {
    /* Make entity 2 scoot up each frame. 
     * It should fall beneath each entity as its bottom 
     * boundary crosses theirs. */
    moveEntity( tau, Entity2_Tan_Circle, 0, -1 );
    SDL_Delay(50);
    runAndCheckZOrder( tau );
  }
}
//FAILED
TEST_F(Tau, moveDownWhileMultipleAreActivated) {
  xActivateComponentByEntity( tau->sP, Entity1_Red_Guy );
  xActivateComponentByEntity( tau->sP, Entity2_Tan_Circle );
  xActivateComponentByEntity( tau->sP, Entity3_Brown_Rect );
  moveEntity( tau, Entity2_Tan_Circle, 0, 25 );
  for (int i = 0; i < N_FRAMES; ++i) {
    // first third, move it and show same layer being y-sorted
    if ( i < ( N_FRAMES / 3 ) ) {
      moveEntity( tau, Entity1_Red_Guy, 0, 1 );
    }
    // second third, hold it above layer 2
    else if ( i == N_FRAMES / 3 ) {
      elevateAndSend( tau, Entity1_Red_Guy, 1 );
      printf("\e[91melevated!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\e[0m\n");
    }
    // last third, put it on layer 2
    else if ( i == 2 * N_FRAMES / 3 ) {
      elevateAndSend( tau, Entity1_Red_Guy, -1 );
      // send an artificial collision detected message since our test only checks collisions in xRender's fray
      printf("\e[91mde-elevated!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\e[0m\n");
    }

    SDL_Delay(50);
    runAndCheckZOrder( tau );
  }
}

TEST_F(Tau, elevateWhileOneIsActivated) {
  // He hasn't added him to the list yet...
  elevateAndSend( tau, 1, -1 );
  // moving entity from z 2 to 1, but activation breaks, thinking it's 
  xActivateComponentByEntity( tau->sP, 1 );
  for (int i = 0; i < N_FRAMES; ++i) {
    runAndCheckZOrder( tau );
  }
}

TEST_F(Tau, elevateActivateThenDeactivate) {
  // He hasn't added him to the list yet...
  elevateAndSend( tau, 1, -1 );
  // moving entity from z 2 to 1, but activation breaks, thinking it's 
  xActivateComponentByEntity( tau->sP, 1 );
  for (int i = 0; i < N_FRAMES; ++i) {
    runAndCheckZOrder( tau );
  }
  xDeactivateComponentByEntity( tau->sP, 1 );
  for (int i = 0; i < N_FRAMES; ++i) {
    runAndCheckZOrder( tau );
  }
  xActivateComponentByEntity( tau->sP, 1 );
  for (int i = 0; i < N_FRAMES; ++i) {
    runAndCheckZOrder( tau );
  }
  elevateAndSend( tau, 2, -1 );
  xActivateComponentByEntity( tau->sP, 2 );
  for (int i = 0; i < N_FRAMES; ++i) {
    runAndCheckZOrder( tau );
  }
  elevateAndSend( tau, 3, -2 );
  xActivateComponentByEntity( tau->sP, 3 );
  for (int i = 0; i < N_FRAMES; ++i) {
    runAndCheckZOrder( tau );
  }
}
