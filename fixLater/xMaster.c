#include "xMaster.h"
#include "implicitGenes.h"
#include "jb.h"
#include "x.h"

X_no_mutations_(Master, 1, FLG_NO_CF_SRC_A_); 

// TODO the main PROBLEM I remember was shares creeping into X territory. Given that can overflow past the max number of systems, that can segfault our histo.
//
// So the GOAL is to make a separate histo for each gene type.

// Unused functions
XProcMsgFuncDefUnused_(Master);
XIniSubcompFuncDefUnused_(Master);
XGetShareFuncDefUnused_(Master);
XPostprocessCompsDefUnused_(Master);

XClrFuncDef_(Master) {
  if (!sP) {
    return SUCCESS;
  }
  XMaster *xP = (XMaster*) sP;
  xMasterDelShareMap(&xP->sharedMPMP);

  if (sP->cF) {
    XMasterComp *cP = sP->cF;
    XMasterComp *cEndP = cP + *_frayGetFirstEmptyIdxP(sP->cF);
    for (; cP < cEndP; ++cP) {
      xClr(*cP);  // cP is a pointer to a system pointer.
    }
  }
  guiDel(&xP->guiP);
  return SUCCESS;
}

// Number of entities to spawn (both total and per spawn struct)
static void _countEntitiesToSpawn(Biome *biomeP) {
  PositionNode *posNodeP, *posNodeEndP;
  Spawn *spawnP = biomeP->spawnA;
  Spawn *spawnEndP = spawnP + biomeP->nSpawns;
  biomeP->nEntitiesToSpawn = 0;
  for (; spawnP < spawnEndP; ++spawnP) {
    spawnP->nEntitiesToSpawn = 0;
    posNodeP = spawnP->positionNodeA;
    posNodeEndP = posNodeP + spawnP->nEntitiesPossible;
    if (spawnP->keyP) {
      for (; posNodeP < posNodeEndP; ++posNodeP) {
        spawnP->nEntitiesToSpawn += (*spawnP->keyP == posNodeP->keyhole);
      }
    }
    else {
      spawnP->nEntitiesToSpawn = spawnP->nEntitiesPossible;
    }
    biomeP->nEntitiesToSpawn += spawnP->nEntitiesToSpawn;
  }
}

/*************************/
/*****  HISTOGRAMS  ******/
/*************************/
// Systems might require "sub"-components (incredients) to make components.
// In such a case, gene.u.unitary.type & MASK_COMPONENT_SUBTYPE will be nonzero.
// A system cannot consume both genes for full and sub-components; 
// It either has to make them out of sub-components or it copies them straight into sp->cF.
//
// For example, XRender needs to consume both a colormap and color palette for each component.
//
// But we must ensure multiple sub-comps don't increment system population by more than one.
// To prevent that, we either increment its population with 
//    * exclusive component genes or 
//    * exclusive or media sub-comp genes if gene.u.unitary.type & MASK_SUBCOMPONENT = 0x40.
//
// Subtype 0 means this is a component, not sub-component.
// Because 0x40 is the first bit of the upper two, meaning the first ingredient.

// All gene histo-incrementing functions are inlined. 
// Their purpose is to enforce their corresponding enums.

typedef enum {DONT_INCREMENT_NUM_OF_ENTITIES, DO_INCREMENT_NUM_OF_ENTITIES} IncrementEntity;
inline static IncrementEntity _incrementXHistoElem(XHistoElem *xheP, Gene *geneP, Key incrementBy) {
  // This combines a subtype needing to be 0x40 or being a full-component gene into one comparison.
  if (geneP->geneClass == COMPOSITE_GENE) {
    xheP->count    += incrementBy;
    xheP->size      = geneP->u.unitary.size;
    xheP->geneClass = geneP->geneClass;
    xheP->geneType  = geneP->u.unitary.type & MASK_COMPONENT_TYPE;  // used to find system
    return DO_INCREMENT_NUM_OF_ENTITIES;
  }
  // else if ((geneP->u.unitary.type & MASK_COMPONENT_SUBTYPE) <= 0x40) {  (the below is equivalent)
  else if (geneP->u.unitary.type < 0x80) {
    xheP->count += incrementBy;
    xheP->size = geneP->u.unitary.size;
    xheP->geneClass = geneP->geneClass;
    xheP->geneType = geneP->u.unitary.type & MASK_COMPONENT_TYPE;  // used to find system
    return DO_INCREMENT_NUM_OF_ENTITIES;
  }
  return DONT_INCREMENT_NUM_OF_ENTITIES;
}

