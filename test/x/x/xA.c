#include "xA.h"

/* STRUCTURE OF AN XA COMPONENT:
 *
 * The component has four subcomponents.
 *   A and D are immutable subcomponents;
 *   B and C are mutable.
 *
 * Therefore, we'll store a one-to-one mapping from each entity to its A and D subcomponents,
 * and then we'll store a one-to-many mapping from each entity to its B and C subcomponents.
 *
 * B and C will also undergo postmutations to fully test that feature of X.
 *
 */

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
  XA *xP = (XA*) sP;
  xP->someSystemwideU32 = 100000;
  Error e = mapNew(&xP->aMP, RAW_DATA, sizeof(int), 255);
  if (!e) {
    e = mapNew(&xP->dMP, RAW_DATA, sizeof(double), 255);
  }
  if (!e) {
    e = frayNew((void**) &xP->entityF, sizeof(Entity), 255);
  }
  return e;
}
#endif
/* Let's think through the design a bit.
 *
 * A's subcomps will be a and d.
 * Because b and c will come from mutations.
 * So 0x40 will map to a and 0x80 to d.
 * That'll be stored in a map.
 * The get-shares will have some integer pointers in the "outside world"; intP.
 * Mutation maps will be added externally in the tau setup.
 * The post-processor will come in and put the shares and subcomponents  into a component.
 * It'll also call some default mutation.
 * The mutation of b and c will cause a multiplication of a and d.
 * Message-processing will then be the only way we interact with this system from then on out.
 */

#if 0
XIniSubcompFuncDefUnused_(A); 
#else 
XIniSubcompFuncDef_(A) {
	if (!sP || !entity || !subtype || !dataP) {
		return E_BAD_ARGS;
  }

  Error e = SUCCESS;
	XA *xP = (XA*) sP;

  switch (subtype) {
    case 0x40:
      e = mapSet(xP->aMP, entity, (int*) dataP);
      break;
    case 0x80:
      e = mapSet(xP->dMP, entity, (double*) dataP);
      break;
    case 0xa0:
    default:
      break;
  }

  // Add entity to entity array for post-processing.
  if (!e) {
    if (xP->entityF[entity - 1] != entity) {
      e = frayAdd(xP->entityF, (void*) &entity, NULL);
    }
  }

	return e;
}
#endif

// We'll populate the component's intP member with this.
#if 0
XGetShareFuncDefUnused_(A);
#else
XGetShareFuncDef_(A) {
  XA *xP = (XA*) sP;
  Error e = SUCCESS;
  int *intP;
  if (sP && shareMPMP) {
    e = mapGetNestedMapP(shareMPMP, 1, &xP->sharedIntMP);
  }
  return e;
}
#endif

#if 0
XPostprocessCompsDefUnused_(A);
#else
// xAddEntityData() is called before this, so just call xAddComp() if you need to.
XPostprocessCompsDef_(A) {
  XA *xP = (XA*) sP;
  Error e = SUCCESS;
  Entity *entityP = xP->entityF;
  Entity *entityEndP = entityP + *_frayGetFirstEmptyIdxP(xP->entityF);
  // For each entity...
  for (; !e && entityP < entityEndP; ++entityP) {
    int *aP = (int*) mapGet(xP->aMP, *entityP);
    // Get current entity's a.
    if (!aP) {
      e = E_BAD_KEY;
      break;
    }
    // Get current entity's d.
    double *dP = (double*) mapGet(xP->dMP, *entityP);
    if (!dP) {
      e = E_BAD_KEY;
      break;
    }
    // Turn it into a full-fledged component.
    XAComp component = {
      .a = *aP,     // this is a pre-populated, immutable subcomponent
      .b = 0,       // this is a share; you don't need to populate it yet. It'll be filled at runtime.
      .c = 0,       // this is a share; you don't need to populate it yet. It'll be filled at runtime.
      .d = *dP,     // this is a pre-populated, immutable subcomponent
      .intP = NULL  // this is from an external share map
    };
    // Now the component's populated. Add it to the system.
    //printf("adding component with a = %d and d = %f to entity %d\n", *aP, *dP, *entityP);
    e = xAddComp(sP, *entityP, &component);
  }

  return e;
}
#endif

#if 0
XPostMutateFuncDefUnused_(A);
#else
XPostMutateFuncDef_(A) {
  unused_(sP);
  if (cP) {
    XAComp *xcP = (XAComp*) cP;
    printf("%d * (%d + 1) = ", xcP->a, xcP->b);
    xcP->a *= xcP->b + 1;
    printf("%d\n", xcP->a);
    printf("%5.1f * (%d - 2) = ", xcP->d, xcP->c);
    xcP->d *= xcP->c - 2;
    printf("%5.1f\n", xcP->d);
  }
  return SUCCESS;
}
#endif

#if 0
XProcMsgFuncDefUnused_(A);
#else
XProcMsgFuncDef_(A) {
  if (!sP || !msgP) {
    return E_BAD_ARGS;
  }
  if (msgP->cmd == 20) {
    XA* xP = (XA*) sP;
    xP->someSystemwideU32 = 100;
  }
	return SUCCESS;
}
#endif

//======================================================
// A run function
//======================================================
Error xARun(System *sP) {
	XAComp *cP = (XAComp*) sP->cF;
	XAComp *cEndP = cP + _frayGetFirstPausedIdx(sP->cF);

  for (; cP < cEndP; cP++) {
    --cP->a;
    ++cP->b;
    --cP->c;
    cP->d = cP->a + 1;
  }

	return SUCCESS;
}

#if 0
XClrFuncDefUnused_(A);
#else
XClrFuncDef_(A) {
  if (sP) {
    XA *xP = (XA*) sP;
    if (xP->aMP) {
      mapDel(&xP->aMP);
    }
    if (xP->dMP) {
      mapDel(&xP->dMP);
    }
    if (xP->entityF) {
      frayDel((void**) &xP->entityF);
    }
    if (xP->sharedIntMP) {
      mapDel(&xP->sharedIntMP);
    }
    xP->someSystemwideU32 = 0;
  }
  return SUCCESS;
}
#endif

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(A, 1, b, FLG_DONT_ADD_COMP);
