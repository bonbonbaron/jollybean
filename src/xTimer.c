#include "xTimer.h"
#include "data.h"
#include "x.h"

//======================================================
// Initialize Timer's system.
//======================================================
Error xTimerIniSys(System *sP, void *sParamsP) {
	unused_(sParamsP);
  unused_(sP);
  return SUCCESS;
}

//======================================================
// Initialize xTimer's components, which are Images.
//======================================================
Error xTimerIniComp(System *sP, void *compDataP, void *compDataSrcP) {
  unused_(sP);
  unused_(compDataP);
  unused_(compDataSrcP);
	return SUCCESS;
}

Error xTimerProcessMessage(System *sP, Message *msgP) {
  XTimerComp *cP = (XTimerComp*) xGetCompPByEntity(sP, msgP->attn);
  switch (msgP->cmd) {
    // Pausing and stopping the timer are already implemented in built-in x.c message-processing.
    case START_TIMER:
      cP->timeLeft = cP->timeFull;
      xActivateComponentByEntity(sP, msgP->attn);
      break;
    default:
      return E_BAD_ARGS;
  }
	return SUCCESS;
}

XClrFuncDef_(Timer) {
  unused_(sP);
  return SUCCESS;
}

XGetShareFuncDef_(Timer) {
  unused_(sP);
  unused_(shareMMP);
  return SUCCESS;
}

//======================================================
// Timer run function
//======================================================
Error xTimerRun(System *sP) {
	Error e = SUCCESS;

	XTimerComp *cP = (XTimerComp*) sP->cF;
	XTimerComp *cStartP = cP;
	XTimerComp *cEndP = cP + frayGetFirstPausedIdx(sP->cF);

  while (cP < cEndP) {
    if (!--cP->timeLeft) {
      mailboxWrite(sP->outboxF, sP->cIdx2eA[cP - cStartP], TIME_UP, 0, 0);
      if  (cP->repeat) {
        cP->timeLeft = cP->timeFull;
        ++cP;
      }
      else {
        xDeactivateComponentByIdx(sP, cP - cStartP);
        --cEndP;
      }
    }
    else
      ++cP;
  }

	return e;
}

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(Timer, 1, FLAGS_HERE);