static Error _histoGene(
    Gene **genePP, 
    const Biome *biomeP, 
    const U32 nSystemsMax,
    Entity *entityP, 
    Spawn *spawnP, 
    GeneHisto *geneHistoP) {

  Error e = SUCCESS;

  XHistoElem *histoA = geneHistoP->nExclusivesA;
  XHistoElem *xHistoElemP = NULL;
  Key *spawnMutationHistoA = geneHistoP->nMutationsPerSpawnAA;
  Key mutationHistoElemIdx;

  StripDataS *sdP;

  Gene gene = **genePP;
  // Get the ECS histo element for the system this gene belongs to.
  xHistoElemP = &histoA[gene.u.unitary.type & MASK_COMPONENT_TYPE];
  switch (gene.geneClass) {
    case COMPOSITE_GENE:
      for (int i = 0; !e && i < gene.u.composite.nGenes; ++i) {
        e = _histoGene(&gene.u.composite.genePA[i], biomeP, nSystemsMax, entityP, spawnP, geneHistoP);
      }
      break;
    case SHARED_GENE:
      geneHistoP->nSharedGeneMaps += !xHistoElemP->count;
      // Fall through; the logic in exclusive gene case applies too.  TODO <-- but why??
    case EXCLUSIVE_GENE:
      if (_incrementXHistoElem(xHistoElemP, &gene, spawnP->nEntitiesToSpawn)) {
        *entityP += spawnP->nEntitiesToSpawn;
        if (gene.u.unitary.key) {
          // The mutation histo is 2D: X dimension: system ID, Y dimension: entity.
          // So you track how many mutations each entity has in each system.
          mutationHistoElemIdx = (spawnP - biomeP->spawnA) * nSystemsMax + (gene.u.unitary.type & MASK_COMPONENT_TYPE);
          ++spawnMutationHistoA[mutationHistoElemIdx];
        }
      }
      break;
    // Media genes can be system sub-components too, hence its call to _incrementXHistoElem().
    case MEDIA_GENE:  
      sdP = *((StripDataS**) gene.u.unitary.dataP);
      if (!(sdP->flags & SD_IS_COUNTED_)) {
        sdP->flags |= SD_IS_COUNTED_;
        ++geneHistoP->nDistinctMedia;
      }
      // Media genes do NOT increment the number of entities.
      _incrementXHistoElem(xHistoElemP, &gene, spawnP->nEntitiesToSpawn);
      break;
    default:
      e = E_INVALID_GENE_CLASS;
      break;
  }
  return e;
}

static Error _histoGenes(GeneHisto *geneHistoP, const Biome *biomeP, const U32 nSystemsMax) {
  Error e = SUCCESS;
  if (!geneHistoP || !geneHistoP->nExclusivesA || !biomeP) {
    return E_BAD_ARGS;
  }

  Spawn *spawnP = biomeP->spawnA;
  Spawn *spawnEndP = spawnP + biomeP->nSpawns;   // pointer to the end of the above array
  Gene **genePP, **geneEndPP;   // pointers to an array of gene header pointers and its end
  Entity entity = 0;
  // Loop through genomes
  for (; !e && spawnP < spawnEndP; spawnP++) {
    genePP = &spawnP->genomeP->genePA[0];
    geneEndPP = genePP + spawnP->genomeP->nGenes;
    // Histo genome's genes that aren't blackboard items.
    for (; !e && genePP < geneEndPP; genePP++) {
      _histoGene(genePP, biomeP, nSystemsMax, &entity, spawnP, geneHistoP);
    }
  }
  // Loop through implicit genes.
  for (Key i = 0, iEnd = arrayGetNElems(geneHistoP->nExclusivesA); i < iEnd; ++i) {
    if (geneHistoP->nExclusivesA[i].count && iglA[i]) {
      /*
       * The reason I add to distinct shared genes without checking "if counted already" here,
       * unlike in _histoGene()'s SHARED_GENE case, is because implicit genes are mutually 
       * exclusive; they only belong to one explicit gene type.
       *
       * Since we're looping through the explicit gene types, if we have any, we just add the
       * number of implicit genes attached to them here. 
       *
       */
      geneHistoP->nSharedGeneMaps += iglA[i]->nGenes;  
    }
  }
  return SUCCESS;
}

