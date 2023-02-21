#include "xGo.h"

XIniSysFuncDefUnused_(Go);
XIniSubcompFuncDefUnused_(Go);

XClrFuncDef_(Go) {
  XGo *xGoSysP = (XGo*) sP;
  hivemindDel(&xGoSysP->hivemindMP);
  return SUCCESS;
}

inline static U8 _isHigherPriority(U8 newPriority, U8 existingPriority) {
  return newPriority > existingPriority;
}

static Error _triggerIndividual(XGo *xGoSysP, Message *msgP) {
  System *sP = &xGoSysP->system;
  // Get entity's Go system component.
  XGoComp *cP = xGetCompPByEntity(sP, msgP->attn);
  if (!cP)
    return E_BAD_KEY;
  // Get the personality, one per entity.
  Map **activityMPP = (Map**) mapGet(sP->mutationMPMP, msgP->attn);
  if (!activityMPP || !(*activityMPP))
    return E_BAD_KEY;
  // Get the tree that *would* be triggered if the entity is inactive or doing something less important. 
  XGoComp *cNewP = (XGoComp*) mapGet(*activityMPP, msgP->arg);

  if (cNewP) {
    // Queue new action if it's higher priority than old or entity's inactive.
    if (!frayElemIsActive(sP->cF, msgP->attn) || 
        _isHigherPriority(cNewP->priority, cP->priority))
      *cP = *cNewP;
    // Activate activity.
    xActivateComponentByEntity(sP, msgP->attn);
  }

  return SUCCESS;
}

static Error _triggerHivemind(XGo *xGoSysP, Message *msgP) {
  assert(msgP);
  Error e = SUCCESS;
  // Get the Go system's hivemind array for the given stimulus.
  Entity *eA = (Entity*) mapGet(xGoSysP->hivemindMP, msgP->arg);
  if (!eA)
    return E_BAD_KEY;
  // Init pointers.
  assert(eA);
  Entity *eP, *eEndP;
  arrayIniPtrs((void*) eA, (void**) &eP, (void**) &eEndP, -1);
  // Forward the message to each individual in the hivemind.
  for (; !e && eP < eEndP; eP++) {
    msgP->attn = *eP;
    e = _triggerIndividual(xGoSysP, msgP);
  }
  return e;
}

XGetShareFuncDefUnused_(Go);

// Entity acts on message if it's more urgent than its current activity.
XProcMsgFuncDef_(Go) {
  Error e;
  XGo *xGoSysP = (XGo*) sP;
  if (msgP->attn)
    e = _triggerIndividual(xGoSysP, msgP);
  else
    e = _triggerHivemind(xGoSysP, msgP);
  return e;
}

Error xGoRun(System *sP) {
  XGoComp *cP = sP->cF;
  XGoComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);
  for (; cP < cEndP; cP++) 
    btRun(cP->btP, cP->btStatP, cP->bbMP, sP->outboxF);

  return SUCCESS;
}

X_(Go, 2, FLG_NO_CF_SRC_A_);
