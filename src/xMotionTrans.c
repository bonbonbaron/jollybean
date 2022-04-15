#include "xMotionTrans.h"

//======================================================
// Initialize MotionTrans's system.
//======================================================
Error xMotionTransIniSys(System *sP, void *sParamsP) {
  return SUCCESS;
	unused_(sParamsP);
  unused_(sP);
}

//======================================================
// Initialize xMotionTrans's components, which are Images.
//======================================================
Error xMotionTransIniComp(System *sP, void *compDataP, void *compDataSrcP) {
  unused_(sP);
  unused_(compDataP);
  unused_(compDataSrcP);
  return SUCCESS;
}

Error xMotionTransProcessMessage(System *sP, Message *msgP) {
  Error e = SUCCESS;
  // Get the entity's switch function. 
  // (Keep in mind this function can also just grab into an array if you tell it to.)
  XSwitchCompU switchU = (XSwitchCompU) mapGet(sP->switchMP, msgP->attn);
  if (switchU) {
    void* compP = xGetCompPByEntity(sP, msgP->attn);
    if (compP) {
      void *tmpP = switchU(msgP->arg);
      if (tmpP)
        memcpy(compP, tmpP, sP->compSz - sizeof(Rect_*));  // don't copy over source rect pointer!
      else
        e = E_BAD_ARGS;
    }
    else
      e = E_BAD_KEY;
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
X_(MotionTrans, 1, FLAGS_HERE);
