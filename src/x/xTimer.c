#include "x/xTimer.h"
#include "jb.h"

//======================================================
// Unused X functions
//======================================================
XIniSysFuncDefUnused_(Timer);
XMakeComponentsDefUnused_(Timer);
XPostMutateFuncDefUnused_(Timer);
XPostDeactivateFuncDefUnused_(Timer);

// Immutable gene body is a whole XTimerComp (timeFull + repeat; timeLeft is reset on start).
XConsumeGeneFuncDefDefault_(Timer);

// Activating a timer (by TIMER_START or built-in ACTIVATE) always starts it from the top.
XPostActivateFuncDef_(Timer) {
  assert( sP && changesP );
  XTimerComp *cP = &((XTimerComp*) sP->cF)[ changesP->newIdx ];
  if ( !cP->timeLeft ) {
    cP->timeLeft = cP->timeFull;
  }
}

XProcMsgFuncDef_(Timer) {
  assert( sP && msgP );
  XTimerComp *cP = (XTimerComp*) xGetCompPByEntity(sP, msgP->attn);
  assert( cP );
  switch (msgP->cmd) {
    // Pausing and stopping the timer are already implemented in built-in x.c message-processing.
    case TIMER_START:
      cP->timeLeft = cP->timeFull;
      xActivateComponentByEntity(sP, msgP->attn);
      break;
    case TIMER_SET_DURATION:
      cP->timeFull = msgP->arg;
      break;
    default:
      assert( 0 );
      break;
  }
}

//======================================================
// Timer run function
//======================================================
void xTimerRun(System *sP) {
  XTimerComp *cP = (XTimerComp*) sP->cF;
  XTimerComp *cEndP = cP + _frayGetFirstPausedIdx(sP->cF);
  Message *actionInboxF = NULL;

  for (; cP < cEndP; ++cP) {
    if (!cP->timeLeft || !--cP->timeLeft) {
      if (!actionInboxF) {
        actionInboxF = xGetInbox(ACTION);
      }
      mailboxWrite(actionInboxF, ACTION, xGetEntityByVoidComponentPtr(sP, cP), TIMER_TIME_UP, 0, NULL);
      if (cP->repeat) {
        cP->timeLeft = cP->timeFull;
      }
      else {
        xQueueDeactivate(sP, cP);  // deferred so we don't shuffle the fray mid-loop
      }
    }
  }
}

//======================================================
// System definition
//======================================================
X_no_mutations_(Timer, TIMER, FLG_NO_MUTATIONS_);
