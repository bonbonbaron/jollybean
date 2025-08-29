#include "x/xMotion.h"
#include "data/share.h"

/* REQUIREMENTS
 *
 *  Mutations for different directions' velocities
 *  Shared destination rectangle for each entity
 */

//======================================================
// Initialize Motion's system.
//======================================================
XIniSysFuncDefUnused_(Motion);
XConsumeGeneFuncDefUnused_(Motion);
XPostActivateFuncDefUnused_(Motion);
XPostDeactivateFuncDefUnused_(Motion);

XPostprocessCompsDef_(Motion) {
  assert(sP);
  XMotion *xP = (XMotion*) sP;
  xP->dstRectMP = shareGetMap( DST_RECT );
  Entity *entityP = sP->cIdx2eA;
  Entity *entityEndP = entityP + arrayGetNElems(sP->cIdx2eA);

  // Populate each motion system component. Start out with zero velocity and let action system mutate it later.
  for (XMotionComp *cP; entityP < entityEndP; ++entityP) {
    cP = (XMotionComp*) xGetCompPByEntity(sP, *entityP);
    if (cP) {
      cP->dstRectP = (Rect_*) mapGet(xP->dstRectMP, *entityP);
    }
    assert (cP->dstRectP);
  }
}

XPostMutateFuncDefUnused_(Motion);
XProcMsgFuncDefUnused_(Motion);

//======================================================
// Motion run function
//======================================================
void xMotionRun(System *sP) {
  assert(sP);

	XMotionComp *cP = (XMotionComp*) sP->cF;
	XMotionComp *cEndP = cP + _frayGetFirstPausedIdx(sP->cF);

  // Operate on all the active elements.
  for (; cP < cEndP; cP++) {
    cP->dstRectP->x += cP->velocity.x;
    cP->dstRectP->y += cP->velocity.y;
  }
}

//======================================================
// System definition
//======================================================
X_(Motion, MOTION, velocity, 0);
