#include "implicitGenes.h"
#include "x.h"  // to feed systems their components' genes

/* DESIGN
 *   It might be kinda nice if we lay out a raw array of media pointers in the gene tool.
 */

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
  switch (gene.geneClass) {
    case COMPOSITE_GENE:  // recurse  back into this function
      Gene** compositeGenePP = gene.u.composite.genePA;
      Gene** compositeGeneEndPP = compositeGenePP + gene.u.composite.nGenes;
      for (; compositeGenePP < compositeGeneEndPP; ++ compositeGenePP) {
        _distributeGene(entity, compositeGenePP, sdPF, masterSysP, sharedGenesMPMP, spawnP);
      }
      break;
    case MEDIA_GENE:
      // Defer unpacking 
      if (!((*((StripDataS**) gene.u.unitary.dataP))->flags & SD_SET_FOR_INFLATION_)) {
        (*((StripDataS**) gene.u.unitary.dataP))->flags |= SD_SET_FOR_INFLATION_;
        frayAdd(sdPF, gene.u.unitary.dataP, NULL);
      }
    case EXCLUSIVE_MUTABLE_GENE:  // Whether mutable or immutable, each system will handle it.
    case EXCLUSIVE_IMMUTABLE_GENE:
      childSysPP = (XMasterComp*) xGetCompPByEntity(masterSysP, gene.u.unitary.type & MASK_COMPONENT_TYPE);
      xAddEntityData(*childSysPP, entity, gene.u.unitary.type, gene.u.unitary.dataP);
      break;
    default:
      break;
  }
}

// TODO Make a simple function that calls all the systems' starter functions in multiple loops.
static void _postProcessChildrenSystems(System *masterSysP) {
  System **childSysPP = masterSysP->cF;
  System **childSysEndPP = childSysPP + xGetNComps(masterSysP);
  for (; childSysPP < childSysEndPP; ++childSysPP) {
    (*childSysPP)->postprocessComps(*childSysPP);
  }
}

// =====================================================================
// Distribute all genes to their appropriate subsystems.
// =====================================================================
static void _distributeGenes() {
  assert( xP );
  _subsystemsIni(&xP->system);
  frayNew((void**) &sdPF, sizeof(StripDataS*), geneHisto.nDistinctMedia, TEMPORARY);  

  // Distribute genes to all spawned entties.
  // Make a blackboard map for this entity if it has any genes to put in it.
  Gene** genePP = spawnP->genomeP->genePA;
  Gene** geneEndPP = genePP + genomeP->nGenes;
  // For each gene of this spawn...
  for (; genePP < geneEndPP; genePP++) {  // genePP is a pointer to a pointer to a global singleton of a component
    _distributeGene( entity, genePP, sdPF, &xP->system );
  }

  // Inflate all media-type genes. All components using them will see it from inside their systems.
  _biomeMediaInflate(sdPF);  
  _postProcessChildrenSystems(&xP->system);
}
