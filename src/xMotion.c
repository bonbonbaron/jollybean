#include "xAnim.h"

//======================================================
// Initialize Anim's system.
//======================================================
Error xAnimIniSys(System *sP, void *sParamsP) {
  return SUCCESS;
	unused_(sParamsP);
  unused_(sP);
}

//======================================================
// Initialize xAnim's components, which are Images.
//======================================================
Error xAnimIniComp(System *sP, void *compDataP, void *compDataSrcP) {
	if (!sP || !compDataP || !compDataSrcP)
		return E_BAD_ARGS;

  Error e = SUCCESS;
	XAnim *xAnimSysP = (XAnim*) sP;
	XAnimComp *cP = (XAnimComp*) compDataP;
  XAnimCompSrc *imgP = (XAnimCompSrc*) compDataSrcP;

	return e;
}

Error xAnimProcessMessage(System *sP, Message *msgP) {
	unused_(sP);
	unused_(msgP);
	return SUCCESS;
}

XClrFuncDef_(Anim) {
  unused_(sP);
  return SUCCESS;
}

XGetShareFuncDef_(Anim) {
  XAnim *xAnimSysP = (XAnim*) sP;
  Error e = SUCCESS;
  return e;
}

//======================================================
// Anim activity
//======================================================
Error xAnimRun(System *sP) {
	Error e = SUCCESS;

	XAnimComp *cP = (XAnimComp*) sP->cF;
	XAnimComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);

  // loop through elements and act on them here

	return e;
}

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(Anim, 1, FLAGS_HERE);
