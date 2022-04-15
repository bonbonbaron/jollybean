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
	if (!sP || !compDataP || !compDataSrcP)
		return E_BAD_ARGS;

  Error e = SUCCESS;
	XTimer *xTimerSysP = (XTimer*) sP;
	XTimerComp *cP = (XTimerComp*) compDataP;
  XTimerCompSrc *imgP = (XTimerCompSrc*) compDataSrcP;

	return e;
}

#define START_TIMER (1)
Error xTimerProcessMessage(System *sP, Message *msgP) {
  XTimerComp *cP = (XTimerComp*) xGetCompPByEntity(sP, msgP->attn);
  switch (msgP->cmd) {
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
  XTimer *xTimerSysP = (XTimer*) sP;
  Error e = SUCCESS;
  return e;
}

//======================================================
// Timer run function
//======================================================
#define TIMES_UP (1) //TODO
Error xTimerRun(System *sP) {
	Error e = SUCCESS;

	XTimerComp *cP = (XTimerComp*) sP->cF;
	XTimerComp *cStartP = cP;
	XTimerComp *cEndP = cP + frayGetFirstPausedIdx(sP->cF);

  while (cP < cEndP) {
    if (!--cP->timeLeft) {
      mailboxWrite(sP->outboxF, sP->cIdx2eA[cP - cStartP], TIMES_UP, 0, 0);
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
