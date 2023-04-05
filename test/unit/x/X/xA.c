#include "xA.h"
#include "data.h"

// TODO make this file as helpful as possible.
// TODO add xP casting in each USED function.
// TODO add necessary checks in each function first line.
// TODO add map-getting wherever relevant.
// TODO add nested map-getting wherever relevant.
// TODO don't be annoying by asssuming unused in USED functions bro. Build out a body at least!
// TODO Makefile and test initialization for jb script

/* HOW THIS FILE IS LAID OUT:
 *
 *  The functions below are ordered by xMaster's calls to them.
 *  That's so you can keep track of what will already have been done by each function call.
 */
//======================================================
// Initialize A's system.
//======================================================
#if 1
XIniSysFuncDefUnused_(A);
#else
Error xAIniSys(System *sP, void *sParamsP) {
	unused_(sParamsP);
  unused_(sP);
  return SUCCESS;
}
#endif

#if 1
XIniSubcompFuncDefUnused_(A);
#else
Error xAIniSubcomp(System *sP, void *compDataP, void *compDataSrcP) {
	if (!sP || !compDataP || !compDataSrcP)
		return E_BAD_ARGS;

  Error e = SUCCESS;
	XA *xASysP = (XA*) sP;
	XAComp *cP = (XAComp*) compDataP;
  XACompSrc *imgP = (XACompSrc*) compDataSrcP;

	return e;
}
#endif

#if 1
XGetShareFuncDefUnused_(A);
#else
XGetShareFuncDef_(A) {
  XA *xASysP = (XA*) sP;
  Error e = SUCCESS;
  return e;
}
#endif

#if 1
XProcMsgFuncDefUnused_(A);
#else
Error xAProcessMessage(System *sP, Message *msgP) {
	unused_(sP);
	unused_(msgP);
	return SUCCESS;
}
#endif

#if 1
XPostprocessCompsDefUnused_(A);
#else
Error XPostprocessComps(A) {
  unused_(sP);
}
#endif

//======================================================
// A run function
//======================================================
Error xARun(System *sP) {
	Error e = SUCCESS;

	XAComp *cP = (XAComp*) sP->cF;
	XAComp *cEndP = cP + _frayGetFirstPausedIdx(sP->cF);

  for (; cP < cEndP; cP++) {
    printf("A: Entity %d's component cF[%d] = {a = %d, b = %d, c = %d}\n", xGetEntityByVoidComponentPtr(sP, cP), cP - (XAComp*) sP->cF, cP->a, cP->b, cP->c);
  }

	return e;
}

#if 1
XClrFuncDefUnused_(A);
#else
XClrFuncDef_(A) {
  unused_(sP);
  return SUCCESS;
}
#endif

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(A, 1, FLAGS_HERE);
