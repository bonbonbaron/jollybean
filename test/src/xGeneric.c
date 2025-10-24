#include "xGeneric.h"
#include "gene/gene.h"
#include "genericSysGenes.h"

XIniSysFuncDefUnused_(Generic);
XMakeComponentsDefUnused_(Generic);
XPostActivateFuncDefUnused_(Generic);
XPostDeactivateFuncDefUnused_(Generic);
XPostMutateFuncDefUnused_(Generic);
XProcMsgFuncDefUnused_(Generic);

XConsumeGeneFuncDef_(Generic) {
  XGeneric* xP = (XGeneric*) sP;
  XGenericComp* cP;
  if ( geneP->class == MUTABLE ) {
    xMakeMutationMap( sP, entity, geneP ); // This is fine as not every system will need this.
  }
  else if ( geneP->class == IMMUTABLE ) {
    cP = (XGenericComp*) xGetCompPByEntity( sP, entity );
    assert( cP );
    GenericImmutableGene* immutableGeneP = (GenericImmutableGene*) geneP;
    cP->immutable = immutableGeneP->body;
  }
  else if ( geneP->class == INTRACOMPOSITE ) {
    IntraCompositeGene* intraP = (IntraCompositeGene*) geneP;
    GeneHdr** currGeneHdrPP = intraP->geneHdrPA;
    GeneHdr** geneHdrEndPP = currGeneHdrPP + intraP->n;
    for (; currGeneHdrPP < geneHdrEndPP; ++currGeneHdrPP) {
      assert(*currGeneHdrPP);
      xGenericConsumeGene( sP, entity, *currGeneHdrPP );
    }
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
