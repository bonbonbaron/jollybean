#include "data.h"
#include "x.h"
#include "xA.h"

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
#if 0
XIniSysFuncDefUnused_(A);
#else
XIniSysFuncDef_(A) {
	unused_(sParamsP);
  unused_(sP);
  // TODO put good code here
  return SUCCESS;
}
#endif

#if 1
XIniSubcompFuncDefUnused_(A);
#else
XIniSubcompFuncDef_(A) {
	if (!sP || !entity || !subtype || !dataP) {
		return E_BAD_ARGS;
  }

  Error e = SUCCESS;
	XA *xASysP = (XA*) sP;
  // TODO put good code here

	return e;
}
#endif

#if 1
XGetShareFuncDefUnused_(A);
#else
XGetShareFuncDef_(A) {
  XA *xP = (XA*) sP;
  Error e = SUCCESS;
  // TODO put good code here
  return e;
}
#endif

#if 1
XPostprocessCompsDefUnused_(A);
#else
XPostprocessCompsDef_(A) {
  // TODO put good code here
  unused_(sP);
  return SUCCESS;
}
#endif

#if 0
XPostMutateFuncDefUnused_(A);
#else
XPostMutateFuncDef_(A) {
  XAComp* _cP = cP;
  _cP->a = _cP->c & 0xff;
  _cP->b = _cP->c * 2;
  _cP->d = (double) _cP->c;
  return SUCCESS;
}
#endif

#if 0
XProcMsgFuncDefUnused_(A);
#else
XProcMsgFuncDef_(A) {
	unused_(sP);
	unused_(msgP);
	return SUCCESS;
}
#endif

//======================================================
// A run function
//======================================================
Error xARun(System *sP) {
	Error e = SUCCESS;
  // TODO put better code here

	XAComp *cP = (XAComp*) sP->cF;
	XAComp *cEndP = cP + _frayGetFirstPausedIdx(sP->cF);

  for (; cP < cEndP; cP++) {
    // do something on each element here
  }

	return e;
}

#if 0
XClrFuncDefUnused_(A);
#else
XClrFuncDef_(A) {
  unused_(sP);
  // TODO put good code here
  return SUCCESS;
}
#endif

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
#define CHANGE_THIS (0)
X_(A, 1, c, FLAGS_HERE);
