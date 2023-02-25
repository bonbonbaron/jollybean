#include "xAnim.h"

Error xAnimProcessMessage(System *sP, Message *msgP) {
  Map *entityAnimationMP = NULL;
  Error e = mapGetNestedMapP(sP->mutationMPMP, msgP->attn, &entityAnimationMP);
  if (!e) {
    // Get entity's map of components to switch to.
    void *compP = xGetCompPByEntity(sP, msgP->attn);
    if (compP) {
      AnimStrip *animStripP = mapGet(entityAnimationMP, msgP->arg);  // arg = key of animation strip
      if (tmpP) {
        memcpy(compP, tmpP, sP->compSz - sizeof(Rect_*));  // don't copy over source rect pointer!
      }
      else {
        e = E_BAD_ARGS;
      }
    }
  }
	return e;
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
