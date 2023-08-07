#include "xMotion.h"

/* REQUIREMENTS
 *
 *  Mutations for different directions' velocities
 *  Shared destination rectangle for each entity
 */

//======================================================
// Initialize Motion's system.
//======================================================
#if 1
XIniSysFuncDefUnused_(Motion);
#else
XIniSysFuncDef_(Motion) {
  if (!sP || !sParamsP) {
    return E_BAD_ARGS;
  }
  XMotion *xP = (XMotion*) sP;
  U32 nComponents = xGetNComps(sP);
  // You may want to initialize whatever specialized maps, arrays, etc. the system owns here.
  // Hint: You most likely need nComponents to allocate them.
  return SUCCESS;
}
#endif

#if 1
XIniSubcompFuncDefUnused_(Motion);
#else
XIniSubcompFuncDef_(Motion) {
	if (!sP || !entity || !subtype || !dataP) {
		return E_BAD_ARGS;
  }

  Error e = SUCCESS;
  XMotion *xP = (XMotion*) sP;
  // template for subtype switch structure. You probably need to store these in maps for access in xMotionPostprocessComps().
  switch (subtype) {
    default:
      break;
  }
  // Only track entity for the first subcomponent. 
  // 0x40 is the lowest of the upper two bits, which is always used in subcomponents since it's the first subtype.
  if (!e && subtype == 0x40) {
    // Exmaple code in tracking which entities have had subcomponents added so far
    // e = frayAdd(xP->entityF, (void*) &entity, NULL);
  }

	return e;
}
#endif

#if 0
XGetShareFuncDefUnused_(Motion);
#else
XGetShareFuncDef_(Motion) {
  if (!sP || !shareMPMP) {
    return E_BAD_ARGS;
  }
  return mapGetNestedMapP(shareMPMP, DST_RECT, &(((XMotion*) sP)->dstRectMP));
}
#endif

#if 0
XPostprocessCompsDefUnused_(Motion);
#else
XPostprocessCompsDef_(Motion) {
  if (!sP) {
    return E_BAD_ARGS;
  }
  XMotion *xP = (XMotion*) sP;
  
  Error e = SUCCESS;
  
  Entity *entityP = sP->cIdx2eA;
  Entity *entityEndP = entityP + arrayGetNElems(sP->cIdx2eA);

  // Populate each motion system component. Start out with zero velocity and let action system mutate it later.
  for (XMotionComp *cP; !e && entityP < entityEndP; ++entityP) {
    cP = (XMotionComp*) xGetCompPByEntity(sP, *entityP);
    if (cP) {
      cP->dstRectP = (Rect_*) mapGet(xP->dstRectMP, *entityP);
    }
    if (!cP->dstRectP) {
      return E_BAD_ARGS;
    }
  }

  return e;
}
#endif

#if 1
XPostMutateFuncDefUnused_(Motion);
#else
XPostMutateFuncDef_(Motion) {
  if (!sP || !cP) {
    return E_BAD_ARGS;
  }

  XMotion *xP = (XMotion*) sP;

  XMotionComp *_cP = (XMotionComp*) cP;
  
  // Operate on _cP here.

  return SUCCESS;
}
#endif

#if 1
XProcMsgFuncDefUnused_(Motion);
#else
XProcMsgFuncDef_(Motion) {
  if (!sP || !msgP) {
    return E_BAD_ARGS;
  }
  XMotion *xP = (XMotion*) sP;

  /* Example code:
  if (msgP->cmd == ANIMATE) {
    e = xMutateComponent(sP, msgP->attn, msgP->arg);
  }
  else {
    e = E_MAILBOX_BAD_RECIPIENT;
  }
  */
	return SUCCESS;
}
#endif

//======================================================
// Motion run function
//======================================================
Error xMotionRun(System *sP) {
  if (!sP) {
    return E_BAD_ARGS;
  }

	Error e = SUCCESS;
  XMotion *xP = (XMotion*) sP;

	XMotionComp *cP = (XMotionComp*) sP->cF;
	XMotionComp *cEndP = cP + _frayGetFirstPausedIdx(sP->cF);

  // Operate on all the active elements.
  for (; cP < cEndP; cP++) {
    printf("{%d, %d, %d, %d}\n", cP->dstRectP->x, cP->dstRectP->y, cP->velocity.x, cP->velocity.y);
    cP->dstRectP->x += cP->velocity.x;
    cP->dstRectP->y += cP->velocity.y;
  }

	return e;
}

#if 1
XClrFuncDefUnused_(Motion);
#else
XClrFuncDef_(Motion) {
  if (!sP) {
    return E_BAD_ARGS;
  }
  XMotion *xP = (XMotion*) sP;
  // TODO put good code here
  return SUCCESS;
}
#endif

//======================================================
// System definition
//======================================================
X_(Motion, MOTION, velocity, 0);