/***************************/
/*****  DISTRIBUTION  ******/
/***************************/

// Initialize subsystems before we put components in them. Only init ones with any components.
static Error _subsystemsIni(System *masterSysP, GeneHisto *geneHistoP) {
  Error e = SUCCESS;
  XHistoElem *histoElemP, *histoElemEndP;
  histoElemP = &geneHistoP->nExclusivesA[0];
  histoElemEndP = histoElemP + arrayGetNElems(geneHistoP->nExclusivesA);
  for (; !e && histoElemP < histoElemEndP; histoElemP++) {
    /* "<= MEDIA_GENE" includes MEDIA_GENE and EXCLUSIVE_GENE, 
       both of which can go into systems. */
    // This is asking "if any entities have components for this system..."
    if (histoElemP->geneClass <= MEDIA_GENE && histoElemP->count)  {  
      XMasterComp *cP = (XMasterComp*) xGetCompPByEntity(masterSysP, histoElemP->geneType & MASK_COMPONENT_TYPE);
      if (cP) {
        // Assume no subsystems need extra parameters.
        e = xIniSys(*cP, histoElemP->count, NULL);  
      }
    }
  }

  return e;
}

static Error _addSharedSubmap(Map *outerShareMP, MapElemType mapElemClass, SharedGeneType sharedObjType, U8 innerMapElemSz, Key nInnerMapElems) {
  Map *innerShareMP = NULL;
  Error e = mapNew(&innerShareMP, mapElemClass, innerMapElemSz, nInnerMapElems);
  if (!e) {
    e = mapSet(outerShareMP, sharedObjType, (void**) &innerShareMP);
  }
  return e;
}

Error _makeSharedGeneMap(Map **sharedGenesMPP, GeneHisto *geneHistoP) {
  // Create all the inner maps according to the histo'd number of elements they hold.
  // Add RENDERER_GENE_TYPE (= 2) to total number for window and renderer shares.
  Error e = mapNew(sharedGenesMPP, MAP_POINTER, sizeof(Map*), geneHistoP->nSharedGeneMaps + GUI_GENE_TYPE);  
  if (!e) {
    Map *sharedGenesMP = *sharedGenesMPP;
    XHistoElem *xHistoElemP    = &geneHistoP->nExclusivesA[0];
    XHistoElem *xHistoElemEndP = xHistoElemP + arrayGetNElems(geneHistoP->nExclusivesA);
    // Loop through explicit shared genes.
    for (; !e && xHistoElemP < xHistoElemEndP; xHistoElemP++) {
      if (xHistoElemP->geneClass == SHARED_GENE && xHistoElemP->count) {
        e = _addSharedSubmap(
            sharedGenesMP, 
            RAW_DATA,  // TODO if this ever fails, add an element type field to implicit genes.
            xHistoElemP->geneType & MASK_COMPONENT_TYPE,  // outer map's key to inner map
            xHistoElemP->size,  // size of elems in inner map
            xHistoElemP->count);  // number of elems in inner map
      }
    }
    // Loop through implicit shared genes.
    for (Key i = 0, iEnd = arrayGetNElems(geneHistoP->nExclusivesA); !e && i < iEnd; ++i) {
      if (geneHistoP->nExclusivesA[i].count && iglA[i]) {
        for (Key j = 0, jEnd = iglA[i]->nGenes; !e && j < jEnd; ++j) {
          e = _addSharedSubmap(
              sharedGenesMP, 
              RAW_DATA,
              iglA[i]->listA[j].type,  // outer map's key to inner map
              iglA[i]->listA[j].size,  // size of elems in inner map 
              geneHistoP->nExclusivesA[i].count);  // number of elems in inner map
        }
      }
    }
  }
  return e;
}

