#include "xAnim.h"

//======================================================
// Initialize Anim's system.
//======================================================
Error xAnimIniSys(System *sP, void *sParamsP) {
  return SUCCESS;
	unused_(sParamsP);
  unused_(sP);
}

//======================================================
// Initialize xAnim's components, which are Images.
//======================================================
Error xAnimIniComp(System *sP, void *compDataP, void *compDataSrcP) {
  unused_(sP);
  unused_(compDataP);
  unused_(compDataSrcP);
	return SUCCESS;
}

Error xAnimProcessMessage(System *sP, Message *msgP) {
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

XClrFuncDef_(Anim) {
  unused_(sP);
  return SUCCESS;
}

XGetShareFuncDef_(Anim) {
  if (!sP->cF)
    return E_BAD_ARGS;
	XAnimComp *cP = (XAnimComp*) sP->cF;
  XAnimComp *cStartP = cP;
	XAnimComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);

  for (; cP < cEndP; ++cP)
    cP->shareRectP = (Rect_*) mapGet(shareMMP, xGetEntityByCompIdx(sP, cP - cStartP));

  return SUCCESS;
}

//======================================================
// Anim activity
//======================================================
Error xAnimRun(System *sP) {
	Error e = SUCCESS;

	XAnimComp *cP = (XAnimComp*) sP->cF;
	XAnimComp *cStartP = cP;
	XAnimComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);

  // loop through elements and act on them here
  while (cP < cEndP) {
    if (!(--cP->timeLeft)) {
      if (cP->repeat) {
        cP->timeLeft = cP->timeA[cP->currIdx = 0];
        *cP->shareRectP = cP->srcRectA[0];
      } else {
        xDeactivateComponentByIdx(sP, cP - cStartP);
        --cEndP;
      }
    } else 
      ++cP;
  }

	return e;
}

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(Anim, ANIMATION, FLAGS_HERE);
