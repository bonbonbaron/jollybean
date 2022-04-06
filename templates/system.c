#include "xREPLACEME.h"

//======================================================
// Initialize REPLACEME's system.
//======================================================
Error xREPLACEMEIniSys(System *sP, void *sParamsP) {
  return SUCCESS;
	unused_(sParamsP);
  unused_(sP);
}

//======================================================
// Initialize xREPLACEME's components, which are Images.
//======================================================
Error xREPLACEMEIniComp(System *sP, void *compDataP, void *compDataSrcP) {
	if (!sP || !compDataP || !compDataSrcP)
		return E_BAD_ARGS;

  Error e = SUCCESS;
	XREPLACEME *xREPLACEMESysP = (XREPLACEME*) sP;
	XREPLACEMEComp *cP = (XREPLACEMEComp*) compDataP;
  XREPLACEMECompSrc *imgP = (XREPLACEMECompSrc*) compDataSrcP;

	return e;
}

Error xREPLACEMEProcessMessage(System *sP, Message *msgP) {
	unused_(sP);
	unused_(msgP);
	return SUCCESS;
}

XClrFuncDef_(REPLACEME) {
  unused_(sP);
  return SUCCESS;
}

XGetShareFuncDef_(REPLACEME) {
  XREPLACEME *xREPLACEMESysP = (XREPLACEME*) sP;
  Error e = SUCCESS;
  return e;
}

//======================================================
// REPLACEME activity
//======================================================
Error xREPLACEMERun(System *sP) {
	Error e = SUCCESS;

	XREPLACEMEComp *cP = (XREPLACEMEComp*) sP->cF;
	XREPLACEMEComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);

  // loop through elements and act on them here

	return e;
}

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(REPLACEME, 1, FLAGS_HERE);