void xMasterDelShareMap(Map **sharedMPMPP) {
  if (sharedMPMPP && *sharedMPMPP) {
    Map *sharedMPMP = *sharedMPMPP;
    /* Assume first element is the GUI.
       We want to delete textures before the renderer, which takes textures along with it.
       Otherwise we risk double-freeing those. */
    Map **mapPP = ((Map**) sharedMPMP->mapA) + GUI_GENE_TYPE;
    Map **mapEndPP = mapPP + sharedMPMP->population - GUI_GENE_TYPE;
    for (; mapPP < mapEndPP; ++mapPP) {
      mapDel(mapPP);
    }
    if (sharedMPMP->population >= GUI_GENE_TYPE) {
      mapPP = (Map**) sharedMPMP->mapA;
      mapDel(&mapPP[0]);
    }
    mapDel(sharedMPMPP);
  }
}


static Error _geneHistoIni(GeneHisto *geneHistoP, Entity nEntities, Entity nSystemsMax) {
  geneHistoP->nDistinctMedia   = 0;
  geneHistoP->nSharedGeneMaps = 0;
  Error e = arrayNew((void**) &geneHistoP->nMutationsPerSpawnAA, sizeof(Key), nSystemsMax * nEntities);
  if (!e) {
    e = arrayNew((void**) &geneHistoP->nExclusivesA, sizeof(XHistoElem), nSystemsMax);
  }
  if (!e) {
    e = arrayNew((void**) &geneHistoP->nCompositesA, sizeof(CompositeGeneHistoElem), KEY_MAX);
  }
  return e;
}

static void _geneHistoClr(GeneHisto *geneHistoP) {
  if (geneHistoP) {
    geneHistoP->nDistinctMedia   = 0;
    geneHistoP->nSharedGeneMaps = 0;
    arrayDel((void**) &geneHistoP->nMutationsPerSpawnAA);
    arrayDel((void**) &geneHistoP->nExclusivesA);
    arrayDel((void**) &geneHistoP->nCompositesA);
  }
}

static Error _distributeSharedGenesToSubsystems(System *masterSysP, Map *sharedGenesMPMP) {
  System **subSysPP = masterSysP->cF;
  System **subSysEndPP = subSysPP + arrayGetNElems((void*) masterSysP->cF);
  Error e = SUCCESS;
  for (; !e && subSysPP < subSysEndPP; subSysPP++) {
    if ((*subSysPP)->getShare) {
      e = ((*subSysPP)->getShare)(*subSysPP, sharedGenesMPMP);
    }
  }
  return e;
}

// This is for adding the window and renderer to master's map of shared genes.
static Error _addSpecialSharedGene(Map *sharedGeneMPMP, void *geneP, Key geneType) {
  Map *innerMapP = NULL;
  // Make the inner map first inside the outer map.
  Error e = _addSharedSubmap(sharedGeneMPMP, NONMAP_POINTER, geneType, sizeof(void*), 1);
  // Then get the inner map and set its element the special gene pointer.
  if (!e) {
    e = mapGetNestedMapP(sharedGeneMPMP, geneType, &innerMapP);
  }
  if (!e) {
    e = mapSet(innerMapP, 1, &geneP);
  }
  return e;
}

