#include "xBlah.h"

//======================================================
// Initialize Blah's system.
//======================================================
Error xBlahIniSys(System *sP, void *sParamsP) {
	unused_(sParamsP);
  unused_(sP);
  return SUCCESS;
}

//======================================================
// Initialize xBlah's components, which are Images.
//======================================================
Error xBlahIniComp(System *sP, void *compDataP, void *compDataSrcP) {
	if (!sP || !compDataP || !compDataSrcP)
		return E_BAD_ARGS;

  Error e = SUCCESS;
	XBlah *xBlahSysP = (XBlah*) sP;
	XBlahComp *cP = (XBlahComp*) compDataP;
  XBlahCompSrc *imgP = (XBlahCompSrc*) compDataSrcP;

	return e;
}

Error xBlahProcessMessage(System *sP, Message *msgP) {
	unused_(sP);
	unused_(msgP);
	return SUCCESS;
}

XClrFuncDef_(Blah) {
  unused_(sP);
  return SUCCESS;
}

XGetShareFuncDef_(Blah) {
  XBlah *xBlahSysP = (XBlah*) sP;
  Error e = SUCCESS;
  return e;
}

//======================================================
// Blah run function
//======================================================
Error xBlahRun(System *sP) {
	Error e = SUCCESS;

	XBlahComp *cP = (XBlahComp*) sP->cF;
	XBlahComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);

  for (; cP < cEndP; cP++) {
    // do something on each element here
  }

	return e;
}

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(Blah, 1, FLAGS_HERE);
