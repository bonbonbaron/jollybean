#include "xAnim.h"

//======================================================
// Initialize Anim's system.
//======================================================
Error xAnimIniSys(System *sP, void *sParamsP) {
	unused_(sParamsP);
  unused_(sP);
  return SUCCESS;
}

//======================================================
// Initialize xAnim's components, which are Images.
//======================================================
Error xAnimIniCompElem(System *sP, void *compDataP, void *compDataSrcP) {
  unused_(sP);
  unused_(compDataP);
  unused_(compDataSrcP);
	return SUCCESS;
}

Error xAnimProcessMessage(System *sP, Message *msgP) {
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

XClrFuncDef_(Anim) {
  unused_(sP);
  return SUCCESS;
}

// TODO: does this break if we call this BEFORE add
XGetShareFuncDef_(Anim) {
  if (!sP->cF)
    return E_BAD_ARGS;
	XAnimComp *cP = (XAnimComp*) sP->cF;
  XAnimComp *cStartP = cP;
	XAnimComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);

  for (; cP < cEndP; ++cP)
    cP->srcRectP = (Rect_*) mapGet(shareMMP, xGetEntityByCompIdx(sP, cP - cStartP));

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
        *cP->srcRectP = cP->srcRectA[0];
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
