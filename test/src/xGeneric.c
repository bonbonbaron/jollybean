#include "xGeneric.h"
#include "share.h"
#include "gene.h"

XIniSysFuncDef_(Generic) {
}

XPostprocessCompsDef_(Generic) {
}

XPostActivateFuncDefUnused_(Generic);
XPostDeactivateFuncDefUnused_(Generic);

XPostMutateFuncDef_(Generic) {
}

XProcMsgFuncDef_(Generic) {
}

XConsumeGeneFuncDef_(Generic) {
  XGeneric* xP = (XGeneric*) sP;
  if ( geneP->class == MUTABLE ) {
    xMakeMutationMap( sP, entity, geneP );
  }
}

void xGenericRun(System *sP) {
  XGenericComp *cP = (XGenericComp*) sP->cF;
  XGenericComp *cEndP = cP + _frayGetFirstPausedIdx(sP->cF);

  for (; cP < cEndP; cP++) {
  }
}

#define FLAGS_HERE (0)
X_(Generic, GENERIC, mutableCompositePc1, 0);
