#include "xMedia.h"
#include "gene/gene.h"
#include "mediaSysGenes.h"
#include <stdio.h>

XIniSysFuncDefUnused_(Media);
XMakeComponentsDefUnused_(Media);
XPostActivateFuncDefUnused_(Media);
XPostDeactivateFuncDefUnused_(Media);
XPostMutateFuncDefUnused_(Media);
XProcMsgFuncDefUnused_(Media);

XConsumeGeneFuncDef_(Media) {
  assert( sP );
  assert( geneP );
  XMedia* xP = (XMedia*) sP;
  XMediaComp* cP;
  assert ( geneP->class == IMMUTABLE );
  assert ( geneP->u.type == MEDIA_SYS_ID );
  cP = xGetCompPByEntity( sP, entity );
  assert( cP );
  MediaGene* mediaGeneP = (MediaGene*) geneP;
  xRegisterForInflation( &mediaGeneP->sd );
  cP->sdP = &mediaGeneP->sd;  
}

XRunFuncDef_(Media) {
  XMediaComp *cP = (XMediaComp*) sP->cF;
  XMediaComp *cEndP = cP + _frayGetFirstPausedIdx(sP->cF);

  for (; cP < cEndP; cP++) {
    assert( ssGetOutput( cP->sdP ) != NULL );
  }
}

#define FLAGS_HERE (0)
X_no_mutations_(Media, MEDIA_SYS_ID, 0);
