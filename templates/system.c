#include "data.h"
#include "x.h"
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
XIniSysFuncDef_(REPLACEME) {
	unused_(sParamsP);
  unused_(sP);
  // TODO put good code here
  return SUCCESS;
}
#endif

#if 0
XIniSubcompFuncDefUnused_(REPLACEME);
#else
XIniSubcompFuncDef_(REPLACEME) {
	if (!sP || !entity || !subtype || !dataP) {
		return E_BAD_ARGS;
  }

  Error e = SUCCESS;
	XREPLACEME *xREPLACEMESysP = (XREPLACEME*) sP;
  // TODO put good code here

	return e;
}
#endif

#if 0
XGetShareFuncDefUnused_(REPLACEME);
#else
XGetShareFuncDef_(REPLACEME) {
  XREPLACEME *xP = (XREPLACEME*) sP;
  Error e = SUCCESS;
  // TODO put good code here
  return e;
}
#endif

#if 0
XPostprocessCompsDefUnused_(REPLACEME);
#else
XPostprocessCompsDef_(REPLACEME) {
  // TODO put good code here
  unused_(sP);
  return SUCCESS;
}
#endif

#if 0
XPostMutateFuncDefUnused_(REPLACEME);
#else
XPostMutateFuncDef_(REPLACEME) {
  unused_(sP);
  unused_(cP);
  return SUCCESS;
}
#endif

#if 0
XProcMsgFuncDefUnused_(REPLACEME);
#else
XProcMsgFuncDef_(REPLACEME) {
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
  // TODO put better code here

	XREPLACEMEComp *cP = (XREPLACEMEComp*) sP->cF;
	XREPLACEMEComp *cEndP = cP + _frayGetFirstPausedIdx(sP->cF);

  for (; cP < cEndP; cP++) {
    // do something on each element here
  }

	return e;
}

#if 0
XClrFuncDefUnused_(REPLACEME);
#else
XClrFuncDef_(REPLACEME) {
  unused_(sP);
  // TODO put good code here
  return SUCCESS;
}
#endif

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(REPLACEME, 1, 0, FLAGS_HERE);
