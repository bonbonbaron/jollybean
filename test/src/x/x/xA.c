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
  xP->aMP = mapNew(RAW_DATA, sizeof(int), 255);
  xP->dMP = mapNew(RAW_DATA, sizeof(double), 255);
  xP->entityF = frayNew( sizeof(Entity), 255);
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
	assert (sP && entity && subtype && dataP);

	XA *xP = (XA*) sP;

  switch (subtype) {
    case 0x40:
      mapSet(xP->aMP, entity, (int*) dataP);
      // Only add entity once to fray for post-processing.
      frayAdd(xP->entityF, (void*) &entity, NULL);
      break;
    case 0x80:
      mapSet(xP->dMP, entity, (double*) dataP);
      break;
    case 0xa0:
    default:
      break;
  }

}
#endif

// We'll populate the component's intP member with this.
// #define XGetShareFuncDef_(name_) Map* x##name_##GetShare(System *sP, Map *shareMPMP)
#if 0
XGetShareFuncDefUnused_(A);
#else
XGetShareFuncDef_(A) {
  XA *xP = (XA*) sP;
  assert(sP);
  xP->sharedIntMP = mapGetNestedMapP(shareMPMP, 1);
}
#endif

#if 0
XPostprocessCompsDefUnused_(A);
#else
// xAddEntityData() is called before this, so just call xAddComp() if you need to.
XPostprocessCompsDef_(A) {
  XA *xP = (XA*) sP;
  // x.c has already added empty components for us.
  // So all we have to do is set them.
  Entity *entityP = xP->entityF;
  Entity *entityEndP = entityP + *_frayGetFirstEmptyIdxP(xP->entityF);
  // For each entity...
  for (; entityP < entityEndP; ++entityP) {
    int *aP = (int*) mapGet(xP->aMP, *entityP);
    // Get current entity's a.
    assert(aP);
    // Get current entity's d.
    double *dP = (double*) mapGet(xP->dMP, *entityP);
    assert (dP);
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
    XAComp* compP = (XAComp*) xGetCompPByEntity( sP, *entityP );
    *compP = component;
  }
}
#endif

#if 0
XPostMutateFuncDefUnused_(A);
#else
XPostMutateFuncDef_(A) {
  unused_(sP);
  if (cP) {
    XAComp *xcP = (XAComp*) cP;
    xcP->a *= xcP->b + 1;
    xcP->d *= xcP->c - 2;
  }
}
#endif

#if 0
XProcMsgFuncDefUnused_(A);
#else
XProcMsgFuncDef_(A) {
  assert (sP &&  msgP);
  if (msgP->cmd == 20) {
    XA* xP = (XA*) sP;
    xP->someSystemwideU32 = 100;
  }
}
#endif

//======================================================
// A run function
//======================================================
void xARun(System *sP) {
	XAComp *cP = (XAComp*) sP->cF;
	XAComp *cEndP = cP + _frayGetFirstPausedIdx(sP->cF);

  for (; cP < cEndP; cP++) {
    --cP->a;  // 30 becomes 29
    ++cP->b;  // 30 becomes 29
    --cP->c;
    cP->d = cP->a + 1;
  }
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
    xP->someSystemwideU32 = 0;
    mapOfNestedMapsClr(&sP->mutationMPMP);  // THIS is where we properly clear the innards of a nested map, 
                                           // in case more than one system has the same mutation map in their
                                           // entity-to-mutations map.
  }
}
#endif

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(A, 1, b, FLG_DONT_ADD_COMP);
