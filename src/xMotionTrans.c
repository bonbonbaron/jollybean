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

XGetShareFuncDef_(MotionTrans) {
	XMotionTransComp *cP = (XMotionTransComp*) sP->cF;
  XMotionTransComp *cStartP = cP;
	XMotionTransComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);

  for (; cP < cEndP; ++cP)
    cP->shareRectP = (Rect_*) mapGet(shareMMP, xGetEntityByCompIdx(sP, cP - cStartP));

  return SUCCESS;
}

//======================================================
// MotionTrans activity
//======================================================
Error xMotionTransRun(System *sP) {
	Error e = SUCCESS;

	XMotionTransComp *cP = (XMotionTransComp*) sP->cF;
	XMotionTransComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);

  for (; cP < cEndP; cP++) {
    cP->shareRectP->x += cP->x;
    cP->shareRectP->y += cP->y;
  }

	return e;
}

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(MotionTrans, 1, FLAGS_HERE);
