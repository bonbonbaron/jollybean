#include "x/xCollision.h"
#include "jb.h"

// TODO
XConsumeGeneFuncDefUnused_(Collision);
XPostprocessCompsDefUnused_(Collision);
XPostMutateFuncDefUnused_(Collision);
XPostActivateFuncDefUnused_(Collision);
XPostDeactivateFuncDefUnused_(Collision);


// Based on the "Performance Comparisons" link, we use brute force until
// the number of moving elements exceeds 200. When it does, we switch to the box-
// intersect algorithm found in link to the paper. 
// ================================================
// Performance Comparisons: https://0fps.net/2015/01/23/collision-detection-part-3-benchmarks/
// Github:                  https://github.com/mikolalysenko/box-intersect
// Paper:                   https://pub.ist.ac.at/~edels/Papers/2002-J-01-FastBoxIntersection.pdf
// ================================================
// TODO: In the future, when I/we implement 16-bit keys, we'll make it switch between brute force
//       and "loose grids" found on {TODO put URL here}.


//======================================================
// Initialize Collision's system.
//======================================================
void xCollisionIniSys(System *sP, void *sParamsP) {
	unused_(sParamsP);
  unused_(sP);
}

//======================================================
// Initialize xCollision's components, which are Images.
//======================================================
void xCollisionIniSubcomp(System *sP, void *compDataP, void *compDataSrcP) {
  assert(sP && compDataP && compDataSrcP);
  XCollisionComp *cP = (XCollisionComp*) compDataP;
  cP->type = *((U8*) compDataSrcP);
}

#define CHANGE_LAYER (1) //TODO
#define ENTITY_STARTED_MOVING (2) //TODO
#define ENTITY_STOPPED_MOVING (3) //TODO
void xCollisionProcessMessage(System *sP, Message *msgP) {
  assert(sP && msgP);
  XCollisionComp *cP;
  cP = xGetCompPByEntity(sP, msgP->attn);
  assert( cP );
  switch (msgP->cmd) {
    case CHANGE_LAYER:
      cP->layer = msgP->arg;
      break; 
    // XColl pauses entities when they come onscreen (1 swap). If they start moving, simply unpause.
    case MSG_MOTION_STARTED:
      // TODO redo this logic with lists
      // xUnpauseComponentByEntity(sP, msgP->attn);
      break;
    case MSG_MOTION_STOPPPED:
      // TODO redo this logic with lists
      // xPauseComponentByEntity(sP, msgP->attn);
      break;
  }
}

#define RECT (1)  // TODO move to enum (call these "keychains")
#define LAYER (2)  // TODO move to enum (call these "keychains")
#if 0  // TODO move to postprocess
XGetShareFuncDef_(Collision) {
  XCollision *xCollisionSysP = (XCollision*) sP;
  // Get system the rectangle array from master.
  Map *rectMP = shareGetMap( DST_RECT );
  Map *layerMP = shareGetMap( LAYER );
  xCollisionSysP->rectA = rectMP->mapA;
  // Give each component the index to its rectangle.
  XCollisionComp *cP = sP->cF;
  XCollisionComp *cStartP = cP;
  XCollisionComp *cEndP = cP + arrayGetNElems(sP->cF);
  U8 *layerP;
  for (; cP < cEndP; ++cP) {
    cP->entity = sP->cIdx2eA[cP - cStartP];
    cP->collRect.idx = mapGetIndex(rectMP, cP->entity);
    layerP = (U8*) mapGet(layerMP, cP->entity);
    assert( layerP );
    cP->layer = *layerP;
  }
}
#endif

inline static Bln _collided(const register Rect_ *r1P, const register Rect_ *r2P) {
  return r1P->y        < r2P->y + r2P->h &&  
         r1P->y + r1P->h > r2P->y        && 
         r1P->x        < r2P->x + r2P->w &&
         r1P->x + r1P->w > r2P->x;        
}

#define COLLISION 1 //TODO move to enum
#define ANTICOLLISION 2 //TODO move to enum
#define CAMERA_COLLISION_TYPE 3
#define TILE_DIM_LOG 3
#define HALF_TILE_DIM 4
inline static void _alertCollision(System *sP, Entity entity, Key collType, Entity otherEntity) {
  mailboxWrite(sP->mailboxF, entity, COLLISION, collType, otherEntity, NULL);
}

inline static void _alertAntiCollision(System *sP, Entity entity, Entity cameraEntity) {
  mailboxWrite(sP->mailboxF, entity, ANTICOLLISION, CAMERA_COLLISION_TYPE, cameraEntity, NULL);
}

