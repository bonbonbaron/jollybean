#include "xGeneric.h"
#include "share.h"
#include "gene.h"
#include "genericSysGenes.h"

XIniSysFuncDefUnused_(Generic);
XPostprocessCompsDefUnused_(Generic);
XPostActivateFuncDefUnused_(Generic);
XPostDeactivateFuncDefUnused_(Generic);
XPostMutateFuncDefUnused_(Generic);
XProcMsgFuncDefUnused_(Generic);

XConsumeGeneFuncDef_(Generic) {
  XGeneric* xP = (XGeneric*) sP;
  if ( geneP->class == MUTABLE ) {
    xMakeMutationMap( sP, entity, geneP ); // This is fine as not every system will need this.
  }
  else if ( geneP->class == IMMUTABLE ) {
    XGenericComp* cP = (XGenericComp*) xGetCompPByEntity( sP, entity );
    assert( cP );
    GenericImmutableGene* immutableGeneP = (GenericImmutableGene*) geneP;
    cP->immutable = immutableGeneP->body;
  }
  // TODO make an if-statement for an intracomposite
}

void xGenericRun(System *sP) {
  XGenericComp *cP = (XGenericComp*) sP->cF;
  XGenericComp *cEndP = cP + _frayGetFirstPausedIdx(sP->cF);

  for (; cP < cEndP; cP++) {
  }
}

#define FLAGS_HERE (0)
X_(Generic, GENERIC, mutableCompositePc1, 0);
