#include "implicitGenes.h"
#include "x/x.h"  // to feed systems their components' genes
#include "data/fray.h"
#include "share.h"

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

static void _distributeGene( Entity entity, Gene **genePP, StripDataS **sdPF ) {
  assert(entity);
  assert(genePP);
  assert(*genePP);
  assert(sdPF);
  assert(*sdPF);

  Gene* geneP = *genePP;
  switch (geneP->class) {
    case SUBTREE:  // a subtree *is* a composite. "Subtree" just tells us the start of a new entity.
      ++entity;
      // fall through
    case COMPOSITE:  // recurse  back into this function
      Gene** compositeGenePP = geneP->u.composite.genePA;
      Gene** compositeGeneEndPP = compositeGenePP + geneP->u.composite.nGenes;
      for (; compositeGenePP < compositeGeneEndPP; ++compositeGenePP) {
        _distributeGene(entity, compositeGenePP, sdPF );
      }
      break;
    case VARIANT:
      // TODO
      break;
    case MEDIA:
      // Defer inflation 
      if (!((*((StripDataS**) geneP->u.unitary.dataP))->flags & SD_SET_FOR_INFLATION_)) {
        (*((StripDataS**) geneP->u.unitary.dataP))->flags |= SD_SET_FOR_INFLATION_;
        frayAdd(sdPF, geneP->u.unitary.dataP, NULL);
      }
      // fall through
    case EXCLUSIVE_MUTABLE:  
      // fall through
    case EXCLUSIVE_IMMUTABLE:
      System* sysP = shareGetSystem( geneP->u.unitary.systemId );
      sysP->consumeGene(sysP, entity, geneP->u.unitary.dataP);
      break;
    default:
      assert(FALSE); // gene has an incompatible gene class
      break;
  }
}

// =====================================================================
// Distribute all genes to their appropriate subsystems.
// =====================================================================
void distributeGenes( Gene* geneP ) {
  assert( geneP );
  assert( geneP->class == ROOT );
  StripDataS** sdPF = frayNew( sizeof(StripDataS*), geneP->u.root.nGeneTypes, TEMPORARY);  

  Gene** genePP = geneP->u.root.composite.genePA;
  Gene** geneEndPP = genePP + geneP->u.root.composite.nGenes;
  for (; genePP < geneEndPP; ++genePP) {  
    _distributeGene( 1, genePP, sdPF );
  }

  _inflateMedia(sdPF);  
}
