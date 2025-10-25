#include "xMedia.h"
#include "gene/gene.h"
#include "genericSysGenes.h"

XIniSysFuncDefUnused_(Media);
XMakeComponentsDefUnused_(Media);
XPostActivateFuncDefUnused_(Media);
XPostDeactivateFuncDefUnused_(Media);
XPostMutateFuncDefUnused_(Media);
XProcMsgFuncDefUnused_(Media);

XConsumeGeneFuncDef_(Media) {
  XMedia* xP = (XMedia*) sP;
  XMediaComp* cP;
  if ( geneP->class == MUTABLE ) {
    xMakeMutationMap( sP, entity, geneP ); // This is fine as not every system will need this.
  }
  else if ( geneP->class == IMMUTABLE ) {
    cP = (XMediaComp*) xGetCompPByEntity( sP, entity );
    assert( cP );
    MediaImmutableGene* immutableGeneP = (MediaImmutableGene*) geneP;
    cP->immutable = immutableGeneP->body;
  }
  else if ( geneP->class == INTRACOMPOSITE ) {
    IntraCompositeGene* intraP = (IntraCompositeGene*) geneP;
    GeneHdr** currGeneHdrPP = intraP->geneHdrPA;
    GeneHdr** geneHdrEndPP = currGeneHdrPP + intraP->n;
    for (; currGeneHdrPP < geneHdrEndPP; ++currGeneHdrPP) {
      assert(*currGeneHdrPP);
      xMediaConsumeGene( sP, entity, *currGeneHdrPP );
    }
  }
}

void xMediaRun(System *sP) {
  XMediaComp *cP = (XMediaComp*) sP->cF;
  XMediaComp *cEndP = cP + _frayGetFirstPausedIdx(sP->cF);

  for (; cP < cEndP; cP++) {
  }
}

#define FLAGS_HERE (0)
X_(Media, GENERIC, mutableCompositePc1, 0);