// This is for adding dest rect. Maybe other stuff in the future too.
static Error _spawnEntities(Map *sharedGeneMPMP, Biome *biomeP) {
  if (!sharedGeneMPMP || !biomeP || !biomeP->nEntitiesToSpawn || !biomeP->nSpawns || !biomeP->spawnA) {
    return E_BAD_ARGS;
  }
  // Master doesn't touch rect offset fray. Renderer updates it and messages Anim to use it.
  // Populate above resources.
  Spawn *spawnP = biomeP->spawnA;
  Spawn *spawnEndP = spawnP + biomeP->nEntitiesToSpawn;
  PositionNode *posP, *posEndP;
  Map  *dstRectMP = NULL;
  Rect_ dstRect = {0};

  Error e = SUCCESS;

  // Do we have a dst rect shared map?
  e = mapGetNestedMapP(sharedGeneMPMP, DST_RECT, &dstRectMP);
  if (e == E_BAD_KEY) {
    return SUCCESS;  // Hmmm... No dest rects, huh? Let's assume the user knows what they're doing.
  }
  else if (!e && dstRectMP) {
    U32 nInserts = 0;
    for (Entity entity = 1; !e && spawnP < spawnEndP; ++spawnP) {
      posP = spawnP->positionNodeA;
      posEndP = posP + spawnP->nEntitiesPossible;
      for (; !e && entity <= biomeP->nEntitiesToSpawn && posP < posEndP; ++posP, ++entity) {
        // If this entity has a dest rect component, place it where it belongs.
        if (!spawnP->keyP || *spawnP->keyP == posP->keyhole) {
          // Set dst rect
          dstRect.x = posP->position.x;
          dstRect.y = posP->position.y;
          e = mapSet(dstRectMP, entity, &dstRect);
          ++nInserts;
        }
      }
    }
  }
  
  return e;
}

static void _mutationMapArrayDel(Biome *biomeP) {
  Spawn *spawnP = biomeP->spawnA;
  Spawn *spawnEndP = spawnP + biomeP->nSpawns;
  for (; spawnP < spawnEndP; ++spawnP) {
    arrayDel((void**) &spawnP->geneMutationMPA);
  }
}

static Error _makeMutationMapArrays(Biome *biomeP, GeneHisto *geneHistoP, Key nSystemsMax) {
  Key nMutationsForCurrGene;
  Gene **genePP;
  Gene **geneEndPP;
  Gene gene;
  Map *currSpawnMutationMP;

  Spawn *spawnP = biomeP->spawnA;
  Spawn *spawnEndP = spawnP + biomeP->nSpawns;   // pointer to the end of the above array

  Error e = SUCCESS;

  /* For each spawn, we have an array of mutation maps: one for each system.
   * Each mutation map has any number of mutations for the entity's component in that system.
   * First we allocate all the non-empty maps. 
   * Then we populate each map one gene at a time.
   */
  for (; !e && spawnP < spawnEndP; ++spawnP) {
    e = arrayNew((void**) &spawnP->geneMutationMPA, sizeof(Map*), nSystemsMax);
    // Allocate a map for each mutable gene for this spawn.
    for (Key systemId = 1; !e && systemId < nSystemsMax; ++systemId) {
      // Indexing a 2D array (x dimension: # of systems; y dimension: # of entities)
      nMutationsForCurrGene = geneHistoP->nMutationsPerSpawnAA[(spawnP - biomeP->spawnA) * nSystemsMax + systemId];
      if (nMutationsForCurrGene) {
        e = mapNew(&spawnP->geneMutationMPA[systemId], 
                   RAW_DATA,
                   geneHistoP->nExclusivesA[systemId].size,  // TODO replace this with mutation size
                   nMutationsForCurrGene);
      }
    }
    // Populate all of this spawn's maps of gene mutations.
    genePP = spawnP->genomeP->genePA;
    geneEndPP = genePP + spawnP->genomeP->nGenes;
    // genePP is a pointer to a pointer to a global singleton of a component
    for (; !e && genePP < geneEndPP; genePP++) {  
      gene = **genePP;
      if (gene.geneClass == EXCLUSIVE_GENE) {
        currSpawnMutationMP = spawnP->geneMutationMPA[gene.u.unitary.type & MASK_COMPONENT_TYPE];
        // Trusting data's been intialized by now, because it's being full-copied over.
        e = mapSet(currSpawnMutationMP, gene.u.unitary.key, gene.u.unitary.dataP);
      }  // add this gene to this spawn's mutation map
    }  // for each gene in this spawn's genome
  }  // for each spawn 
  
  return e;
}

