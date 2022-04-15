#include "xCollision.h"
#include "data.h"
#include "xControl.h"

// Based on the "Performance Comparisons" link, we use brute force until
// the number of moving elements exceeds 200. When it does, we switch to the box-
// intersect algorithm found in link to the paper. 
// ================================================
// Performance Comparisons: https://0fps.net/2015/01/23/collision-detection-part-3-benchmarks/
// Github:                  https://github.com/mikolalysenko/box-intersect
// Paper:                   https://pub.ist.ac.at/~edels/Papers/2002-J-01-FastBoxIntersection.pdf
// ================================================


//======================================================
// Initialize Collision's system.
//======================================================
Error xCollisionIniSys(System *sP, void *sParamsP) {
  return SUCCESS;
	unused_(sParamsP);
  unused_(sP);
}

//======================================================
// Initialize xCollision's components, which are Images.
//======================================================
Error xCollisionIniComp(System *sP, void *compDataP, void *compDataSrcP) {
	if (!sP || !compDataP || !compDataSrcP)
		return E_BAD_ARGS;

  Error e = SUCCESS;
	XCollision *xCollisionSysP = (XCollision*) sP;
	XCollisionComp *cP = (XCollisionComp*) compDataP;
  XCollisionCompSrc *imgP = (XCollisionCompSrc*) compDataSrcP;

	return e;
}

Error xCollisionProcessMessage(System *sP, Message *msgP) {
	unused_(sP);
	unused_(msgP);
	return SUCCESS;
}

XClrFuncDef_(Collision) {
  unused_(sP);
  return SUCCESS;
}

XGetShareFuncDef_(Collision) {
  XCollision *xCollisionSysP = (XCollision*) sP;
  Error e = SUCCESS;
  return e;
}

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
inline static Error _alertCollision(System *sP, Entity entity, Key collType, Entity otherEntity) {
  return mailboxWrite(sP->outboxF, entity, COLLISION, collType, otherEntity);
}

inline static Error _alertAntiCollision(System *sP, Entity entity, Entity cameraEntity) {
  return mailboxWrite(sP->outboxF, entity, ANTICOLLISION, CAMERA_COLLISION_TYPE, cameraEntity);
}

static Error _alertCollisions(System *sP, Rect_ *rectA, XCollisionComp *c1P, XCollisionComp *c2P) {
  Error e = _alertCollision(sP, c1P->entity, c2P->type, c2P->entity);
  if (!e)
    return _alertCollision(sP, c2P->entity, c1P->type, c1P->entity);
  return e;
}

//======================================================
// Collision run function
//======================================================

Error xCollisionRun(System *sP) {
  Error e = SUCCESS;
  XCollisionComp *cP = sP->cF;
  XCollisionComp *cStartP = cP;
  XCollisionComp *cFirstInactiveP = &cP[frayGetFirstInactiveIdx(sP->cF)];
  XCollisionComp *cEndP = cP + arrayGetNElems(sP->cF);
  XCollisionComp *cFirstPausedP = &cP[frayGetFirstPausedIdx(sP->cF)];

  XCollision *xCollSysP = (XCollision*) sP;
  Rect_ *rectA = xCollSysP->rectA;
  // Get camera.
  XCollisionComp *camCP = (XCollisionComp*) xGetCompPByEntity(sP, xCollSysP->cameraEntity);
  Rect_ camRect = rectA[camCP->rectIdx];
  Entity cameraEntity = camCP->entity;
  // You'll need this later.
  const U32 N_TILES_WIDE = xCollSysP->worldWidth >> TILE_DIM_LOG;
  const U32 N_TILES_HIGH = xCollSysP->worldHeight >> TILE_DIM_LOG;
  const U32 N_TILES_PER_LAYER = N_TILES_HIGH * N_TILES_WIDE;

  // First check onscreen moving elems for anti-collisions in case someone left the screen.
  while (cP < cFirstPausedP) {
    if (!_collided(&camRect, &rectA[cP->rectIdx])) {
      xPauseComponentByEntity(sP, cP->entity);
      e = _alertAntiCollision(sP, cP->entity, cameraEntity); // Active elems deactivate in two swaps
      --cFirstPausedP;                                       // when there are paused elems between. 
      if (e)
        break;
    }
    else 
      ++cP;
  }
  // Second, if camera's in motion, check paused elems (if any) for anti-collisions. 
  if (!e && xCollSysP->cameraIsMoving) {
    while (cP < cFirstInactiveP) {
      if (!_collided(&camRect, &rectA[cP->rectIdx])) {
        xDeactivateComponentByEntity(sP, cP->entity);  // paused elems deactivate in one swap
        --cFirstInactiveP;
      }
      else
        ++cP;
    }
    // Then check everyone else for camera collisions.
    for (cP = cFirstInactiveP; cP < cEndP; ++cP) 
      if (_collided(&camRect, &rectA[cP->rectIdx])) {
        e = _alertCollisions(sP, rectA, cP, camCP);
        if (e)
          break;
      }
  }
  // Always test onscreen moving entities for collisions with other onscreen entities.
  if (!e)
    for (cP = cStartP; cP < cFirstPausedP; ++cP)
      for (XCollisionComp *c2P = cFirstPausedP; c2P < cEndP; ++c2P)
        if (cP->layer == c2P->layer && _collided(&rectA[cP->rectIdx], &rectA[c2P->rectIdx]))
          _alertCollisions(sP, rectA, cP, c2P);
 
  // Finally, check mobile entities for collisions against static grid.
  U32 xStart, xEnd, y, yEnd, xN;
  U8 *tileP,*tileEndP;
  Rect_ entityRect;
  if (!e)
    for (cP = cStartP; cP < cFirstPausedP; ++cP) {
      entityRect = rectA[cP->rectIdx];
      xStart = entityRect.x >> TILE_DIM_LOG;
      xEnd   = xStart + ((entityRect.w + HALF_TILE_DIM) >> TILE_DIM_LOG);
      xN     = xEnd - xStart;
      y      = entityRect.y >> TILE_DIM_LOG;
      yEnd   = y + ((entityRect.h + HALF_TILE_DIM) >> TILE_DIM_LOG);
      U8 *gridA = xCollSysP->gridA + (N_TILES_PER_LAYER * cP->layer);
      for (; y < yEnd; ++y) {
        tileP = gridA + (N_TILES_WIDE * y) + xStart;
        tileEndP = tileP + xN;
        for (; tileP < tileEndP; ++tileP) {
          if (*tileP)
            _alertCollision(sP, cP->entity, *tileP, 0);
        }
      }
    }

  return e;
}

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(Collision, 1, FLAGS_HERE);
