#include "xREPLACEME.h"

// TODO add map-getting wherever relevant.
// TODO add nested map-getting wherever relevant.

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
  assert (sP && sParamsP);
  XREPLACEME *xP = (XREPLACEME*) sP;
  U32 nComponents = xGetNComps(sP);
  // You may want to initialize whatever specialized maps, arrays, etc. the system owns here.
  // Hint: You most likely need nComponents to allocate them.
}
#endif

#if 0
XIniSubcompFuncDefUnused_(REPLACEME);
#else
XIniSubcompFuncDef_(REPLACEME) {
	assert (sP && entity && subtype && dataP);

  XREPLACEME *xP = (XREPLACEME*) sP;
  // template for subtype switch structure. You probably need to store these in maps for access in xREPLACEMEPostprocessComps().
  switch (subtype) {
    default:
      break;
  }
  // Only track entity for the first subcomponent. 
  // 0x40 is the lowest of the upper two bits, which is always used in subcomponents since it's the first subtype.
  if (subtype == 0x40) {
    // Exmaple code in tracking which entities have had subcomponents added so far
    // e = frayAdd(xP->entityF, (void*) &entity, NULL);
  }
}
#endif

#if 0
XGetShareFuncDefUnused_(REPLACEME);
#else
XGetShareFuncDef_(REPLACEME) {
  assert (sP && shareMPMP);
  XREPLACEME *xP = (XREPLACEME*) sP;
  // Example code in getting, say, a destination rectangle from the share map of maps
  /*
  if (!e) {
    e = mapGetNestedMapP(shareMPMP, DST_RECT, &xP->dstRectMP);  
  }
  */
}
#endif

#if 0
XPostprocessCompsDefUnused_(REPLACEME);
#else
XPostprocessCompsDef_(REPLACEME) {
  assert (sP);
  XREPLACEME *xP = (XREPLACEME*) sP;
  // You'll put whatever logic depends on subcomponents and/or shared subcomponents here.
  // For example, using the colormap and color pallete subcomponents to piece together the texture atlas,
  // and then populating the shared source rectangles.
}
#endif

#if 0
XPostMutateFuncDefUnused_(REPLACEME);
#else
XPostMutateFuncDef_(REPLACEME) {
  assert (sP && cP);

  XREPLACEME *xP = (XREPLACEME*) sP;

  XREPLACEMEComp *_cP = (XREPLACEMEComp*) cP;
  
  // Operate on _cP here.
}
#endif

#if 0
XPostActivateFuncDefUnused_(REPLACEME);
#else
XPostActivateFuncDef_(REPLACEME) {
  assert (sP && sParamsP);
  XREPLACEME *xP = (XREPLACEME*) sP;
}
#endif

#if 0
XPostDeactivateFuncDefUnused_(REPLACEME);
#else
XPostDeactivateFuncDef_(REPLACEME) {
  assert (sP && sParamsP);
  XREPLACEME *xP = (XREPLACEME*) sP;
}
#endif


#if 0
XProcMsgFuncDefUnused_(REPLACEME);
#else
XProcMsgFuncDef_(REPLACEME) {
  assert (sP && msgP);
  XREPLACEME *xP = (XREPLACEME*) sP;

  /* Example code:
  if (msgP->cmd == ANIMATE) {
    xMutateComponent(sP, msgP->attn, msgP->arg);
  }
  */
}
#endif

//======================================================
// REPLACEME run function
//======================================================
Error xREPLACEMERun(System *sP) {
  assert (sP);

  XREPLACEME *xP = (XREPLACEME*) sP;

	XREPLACEMEComp *cP = (XREPLACEMEComp*) sP->cF;
	XREPLACEMEComp *cEndP = cP + _frayGetFirstPausedIdx(sP->cF);

  // Operate on all the active elements.
  for (; cP < cEndP; cP++) {
    // do something on each element here
  }
}

#if 0
XClrFuncDefUnused_(REPLACEME);
#else
XClrFuncDef_(REPLACEME) {
  assert (sP);
  XREPLACEME *xP = (XREPLACEME*) sP;
  // TODO put good code here
}
#endif

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(REPLACEME, 1, 0, FLAGS_HERE);
