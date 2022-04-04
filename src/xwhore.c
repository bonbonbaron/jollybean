#include "xwhore.h"

//======================================================
// Initialize whore's system.
//======================================================
Error xwhoreIniSys(System *sP, void *sParamsP) {
  return SUCCESS;
	unused_(sParamsP);
  unused_(sP);
}

//======================================================
// Initialize xwhore's components, which are Images.
//======================================================
Error xwhoreIniComp(System *sP, void *compDataP, void *compDataSrcP) {
	if (!sP || !compDataP || !compDataSrcP)
		return E_BAD_ARGS;

  Error e = SUCCESS;
	Xwhore *xwhoreSysP = (Xwhore*) sP;
	XwhoreComp *cP = (XwhoreComp*) compDataP;
  XwhoreCompSrc *imgP = (XwhoreCompSrc*) compDataSrcP;

	return e;
}

Error xwhoreProcessMessage(System *sP, Message *msgP) {
	unused_(sP);
	unused_(msgP);
	return SUCCESS;
}

XClrFuncDef_(whore) {
  unused_(sP);
  return SUCCESS;
}

XGetShareFuncDef_(whore) {
  Xwhore *xwhoreSysP = (Xwhore*) sP;
  Error e = SUCCESS;
  return e;
}

//======================================================
// whore activity
//======================================================
Error xwhoreRun(System *sP) {
	Error e = SUCCESS;

	XwhoreComp *cP = (XwhoreComp*) sP->cF;
	XwhoreComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);

  // loop through elements and act on them here

	return e;
}

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(whore, 1, FLAGS_HERE);