// Inflate a whole array of strip data.
static Error _biomeMediaInflate(StripDataS **sdPF) {
  if (!sdPF) {
    return E_BAD_ARGS;
  }
  // Inflate colormap inflatables
#if MULTITHREADED
  Error e = multithread_(sdInflate, (void*) sdPF);
  // Unpack stripsets
  if (!e) {
    e = multithread_(sdUnpack, (void*) sdPF);
  }
  // Assemble colormaps from strips
  if (!e) {
    e = multithread_(sdAssemble, (void*) sdPF);
  }
#else 
  for (int i = 0; !e && i < N_SAMPLES; ++i) {
    e = sdIni(sdPF[i]);
  }
#endif
  return e;
}

static Error _distributeGene(
    Entity entity,
    Gene **genePP, 
    StripDataS **sdPF, 
    System *masterSysP,
    Map *sharedGenesMPMP,
    Map *bbMP,
    Spawn *spawnP) {
  Gene gene = **genePP;
  XMasterComp *childSysPP = NULL;
  Map *innerMapP;
  Gene **compositeGenePP;
  Gene **compositeGeneEndPP;

  Error e = SUCCESS;
  switch (gene.geneClass) {
    case COMPOSITE_GENE:
      compositeGenePP = gene.u.composite.genePA;
      compositeGeneEndPP = compositeGenePP + gene.u.composite.nGenes;
      for (; compositeGenePP < compositeGeneEndPP; ++ compositeGenePP) {
        e = _distributeGene(entity, compositeGenePP, sdPF, masterSysP, sharedGenesMPMP, bbMP, spawnP);
      }
      break;
    case MEDIA_GENE:
      // dataP is a pointer to an object that in turn has another pointer to a strip data.
      // frayAdd() copies the CONTENTS of the passed-in pointer, so it works out beautifully.
      if (!((*((StripDataS**) gene.u.unitary.dataP))->flags & SD_SET_FOR_INFLATION_)) {
        (*((StripDataS**) gene.u.unitary.dataP))->flags |= SD_SET_FOR_INFLATION_;
        e = frayAdd(sdPF, gene.u.unitary.dataP, NULL);
      }
      if (e) {
        break;
      }
    // Fall through to next case to distribute media gene to the system.
    // These media are still compressed at this point, but it'll be inflated after distro'ing.
    case EXCLUSIVE_GENE:
      childSysPP = (XMasterComp*) xGetCompPByEntity(masterSysP, gene.u.unitary.type & MASK_COMPONENT_TYPE);
      if (childSysPP) {
        e = xAddEntityData(*childSysPP, entity, gene.u.unitary.type, gene.u.unitary.dataP);
        if (!e) {
          e = xAddMutationMap(*childSysPP, entity, spawnP->geneMutationMPA[gene.u.unitary.type & MASK_COMPONENT_TYPE]);
        }
      }
      break;
    // Shared objects are kept by the master system. 
    // Child systems who want to use them can get their pointers later.
    case SHARED_GENE: 
      // Outer map is a map of map pointers. The key to it is the enumerated type of shared object.
      // Inner map knows how big gene's header's container is.
      e = mapGetNestedMapP(sharedGenesMPMP, gene.u.unitary.type & MASK_COMPONENT_TYPE, &innerMapP);
      // Inner map is a map of components. 
      // Map knows how big gene's header's container is.
      if (!e) {
        for (Entity entityEnd = entity + spawnP->nEntitiesToSpawn; entity < entityEnd; ++entity) {
          e = mapSet(innerMapP, entity, (const void*) &gene);  
        }
      }
      break;
    // Media are the only things needing inflating and unpacking.
    default:
      e = E_BAD_COMPONENT_TYPE;
      break;
  }

  return e;
}

static Error _postProcessChildrenSystems(System *masterSysP) {
  Error e = SUCCESS;
  System **childSysPP = masterSysP->cF;
  System **childSysEndPP = childSysPP + xGetNComps(masterSysP);
  for (; !e && childSysPP < childSysEndPP; ++childSysPP) {
    e = (*childSysPP)->postprocessComps(*childSysPP);
  }
  return e;
}

