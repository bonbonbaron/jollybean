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
  for (int i = 0; i < 255; ++i) {  // TODO make this more pro bruh
    stripIni(sdPF[i], TEMPORARY);
  }
#endif
}

// don't have to worry about intercomps, subtrees, 
// What about shares? 
//
// do worry about:
//
//  for inflation function:
//    media
//
//  for system component frays:
//    immutable (obviously only counts once)
//    mutables (whole set only counts once)
//    intracomposites (whole set only counts once)

// histo def: 
  // U32 *nExclusivesA;          // determines each subsystem's number of components 
  // U32  nDistinctMedia;        // determines # of strip data to inflate/unpack/assemble
static void _initSystems( const GeneHisto* histoP ) {
  assert( histoP );
  assert( histoP->nExclusivesA );
  System* sP;
  for (int i = 1; i < N_SYSTEM_TYPES; ++i) {
    if ( histoP->nExclusivesA[i] ) {
       sP = shareGetSystem( i );  // This will crash if gaps exist between impl'd system IDs. Good!
       xIniSys( sP, histoP->nExclusivesA[i] );
    }
  }
}


static void _distributeGene( Entity entity, GeneHdr **geneHdrPP, StripDataS **sdPF ) {
  assert(entity);
  assert(geneHdrPP);
  assert(*geneHdrPP);
  assert(sdPF);
  assert(*sdPF);

  System *sP;
  GeneHdr* geneHdrP = *geneHdrPP;
  switch (geneHdrP->class) {
    case SUBTREE:  // a subtree *is* a composite. "Subtree" just tells us the start of a new entity.
      ++entity;
      // fall through
    case INTERCOMPOSITE:  // recurse  back into this function
      InterCompositeGene* compGeneP = (InterCompositeGene*) geneHdrP;
      GeneHdr** currGeneHdrPP = compGeneP->geneHdrPA;
      GeneHdr** geneHdrEndPP = currGeneHdrPP + compGeneP->hdr.u.n;
      for (; currGeneHdrPP < geneHdrEndPP; ++currGeneHdrPP) {
        _distributeGene(entity, currGeneHdrPP, sdPF );
      }
      break;
    // TODO potential case: ALTERNATIVE
    //  cocnept: if you have a whole genome, but you onyl want to tweak one gene for another instance, 
    //           should you really have to copy the whole genome again with that one change? Seems like
    //           an inefficient way to vary singles. You can already do that with alternatives, but what's
    //           not in place yet is the replacement mechanism. Then again, I haven't coded variants yet 
    //           in the first place. 
    case VARIANT:
      // TODO
      break;
    // TODO what if it's a mutable media gene? How do you tell the difference?
    case MEDIA:
      MediaGene* mediaGeneP = (MediaGene*) geneHdrP;
      // Defer inflation 
      if (!(mediaGeneP->sd.flags & SD_SET_FOR_INFLATION_)) {
        mediaGeneP->sd.flags |= SD_SET_FOR_INFLATION_;
        StripDataS* sdP = &mediaGeneP->sd;  // because you must pass a double-pointer
        frayAdd(sdPF, &sdP, NULL);
      }
      break;
    case INTRACOMPOSITE:
    case IMMUTABLE:
    case MUTABLE:
      sP = shareGetSystem( geneHdrP->u.type ); 
      sP->consumeGene(sP, entity, geneHdrP);
      break;
    default:
      assert(FALSE); // gene has an incompatible gene class
      break;
  }
}

// TODO you need to make a function to init the systems based on the number of genes in each one.
// =====================================================================
// Distribute all genes to their appropriate subsystems.
// =====================================================================
void distributeGenes( const RootGene* rootP ) {
  assert( rootP );
  assert( rootP->hdr.class == ROOT );
  _initSystems( &rootP->histo );
  StripDataS** sdPF = frayNew( sizeof(StripDataS*), rootP->histo.nDistinctMedia, TEMPORARY);  

  GeneHdr** geneHdrPP = rootP->geneHdrPA;
  GeneHdr** geneHdrEndPP = geneHdrPP + rootP->hdr.u.n;
  for (; geneHdrPP < geneHdrEndPP; ++geneHdrPP) {  
    _distributeGene( FIRST_ENTITY, geneHdrPP, sdPF );
  }

  _inflateMedia(sdPF);  
}
