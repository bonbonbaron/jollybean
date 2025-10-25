#include "xMedia.h"
#include "gene/gene.h"
#include "mediaSysGenes.h"

XIniSysFuncDefUnused_(Media);
XMakeComponentsDefUnused_(Media);
XPostActivateFuncDefUnused_(Media);
XPostDeactivateFuncDefUnused_(Media);
XPostMutateFuncDefUnused_(Media);
XProcMsgFuncDefUnused_(Media);

XConsumeGeneFuncDef_(Media) {
  XMedia* xP = (XMedia*) sP;
  XMediaComp* cP;
  if ( geneP->class == INTRACOMPOSITE ) {
    IntraCompositeGene* intraP = (IntraCompositeGene*) geneP;
    GeneHdr** currGeneHdrPP = intraP->geneHdrPA;
    GeneHdr** geneHdrEndPP = currGeneHdrPP + intraP->n;
    for (; currGeneHdrPP < geneHdrEndPP; ++currGeneHdrPP) {
      assert(*currGeneHdrPP);
      xMediaConsumeGene( sP, entity, *currGeneHdrPP );
    }
  }
  // I'm getting tired of coding. I'll save this for Sunday evening or so.
  else if ( geneP->class == MEDIA  ) {
    xRegisterMediaGene( geneP );
    // TODO stick the image pointer into the component.
  }
  else if ( geneP->class == IMMUTABLE ) {
    // TODO stick the int into the component.
  }
}

XRunFuncDef_(Media);
  XMediaComp *cP = (XMediaComp*) sP->cF;
  XMediaComp *cEndP = cP + _frayGetFirstPausedIdx(sP->cF);

  for (; cP < cEndP; cP++) {
  }
}

#define FLAGS_HERE (0)
X_(Media, MEDIA_SYS_ID, mutableCompositePc1, 0);