static void _alertCollisions(System *sP, Rect_ *rectA, XCollisionComp *c1P, XCollisionComp *c2P) {
  _alertCollision(sP, c1P->entity, c2P->type, c2P->entity);
  _alertCollision(sP, c2P->entity, c1P->type, c1P->entity);
}

//======================================================
// Collision run function
//======================================================

void xCollisionRun(System *sP) {
  XCollisionComp *cP = sP->cF;
  XCollisionComp *cStartP = cP;
  XCollisionComp *cFirstInactiveP = &cP[_frayGetFirstInactiveIdx(sP->cF)];
  XCollisionComp *cEndP = cP + arrayGetNElems(sP->cF);
  XCollisionComp *cFirstPausedP = cP + _frayGetFirstPausedIdx(sP->cF);

  XCollision *xCollSysP = (XCollision*) sP;
  Rect_ *rectA = xCollSysP->rectA;
  // Get camera.
  XCollisionComp *camCP = (XCollisionComp*) xGetCompPByEntity(sP, xCollSysP->cameraEntity);
  Rect_ camRect = rectA[camCP->collRect.idx];
  Entity cameraEntity = camCP->entity;
  // You'll need this later.
  const U32 N_TILES_WIDE = xCollSysP->worldWidth >> TILE_DIM_LOG;
  const U32 N_TILES_HIGH = xCollSysP->worldHeight >> TILE_DIM_LOG;
  const U32 N_TILES_PER_LAYER = N_TILES_HIGH * N_TILES_WIDE;

  // First check onscreen moving elems for anti-collisions in case someone left the screen.
  while (cP < cFirstPausedP) {
    if (!_collided(&camRect, &rectA[cP->collRect.idx])) {
      xPauseComponentByEntity(sP, cP->entity);
      _alertAntiCollision(sP, cP->entity, cameraEntity); // Active elems deactivate in two swaps
      --cFirstPausedP;                                       // when there are paused elems between. 
    }
    else 
      ++cP;
  }
  // Second, if camera's in motion, check paused elems (if any) for anti-collisions. 
  if (xCollSysP->cameraIsMoving) {
    while (cP < cFirstInactiveP) {
      if (!_collided(&camRect, &rectA[cP->collRect.idx])) {
        xDeactivateComponentByEntity(sP, cP->entity);  // paused elems deactivate in one swap
        --cFirstInactiveP;
      }
      else
        ++cP;
    }
    // Then check everyone else for camera collisions.
    for (cP = cFirstInactiveP; cP < cEndP; ++cP)  {
      if (_collided(&camRect, &rectA[cP->collRect.idx])) {
        _alertCollisions(sP, rectA, cP, camCP);
      }
    }
  }
  // Always test onscreen moving entities for collisions with other onscreen entities.
  for (cP = cStartP; cP < cFirstPausedP; ++cP) {
    for (XCollisionComp *c2P = cFirstPausedP; c2P < cEndP; ++c2P) {
      if (cP->layer == c2P->layer && _collided(&rectA[cP->collRect.idx], &rectA[c2P->collRect.idx])) {
        _alertCollisions(sP, rectA, cP, c2P);
      }
    }
  }

  // Finally, check mobile entities for collisions against static grid.
  U32 xStart, xEnd, y, yEnd, xN;
  U8 *tileP,*tileEndP;
  Rect_ entityRect;
  // for each moving component, check for soft- and hard-collisions against all active comps
  for (cP = cStartP; cP < cFirstPausedP; ++cP) {
    entityRect = rectA[cP->collRect.idx];
    xStart = entityRect.x >> TILE_DIM_LOG;
    xEnd   = xStart + ((entityRect.w + HALF_TILE_DIM) >> TILE_DIM_LOG);
    xN     = xEnd - xStart;
    y      = entityRect.y >> TILE_DIM_LOG;
    yEnd   = y + ((entityRect.h + HALF_TILE_DIM) >> TILE_DIM_LOG);
    U8 *gridA = xCollSysP->gridA + (N_TILES_PER_LAYER * cP->layer);
    // for current component, check against tilemap for highest-priority tile collision
    for (; y < yEnd; ++y) {
      tileP = gridA + (N_TILES_WIDE * y) + xStart;
      tileEndP = tileP + xN;
      for (; tileP < tileEndP; ++tileP) {
        if (*tileP)
          _alertCollision(sP, cP->entity, *tileP, 0);
      }
    }  // for current component, check against tilemap for highest-priority tile collision
  }  // for each moving component, check for soft- and hard-collisions against all active comps
}   // xCollisionRun()

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(Collision, COLLISION, collRect, 0);
