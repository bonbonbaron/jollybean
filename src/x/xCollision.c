#include "x/xCollision.h"
#include "jb.h"
#include "data/share.h"

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
// Initialize xCollision's system.
//======================================================
XIniSysFuncDef_(Collision) {
  XCollision *xP = (XCollision*) sP;
  xP->cameraEntity   = 0;
  xP->cameraIsMoving = FALSE;
  xP->collisionA     = NULL;
  xP->worldWidth     = 0;
  xP->worldHeight    = 0;
  xP->rectA          = NULL;
  xP->gridA          = NULL;  // static tile grid; stays NULL until a tilemap gene supplies one
}

//======================================================
// Consume collision genes
//   IMMUTABLE: body is the collision type (Key).
//   MUTABLE:   CollRect mutations swapped in as animation frames change.
//======================================================
XConsumeGeneFuncDef_(Collision) {
  assert( sP && geneP );
  XCollisionComp *cP = (XCollisionComp*) xGetCompPByEntity( sP, entity );
  assert( cP );
  cP->entity = entity;
  switch ( geneP->class ) {
    case IMMUTABLE:
      assert( ((const ImmutableGene*) geneP)->bodyP );
      cP->type = *((Key*) ((const ImmutableGene*) geneP)->bodyP);
      break;
    case MUTABLE:
    case INTRACOMPOSITE:
      xConsumeGeneDefault( sP, entity, geneP );
      break;
    default:
      assert( FALSE );
      break;
  }
}

//======================================================
// Hook components up to shared rects and z-heights (z-height doubles as collision layer).
//======================================================
XMakeComponentsDef_(Collision) {
  XCollision *xP = (XCollision*) sP;
  Map *rectMP = shareGetMap( DST_RECT );
  Map *zHeightMP = shareGetMap( Z_HEIGHT );
  assert( rectMP );
  xP->rectA = (Rect_*) rectMP->mapA;
  XCollisionComp *cP = (XCollisionComp*) sP->cF;
  XCollisionComp *cStartP = cP;
  XCollisionComp *cEndP = cP + *_frayGetFirstEmptyIdxP(sP->cF);
  Key *layerP;
  for (; cP < cEndP; ++cP) {
    cP->entity = sP->cIdx2eA[cP - cStartP];
    assert( mapHasKey( rectMP, cP->entity ) );
    cP->collRect.idx = mapGetIndex(rectMP, cP->entity);
    layerP = zHeightMP ? (Key*) mapGet(zHeightMP, cP->entity) : NULL;
    cP->layer = layerP ? *layerP : 0;
  }
}

XProcMsgFuncDef_(Collision) {
  assert(sP && msgP);
  XCollision *xP = (XCollision*) sP;
  XCollisionComp *cP;
  switch (msgP->cmd) {
    case COLL_CHANGE_LAYER:
    case MSG_LAYER_CHANGED:
      cP = xGetCompPByEntity(sP, msgP->attn);
      assert( cP );
      cP->layer = msgP->arg;
      break; 
    case COLL_SET_CAMERA:
      xP->cameraEntity = msgP->attn;
      break;
    case COLL_CAMERA_STARTED_MOVING:
      xP->cameraIsMoving = TRUE;
      break;
    case COLL_CAMERA_STOPPED_MOVING:
      xP->cameraIsMoving = FALSE;
      break;
    // Onscreen still entities sit in the paused section; moving ones are active.
    case MSG_MOTION_STARTED:
      xUnpauseComponentByEntity(sP, msgP->attn);
      break;
    case MSG_MOTION_STOPPPED:
      xPauseComponentByEntity(sP, msgP->attn);
      break;
    default:
      break;
  }
}

inline static Bln _collided(const register Rect_ *r1P, const register Rect_ *r2P) {
  return r1P->y        < r2P->y + r2P->h &&  
         r1P->y + r1P->h > r2P->y        && 
         r1P->x        < r2P->x + r2P->w &&
         r1P->x + r1P->w > r2P->x;        
}

#define CAMERA_COLLISION_TYPE 3
#define TILE_DIM_LOG 3
#define HALF_TILE_DIM 4
inline static void _alertCollision(System *sP, Entity entity, Key collType, Entity otherEntity) {
  // Collisions drive behavior, so the Action system is the listener:
  //   cmd = trigger, arg = other entity, attachment = other party's collision type.
  Generic attachment = { .u32 = collType };
  mailboxWrite(xGetInbox(ACTION), ACTION, entity, COLL_COLLISION, otherEntity, &attachment);
}

inline static void _alertAntiCollision(System *sP, Entity entity, Entity cameraEntity) {
  Generic attachment = { .u32 = CAMERA_COLLISION_TYPE };
  mailboxWrite(xGetInbox(ACTION), ACTION, entity, COLL_ANTICOLLISION, cameraEntity, &attachment);
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
  if (!rectA)
    return;
  XCollisionComp *camCP = (XCollisionComp*) xGetCompPByEntity(sP, xCollSysP->cameraEntity);
  if (!camCP)
    return;  // no camera registered yet (see COLL_SET_CAMERA)
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
  if (!xCollSysP->gridA)
    return;
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
