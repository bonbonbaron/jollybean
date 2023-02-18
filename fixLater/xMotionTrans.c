#include "xMotionTrans.h"

//======================================================
// Initialize MotionTrans's system.
//======================================================
Error xMotionTransIniSys(System *sP, void *sParamsP) {
	unused_(sParamsP);
  unused_(sP);
  return SUCCESS;
}

//======================================================
// Initialize xMotionTrans's components, which are Images.
//======================================================
Error xMotionTransIniCompElem(System *sP, void *compDataP, void *compDataSrcP) {
  unused_(sP);
  unused_(compDataP);
  unused_(compDataSrcP);
  return SUCCESS;
}

Error xMotionTransProcessMessage(System *sP, Message *msgP) {
  Error e = SUCCESS;
  // Get entity's map of switchable components.
  Map **switchMPP = (Map**) mapGet(sP->switchMPMP, msgP->attn);
  if (!switchMPP)
    e = E_BAD_KEY;
  Map *switchMP;
  if (!e) {
    switchMP = *switchMPP;
    if (!switchMP) 
      e = E_BAD_KEY;
  }
  if (!e) {
    // Get entity's map of components to switch to.
    void *compP = xGetCompPByEntity(sP, msgP->attn);
    if (compP) {
      void *tmpP = mapGet(switchMP, msgP->arg);
      if (tmpP)
        memcpy(compP, tmpP, sP->compSz - sizeof(Rect_*));  // don't copy over source rect pointer!
      else
        e = E_BAD_ARGS;
    }
  }
	return e;
}

XClrFuncDef_(MotionTrans) {
  unused_(sP);
  return SUCCESS;
}

#define RECT 1  // TODO
XGetShareFuncDef_(MotionTrans) {
  XMotionTrans *xMotionSysP = (XMotionTrans*) sP;
  Map *rectMP = (Map*) mapGet(shareMMP, RECT);
  if (!rectMP)
    return E_BAD_KEY;
  xMotionSysP->rectA = rectMP->mapA;
  return SUCCESS;
}

//======================================================
// MotionTrans activity
//======================================================
Error xMotionTransRun(System *sP) {
  XMotionTrans *xMotionSysP = (XMotionTrans*) sP;
	XMotionTransComp *cP = (XMotionTransComp*) sP->cF;
	XMotionTransComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);
  Rect_ *rectA = xMotionSysP->rectA;

  for (; cP < cEndP; cP++) {
    Rect_ *rectP = &rectA[cP->rectIdx];
    rectP->x += cP->x;
    rectP->y += cP->y;
  }

	return SUCCESS;
}

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(MotionTrans, MOTION_TRAN, FLAGS_HERE);
