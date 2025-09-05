#include "gene.h"
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
  System *sysP;
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
    // TODO what if it's a mutable media gene? How do you tell the difference?
    case MEDIA:
      MediaGene* mediaGeneP = (MediaGene*) geneHdrP;
      // Defer inflation 
      if (!(mediaGeneP->sd.flags & SD_SET_FOR_INFLATION_)) {
        mediaGeneP->sd.flags |= SD_SET_FOR_INFLATION_;
        frayAdd(sdPF, &mediaGeneP->sd, NULL);
      }
      // fall through
    case EXCLUSIVE_IMMUTABLE:
      sysP = shareGetSystemFromType( geneHdrP->u.type );
      sysP->consumeGene(sysP, entity, geneHdrP);
      break;
    // TODO where is key going to come from? Any Houdini tricks there?
    case EXCLUSIVE_MUTABLE:  
      ExclusiveMutableGene* exmutGeneP = (ExclusiveMutableGene*) geneHdrP;
      GeneHdr** mutationHdrPP = exmutGeneP->mutationPA;
      GeneHdr** mutationEndPP = mutationHdrPP + exmutGeneP->hdr.u.n;
      sysP = shareGetSystemFromType( geneHdrP->u.type );  // TODO Get idx from first elem in array.
      for ( ; mutationHdrPP < mutationEndPP; ++mutationHdrPP ) {
        sysP->consumeGene(sysP, entity, geneHdrP);
      }
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

  GeneHdr** geneHdrPP = rootP->geneHdrPA;
  GeneHdr** geneHdrEndPP = geneHdrPP + rootP->hdr.u.n;
  for (; geneHdrPP < geneHdrEndPP; ++geneHdrPP) {  
    _distributeGene( FIRST_ENTITY, geneHdrPP, sdPF );
  }

  _inflateMedia(sdPF);  
}
