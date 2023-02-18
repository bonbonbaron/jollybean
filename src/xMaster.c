#include "xMaster.h"
#include "data.h"
#include "x.h"

X_(Master, 1, FLG_NO_CF_SRC_A_); 

// =====================================================================
// Initialize the Master system.
// =====================================================================
Error xMasterIniSys(System *sP, void *sParamsP) {
  XMaster *xMasterSysP = (XMaster*) sP;
  // Check args
  if (!sP || !sParamsP)
    return E_BAD_ARGS;

  XMasterIniSysPrms *xMasterIniSysPrmsP = (XMasterIniSysPrms*) sParamsP;

  if (!xMasterIniSysPrmsP->biomeP || !xMasterIniSysPrmsP->xSysPA)
    return E_BAD_ARGS;

  xMasterSysP->biomeP = xMasterIniSysPrmsP->biomeP;
 
  // Initialize the GUI through whatever library we're using.
  Error e = guiNew(&xMasterSysP->windowP, &xMasterSysP->rendererP);

  // Add subsystems to master system. Don't add Go yet.
  U8 nSystemsExceptGoSys = xMasterIniSysPrmsP->nXSystems - 1;
  if (xMasterIniSysPrmsP) 
    for (U32 i = 0; !e && i < nSystemsExceptGoSys; i++) 
      if (xMasterIniSysPrmsP->xSysPA[i])
        e = xAddComp(sP, xMasterIniSysPrmsP->xSysPA[i]->id, sP->id, (void**) &xMasterIniSysPrmsP->xSysPA[i], NULL);

  XGo *xGoP = NULL;
  // Add Go System to Master's components.
  if (!e && xMasterIniSysPrmsP->behaviorSysP) {
    xGoP = (XGo*) xMasterIniSysPrmsP->behaviorSysP;
    e = xAddComp(sP, xGoP->system.id, sP->id, (void**) &sGoP, NULL);
  }

  // Extract biome.
  if (!e)
    e = biomeExtract(xMasterSysP->biomeP, sP, sGoP, &xMasterSysP->sharedMMP, xMasterSysP->windowP, xMasterSysP->rendererP, &xGoP->hivemindMP, xMasterIniSysPrmsP->nXSystemsMax);

  // Kick off the scene! 
  // (In XGo, "attn == 0" is interpreted as "trigger everybody's response to the event you passed in.")
  if (!e)
    mailboxWrite(xGoP->system.inboxF, 0, 0, SCENE_START, 0);

  return e;
}

Error xMasterProcessMessage(System *sP, Message *msgP) {
  if (msgP->address != MASTER_) {
    XMasterComp *cP = (XMasterComp*) xGetCompPByEntity(sP, msgP->address);
    if (!cP)
      return E_MAILBOX_BAD_RECIPIENT;
    Error e = mailboxForward((*cP)->inboxF, msgP);
    // Wake up sleeping systems when someone sends them messages.
    // However, messages to XGo are shortcircuited by setting subsystems' outboxes to its inbox.
    if (!e)
      if (!frayElemIsActive((*cP)->cF, cP - (XMasterComp*) sP->cF))
        frayActivate(sP->cF, cP - (XMasterComp*) sP->cF);
  }
  return SUCCESS; // TODO impement maste logi c ier
}

XIniCompElemFuncDefUnused_(Master);
XGetShareFuncDefUnused_(Master);
XClrFuncDefUnused_(Master);

/* xIni() initializes the parent system as well as its children. */
Error xMasterIni(XMaster *xMasterSysP, System *goSysP, System **sPA, U16 nXSystems, Key nXSystemsMax, Biome *biomeP) {
  if (!sPA || nXSystems < 1 || !biomeP)
    return E_BAD_ARGS;

  XMasterIniSysPrms xMasterSysIniPrms = {
    .nXSystemsMax = nXSystemsMax + 1, // + 1 for "go"-system
    .nXSystems = nXSystems,   // TODO needs a clearer names as it doesn't account for Go system
    .biomeP = biomeP,
    .xSysPA = sPA,
    .behaviorSysP = goSysP
  };

  return xIniSys(&xMasterSysP->system, xMasterSysIniPrms.nXSystems, (void*) &xMasterSysIniPrms);
}

Error xMasterRun(System *sP) {
  Error e = SUCCESS;
  XMasterComp *cIniP =sP->cF;
  XMasterComp *cP = cIniP;
  XMasterComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);
  while (cP < cEndP) {
    e = xRun(*cP);  // cP is a pointer to XMasterComp, which itself is also a pointer.
    // Put idle subsystems to sleep (last condition keeps Go element active).
    if (!e && cP != sP->cF && frayGetFirstInactiveIdx((*cP)->cF)) {  
      xDeactivateComponentByIdx(sP, cP - cIniP);
      cEndP--;
    } else
      cP++;
  }
  return e;
}
