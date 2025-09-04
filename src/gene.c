#include "implicitGenes.h"
#include "x/x.h"  // to feed systems their components' genes
#include "data/fray.h"
#include "share.h"

#define FIRST_ENTITY ( 1 )

// Inflate a whole array of strip data.
static void _inflateMedia(StripDataS **sdPF) {
  assert(sdPF);
#if MULTITHREADED
  multithread_(sdInflate, (void*) sdPF);
  multithread_(sdUnpack, (void*) sdPF);
  multithread_(sdAssemble, (void*) sdPF);
#else 
  for (int i = 0; i < 255; ++i) {
    stripIni(sdPF[i], TEMPORARY);
  }
#endif
}

static void _distributeGene( Entity entity, GeneHdr **geneHdrPP, StripDataS **sdPF ) {
  assert(entity);
  assert(geneHdrPP);
  assert(*geneHdrPP);
  assert(sdPF);
  assert(*sdPF);

  GeneHdr* geneHdrP = *geneHdrPP;
  switch (geneHdrP->class) {
    case SUBTREE:  // a subtree *is* a composite. "Subtree" just tells us the start of a new entity.
      ++entity;
      // fall through
    case COMPOSITE:  // recurse  back into this function
      CompositeGene* compGeneP = (CompositeGene*) geneHdrP;
      GeneHdr** currGeneHdrPP = compGeneP->geneHdrPA;
      GeneHdr** geneHdrEndPP = currGeneHdrPP + compGeneP->hdr.u.n;
      for (; currGeneHdrPP < geneHdrEndPP; ++currGeneHdrPP) {
        _distributeGene(entity, currGeneHdrPP, sdPF );
      }
      break;
    case VARIANT:
      // TODO
      break;
    case MEDIA:
      MediaGene* mediaGeneP = (MediaGene*) geneHdrP;
      // Defer inflation 
      if (!(mediaGeneP->sd.flags & SD_SET_FOR_INFLATION_)) {
        mediaGeneP->sd.flags |= SD_SET_FOR_INFLATION_;
        frayAdd(sdPF, &mediaGeneP->sd, NULL);
      }
      // fall through
    case EXCLUSIVE_MUTABLE:  
      // fall through
    case EXCLUSIVE_IMMUTABLE:
      System* sysP = shareGetSystem( geneHdrP->hdr.u.systemId );
      sysP->consumeGene(sysP, entity, geneHdrP);
      break;
    default:
      assert(FALSE); // gene has an incompatible gene class
      break;
  }
}

// =====================================================================
// Distribute all genes to their appropriate subsystems.
// =====================================================================
void distributeGenes( RootGene* rootP ) {
  assert( rootP );
  assert( rootP->hdr.class == ROOT );
  StripDataS** sdPF = frayNew( sizeof(StripDataS*), rootP->histo.nDistinctMedia, TEMPORARY);  

  Gene** geneHdrPP = rootP->geneHdrPA;
  Gene** geneHdrEndPP = geneHdrPP + rootHdrP->hdr.u.n;
  for (; genePP < geneEndPP; ++genePP) {  
    _distributeGene( FIRST_ENTITY, genePP, sdPF );
  }

  _inflateMedia(sdPF);  
}