static Error _xMasterForwardMail(XMaster *xP, System *writerSysP) {
  Message *msgP = writerSysP->mailboxF;
  Message *msgEndP = msgP + *_frayGetFirstEmptyIdxP(writerSysP->mailboxF);

  Error e = SUCCESS;

  for (; !e && msgP < msgEndP; ++msgP) {
    XMasterComp *cP = (XMasterComp*) xGetCompPByEntity(&xP->system, msgP->address);
    /* Mailboxes are both input and output. If this mailbox has received forwarded
       messages already, we don't want to re-forward it to the same recipient. */
    if (cP) {  // If a letter is bound to a nonexistent recipient, just drop it on the floor.
      if (writerSysP->id != msgP->address) {
        e = mailboxForward((*cP)->mailboxF, msgP);
      }
    }
  }

  return e;
}

// =====================================================================
// Distribute all genes to their appropriate subsystems.
// =====================================================================
static Error _distributeGenes(XMaster *xP, Key nSystemsMax) {
  if (!xP || !nSystemsMax || !xP->biomeP || !xP->biomeP->spawnA) {
    return E_BAD_ARGS;
  }
  Spawn *spawnP = xP->biomeP->spawnA;
  Spawn *spawnEndP = spawnP + xP->biomeP->nSpawns;   // pointer to the end of the above array
  Genome *genomeP;
  Gene **genePP, **geneEndPP;   // pointers to an array of gene header pointers and its end
  Map *bbMP = NULL;
  GeneHisto geneHisto = {0};
  StripDataS **sdPF = NULL;

  _countEntitiesToSpawn(xP->biomeP);

  Error e = _geneHistoIni(&geneHisto, xP->biomeP->nEntitiesToSpawn, nSystemsMax);
  if (!e) {
    e = _histoGenes(&geneHisto, xP->biomeP, nSystemsMax);
  }
  if (!e) {
    e = _subsystemsIni(&xP->system, &geneHisto);
  }
  if (!e) {
    e = _makeSharedGeneMap(&xP->sharedMPMP, &geneHisto);
  }
  if (!e) {
    e = _addSpecialSharedGene(xP->sharedMPMP, xP->guiP, GUI_GENE_TYPE);
  }

  if (!e) {
    e = _spawnEntities(xP->sharedMPMP, xP->biomeP);
  }
  if (!e) {
    e = _makeMutationMapArrays(xP->biomeP, &geneHisto, nSystemsMax);
  }
  if (!e) {
    e = frayNew((void**) &sdPF, sizeof(StripDataS*), geneHisto.nDistinctMedia);
  }

  // Distribute genes to all spawned entties.
  for (Entity entity = 1; !e && spawnP < spawnEndP; ++spawnP) {
    for (Entity entityEnd = entity + spawnP->nEntitiesToSpawn; entity < entityEnd; ++entity) {
      // Make a blackboard map for this entity if it has any genes to put in it.
      bbMP = NULL;
      genomeP = spawnP->genomeP;
      genePP = spawnP->genomeP->genePA;
      geneEndPP = genePP + genomeP->nGenes;
      // For each gene of this spawn...
      for (; !e && genePP < geneEndPP; genePP++) {  // genePP is a pointer to a pointer to a global singleton of a component
        e = _distributeGene(entity, genePP, sdPF, &xP->system, xP->sharedMPMP, bbMP, spawnP);
      }
      // If this entity has a blackboard, stick it into the map of entity blackboards.
      if (!e && bbMP) {
        e = mapSet(xP->system.mutationMPMP, entity, &bbMP);
      }
    }
  }

  // Inflate all media-type genes. All components using them will see it from inside their systems.
  if (!e) {
    e = _biomeMediaInflate(sdPF);
  }
  // Shared genes can't be distributed till components using them exist.
  // Some systems like xRender need to wait till post-processing to make their components.
  // So we're only guaranteed components' existence after post-processing.
  if (!e) {
    e = _distributeSharedGenesToSubsystems(&xP->system, xP->sharedMPMP);
  }
  // Post-process all children systems.
  // Some systems using composite genes don't make components until they have all their ingredients.
  if (!e) {
    e = _postProcessChildrenSystems(&xP->system);
  }
  // Clean up your strip data
  if (sdPF) {
    // Ignore return error here to avoid changing outer return. We want to clean up no matter what.
    multithread_(stripClr, (void*) sdPF);
    frayDel((void**) &sdPF);
  }

  // TODO get rid of this after impl'ing behavior s ystem
  if (!e) {
    _frayActivateAll(&xP->system.cF);
  }

  _mutationMapArrayDel(xP->biomeP);
  _geneHistoClr(&geneHisto);
  return e;
}

