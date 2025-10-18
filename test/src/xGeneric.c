#include "xGeneric.h"
#include "share.h"

XIniSysFuncDef_(Generic) {
  XGeneric *xP = (XGeneric*) sP;
  xP->someSystemwideU32 = 100000;
  xP->aMP = mapNew(RAW_DATA, sizeof(int), 255, GENERAL);
  xP->dMP = mapNew(RAW_DATA, sizeof(double), 255, GENERAL);
  xP->entityF = frayNew( sizeof(Entity), 255, GENERAL);
  shareSetSystemFromType( GENA, sP );
}

XPostprocessCompsDef_(Generic) {
  XGeneric *xP = (XGeneric*) sP;
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
    XGenericComp component = {
//       .a = *aP,     // this is a pre-populated, immutable subcomponent
//       .b = 0,       // this is a share; you don't need to populate it yet. It'll be filled at runtime.
//       .c = 0,       // this is a share; you don't need to populate it yet. It'll be filled at runtime.
//       .d = *dP,     // this is a pre-populated, immutable subcomponent
//       .intP = NULL  // this is from an external share map
    };
    // Now the component's populated. Add it to the system.
    //printf("adding component with a = %d and d = %f to entity %d\n", *aP, *dP, *entityP);
    XGenericComp* compP = (XGenericComp*) xGetCompPByEntity( sP, *entityP );
    *compP = component;
  }
}

XPostActivateFuncDefUnused_(Generic);
XPostDeactivateFuncDefUnused_(Generic);

XPostMutateFuncDef_(Generic) {
  unused_(sP);
  if (cP) {
    XGenericComp *xcP = (XGenericComp*) cP;
    xcP->a *= xcP->b + 1;
    xcP->d *= xcP->c - 2;
  }
}

XProcMsgFuncDef_(Generic) {
  assert (sP &&  msgP);
  if (msgP->cmd == 20) {
    XGeneric* xP = (XGeneric*) sP;
    xP->someSystemwideU32 = 100;
  }
}

XConsumeGeneFuncDef_(Generic) {
  XGeneric* xP = (XGeneric*) sP;
  if ( geneP->class == MUTABLE ) {
    MutableGene* mutableGeneP = (MutableGene*) geneP;
    Map* entitysMutationMP = xNewMutationMap( sP, entity, geneP->u.n );

    Mutation* mutationP = mutableGeneP->mutationPA;
    Mutation* mutationEndP = mutationP + mutableGeneP->hdr.u.n;
    for ( ; mutationP < mutationEndPP; ++mutationP ) {
      mapSet(entitysMutationMP, mutationP->key, mutationP->val);
    }
  }
}

void xGenericRun(System *sP) {
  XGenericComp *cP = (XGenericComp*) sP->cF;
  XGenericComp *cEndP = cP + _frayGetFirstPausedIdx(sP->cF);

  for (; cP < cEndP; cP++) {
    --cP->a;  // 30 becomes 29
    ++cP->b;  // 30 becomes 29
    --cP->c;
    cP->d = cP->a + 1;
  }
}

#define FLAGS_HERE (0)
X_(Generic, GENERIC, b, 0);
