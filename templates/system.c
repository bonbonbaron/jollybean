#include "xREPLACEME.h"

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
// Initialize REPLACEME's system.
//======================================================
#if 0
XIniSysFuncDefUnused_(REPLACEME);
#else
Error xREPLACEMEIniSys(System *sP, void *sParamsP) {
	unused_(sParamsP);
  unused_(sP);
  return SUCCESS;
}
#endif

#if 0
XIniSubcompFuncDefUnused_(REPLACEME);
#else
Error xREPLACEMEIniSubcomp(System *sP, void *compDataP, void *compDataSrcP) {
	if (!sP || !compDataP || !compDataSrcP)
		return E_BAD_ARGS;

  Error e = SUCCESS;
	XREPLACEME *xREPLACEMESysP = (XREPLACEME*) sP;
	XREPLACEMEComp *cP = (XREPLACEMEComp*) compDataP;
  XREPLACEMECompSrc *imgP = (XREPLACEMECompSrc*) compDataSrcP;

	return e;
}
#endif

#if 0
XGetShareFuncDefUnused_(REPLACEME);
#else
XGetShareFuncDef_(REPLACEME) {
  XREPLACEME *xREPLACEMESysP = (XREPLACEME*) sP;
  Error e = SUCCESS;
  return e;
}
#endif

#if 0
XProcMsgFuncDefUnused_(REPLACEME)
#else
Error xREPLACEMEProcessMessage(System *sP, Message *msgP) {
	unused_(sP);
	unused_(msgP);
	return SUCCESS;
}
#endif

//======================================================
// REPLACEME run function
//======================================================
Error xREPLACEMERun(System *sP) {
	Error e = SUCCESS;

	XREPLACEMEComp *cP = (XREPLACEMEComp*) sP->cF;
	XREPLACEMEComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);

  for (; cP < cEndP; cP++) {
    // do something on each element here
  }

	return e;
}

XClrFuncDef_(REPLACEME) {
  unused_(sP);
  return SUCCESS;
}

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(REPLACEME, 1, FLAGS_HERE);