XPostMutateFuncDefUnused_(Master);

// =====================================================================
// Initialize the Master system.
// =====================================================================
Error xMasterIniSys(System *sP, void *sParamsP) {
  XMaster *xMasterSysP = (XMaster*) sP;
  // Check args
  if (!sP || !sParamsP) {
    return E_BAD_ARGS;
  }

  XMasterIniSysPrms *xMasterIniSysPrmsP = (XMasterIniSysPrms*) sParamsP;

  if (!xMasterIniSysPrmsP->biomeP || !xMasterIniSysPrmsP->xSysPA) {
    return E_BAD_ARGS;
  }

  xMasterSysP->biomeP = xMasterIniSysPrmsP->biomeP;
 
  // Initialize the GUI through whatever library we're using.
  Error e = guiNew(&xMasterSysP->guiP);

  if (xMasterIniSysPrmsP) {
    for (U32 i = 0; !e && i < xMasterIniSysPrmsP->nXSystems; i++) {
      if (xMasterIniSysPrmsP->xSysPA[i]) {
        e = xAddEntityData(sP, xMasterIniSysPrmsP->xSysPA[i]->id, sP->id, (void**) &xMasterIniSysPrmsP->xSysPA[i]);
      }
    }
  }

  // Distribute biome's genomes to entities.
  if (!e) {
    e = _distributeGenes(xMasterSysP, xMasterIniSysPrmsP->nXSystemsMax);
  }

  // Forward all mail that may have been written by this time.
  if (!e) {
    XMaster *xP = (XMaster*) sP;

    XMasterComp *cP = sP->cF;
    XMasterComp *cEndP = cP + *_frayGetFirstEmptyIdxP(sP->cF);

    // If this child system has any mail to send, send it.
    for (; !e && cP < cEndP; ++cP) {
      if (*_frayGetFirstEmptyIdxP((*cP)->mailboxF)) {
        e = _xMasterForwardMail(xP, *cP);
      }
    }
  }

  // Avoid moving things by cheating and telling the fray everything's active.
  if (!e) {
    _frayActivateAll(sP->cF);
  }


  return e;
}

/* xIni() initializes the parent system as well as its children. */
Error xMasterIni(XMaster *xMasterSysP, System **sPA, U16 nXSystems, Key nXSystemsMax, Biome *biomeP) {
  if (!sPA || nXSystems < 1 || !biomeP) {
    return E_BAD_ARGS;
  }

  XMasterIniSysPrms xMasterSysIniPrms = {
    .nXSystemsMax = nXSystemsMax, // + 1 for "go"-system
    .nXSystems = nXSystems,
    .biomeP = biomeP,
    .xSysPA = sPA,
  };

  return xIniSys(&xMasterSysP->system, xMasterSysIniPrms.nXSystems, (void*) &xMasterSysIniPrms);
}

Error xMasterRun(System *sP) {
  XMaster *xP = (XMaster*) sP;

  Error e = guiProcessEvents(xP->guiP);
  if (e == E_PAUSE) {
    _frayPauseAll(sP->cF);  // pause all systems
    return SUCCESS;
  }
  else if (e == E_UNPAUSE) {
    _frayUnpauseAll(sP->cF);
  }
  else if (e == E_QUIT) {
    return E_QUIT;
  }

  e = SUCCESS;
  XMasterComp *cP = sP->cF;
  XMasterComp *cEndP = cP + _frayGetFirstPausedIdx(sP->cF);
  for (; !e && cP < cEndP; ++cP) {
    e = xRun(*cP);  // cP is a pointer to XMasterComp, which itself is also a pointer.
    if (*_frayGetFirstEmptyIdxP((*cP)->mailboxF)) {
      e = _xMasterForwardMail(xP, *cP);
    }
  }
  return e;
}
