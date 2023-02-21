#include "biome.h"

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
    if (spawnP->keyP)
      for (; posNodeP < posNodeEndP; ++posNodeP)
        spawnP->nEntitiesToSpawn += (*spawnP->keyP == posNodeP->keyhole);
    else
      spawnP->nEntitiesToSpawn = spawnP->nEntitiesPossible;

    biomeP->nEntitiesToSpawn += spawnP->nEntitiesToSpawn;
  }
}

/*************************/
/*****  HISTOGRAMS  ******/
/*************************/
static Error _histoHivemindTriggers(U32 **histoAP, Biome *biomeP, U32 *totalDistinctHivemindsP) {
  // An  N-byte number can represent 1 << (N * 8) values.
  U32 N = 1 << (sizeof(Key) * 8);
  Error e = histoNew(histoAP, N);
  U32 *histoA = NULL;

  if (!e) {
    histoA = *histoAP;
    Spawn *spawnP = biomeP->spawnA;
    Spawn *spawnEndP = spawnP + biomeP->nSpawns;
    for (; spawnP < spawnEndP; spawnP++) {
      Personality *personalityP = spawnP->seedP->personalityP;
      Quirk *quirkP = personalityP->quirkA;
      Quirk *quirkEndP = quirkP + personalityP->nQuirks;
      for (; quirkP < quirkEndP; ++quirkP) {
        *totalDistinctHivemindsP += (!histoA[quirkP->triggerKey]);
        histoA[quirkP->triggerKey] += spawnP->nEntitiesToSpawn;
      }
    }
  }

  if (e && histoA)
    histoDel(histoAP);

  return e;
}

/*
 * Okay, so what's the relationship between compoiste genes and systems' populations?
 * Body has ?animation, ?collsion, ?collision palette, colormap, and color palette.
 * That means I have to dig in, avoid false positives, and multi-counting.
 * But I have to do so while remaining agnostic to the types of sytems out there.
 *
 *
 */

static Error _histoGene(
    Gene **genePP, 
    const Biome *biomeP, 
    const U32 nSystemsMax,
    Entity *entityP, 
    Spawn *spawnP, 
    GeneHisto *geneHistoP) {
  Error e = SUCCESS;

  XHistoElem *histoA = geneHistoP->histoXElemA;
  XHistoElem *xHistoElemP = NULL;
  Key *spawnMutationHistoA = geneHistoP->histoSpawnMutations;
  Key mutationHistoElemIdx;

  StripDataS *sdP;

  Gene gene = **genePP;
  xHistoElemP = &histoA[gene.u.unitary.type];
  switch (gene.geneClass) {
    case SHARED_GENE:
      geneHistoP->nDistinctShareds += !xHistoElemP->count;
      // Don't break; the logic in exclusive gene case applies too.
    case EXCLUSIVE_GENE:
      // Systems might consume multiple genes per component-- "sub"-components, if you will.
      // For example, XRender needs to consume both a colormap and color palette for each component.
      // But we don't want to interpret multiple pieces as implying multiple entities.
      // So, to sidestep that, we only count when the MASK_COMPONENT_SUBTYPE portion is 0.
      // That implies this is the first of up to four ingredients for a component.
      if (!(gene.u.unitary.type & MASK_COMPONENT_SUBTYPE)) {
        xHistoElemP->count += spawnP->nEntitiesToSpawn;
        xHistoElemP->size = gene.u.unitary.size;
        xHistoElemP->geneClass = gene.geneClass;
        xHistoElemP->geneType = gene.u.unitary.type & MASK_COMPONENT_TYPE;  // used to find system
        *entityP += spawnP->nEntitiesToSpawn;
        if (gene.u.unitary.key) {
          mutationHistoElemIdx = (spawnP - biomeP->spawnA) * nSystemsMax + gene.u.unitary.type;
          ++spawnMutationHistoA[mutationHistoElemIdx];
        }
      }
      break;
    case BB_GENE:
      for (Entity entityEnd = *entityP + spawnP->nEntitiesToSpawn; *entityP < entityEnd; ++*entityP){
        ++(geneHistoP->histoBbElemA[*entityP]);
      }
      break;
    case MEDIA_GENE:
      sdP = (StripDataS*) gene.u.unitary.dataP;
      if (!(sdP->flags & SD_IS_COUNTED_)) {
        sdP->flags |= SD_IS_COUNTED_;
        ++geneHistoP->nDistinctMedia;
      }
      break;
    case COMPOSITE_GENE:
      for (int i = 0; i < gene.u.composite.nGenes; ++i) {
        _histoGene(&gene.u.composite.genePA[i], biomeP, nSystemsMax, entityP, spawnP, geneHistoP);
      }
      break;
    default:
      e = E_INVALID_GENE_CLASS;
      break;
  }
  return e;
}

static Error _histoGenes(GeneHisto *geneHistoP, const Biome *biomeP, const U32 nSystemsMax) {
  Error e = SUCCESS;
  if (!geneHistoP || !geneHistoP->histoXElemA || !biomeP)
    return E_BAD_ARGS;

  Genome *genomeP;
  Gene **genePP, **geneEndPP;   // pointers to an array of gene header pointers and its end

  Spawn *spawnP = biomeP->spawnA;
  Spawn *spawnEndP = spawnP + biomeP->nSpawns;   // pointer to the end of the above array
  // Loop through genomes
  for (Entity entity = 0; !e && spawnP < spawnEndP; spawnP++) {
    genomeP = spawnP->seedP->genomeP;
    genePP = &genomeP->genePA[0];
    geneEndPP = genePP + genomeP->nGenes;
    // Histo genome's genes that aren't blackboard items.
    for (; !e && genePP < geneEndPP; genePP++) {
      _histoGene(genePP, biomeP, nSystemsMax, &entity, spawnP, geneHistoP);
    }
  }
  return SUCCESS;
}

/***************************/
/*****  DISTRIBUTION  ******/
/***************************/
static Error _distributeHiveminds(Map **hivemindMPP, Biome *biomeP) {
  if (!hivemindMPP || !biomeP)
    return E_BAD_ARGS;

  U32 *histoA = NULL;  // determines how many entities listen for each trigger
  Entity **hivemindEntitiesAP = NULL;
  U32 nHiveminds = 0;
  Map *hivemindMP;
  // Histo the number of trees existing for each trigger.
  Error e = _histoHivemindTriggers(&histoA, biomeP, &nHiveminds);
  if (!nHiveminds || !histoA)
    goto nevermind;
  // Allocate hivemind map
  if (!e)
    e = mapNew(hivemindMPP, sizeof(Entity*), nHiveminds);
  // Allocate empty hiveminds.
  if (!e) {
    hivemindMP = *hivemindMPP;
    U32 nElems = arrayGetNElems(histoA);
    for (U32 trigger = 0; !e && trigger < nElems; ++trigger)
      if (histoA[trigger]) {
        Entity *entityA = NULL;
        e = arrayNew((void**) &entityA, sizeof(Entity), histoA[trigger]);
        if (!e) {
          // Put the array of entities subscribed to trigger "trigger" in the hivemind map.
          hivemindEntitiesAP = (Entity**) mapGet(hivemindMP, trigger);
          if (hivemindEntitiesAP)
            *hivemindEntitiesAP = entityA;
        }
      }
  }
  // Fill the hiveminds.
  if (!e) {
    Spawn *spawnP = biomeP->spawnA;
    Spawn *spawnEndP = spawnP + biomeP->nSpawns;
    for (Entity entity = 1; spawnP < spawnEndP; ++spawnP) {
      Personality *personalityP = spawnP->seedP->personalityP;
      Quirk *quirkP = personalityP->quirkA; 
      Quirk *quirkEndP = quirkP + personalityP->nQuirks;
      for (; quirkP < quirkEndP; ++quirkP) {
        // Get pointer to array of entities out of hivemind map.
        hivemindEntitiesAP = (Entity**) mapGet(hivemindMP, quirkP->triggerKey);
        // This is how we fill arrays without storing the current index of each one's next empty slot.
        if (hivemindEntitiesAP)
          for (Entity entityEnd = entity + spawnP->nEntitiesToSpawn; entity < entityEnd; ++entity)
            (*hivemindEntitiesAP)[--histoA[quirkP->triggerKey]] = entity;
      }
    }
  }

  nevermind:
  histoDel(&histoA);

  return e;
}

// Initialize subsystems before we put components in them. Only init ones with any components.
static Error _subsystemsIni(System *masterSysP, GeneHisto *geneHistoP) {
  Error e = SUCCESS;
  XHistoElem *histoElemP, *histoElemEndP;
  histoElemP = &geneHistoP->histoXElemA[0];
  histoElemEndP = histoElemP + arrayGetNElems(geneHistoP->histoXElemA);
  for (; !e && histoElemP < histoElemEndP; histoElemP++) 
    if (histoElemP->geneClass == EXCLUSIVE_GENE && histoElemP->count)  {  // i.e. if any entities exist having components for this system...
      System **_sPP = (System**) xGetCompPByEntity(masterSysP, histoElemP->geneType);
      if (_sPP)
        // Assume no subsystems need extra parameters.
        e = xIniSys(*_sPP, histoElemP->count, NULL);  
    }

  return e;
}

static Error _sharedGenesMapNew(Map **sharedGenesMPP, GeneHisto *geneHistoP) {
  // Create all the inner maps according to the histo'd number of elements they hold.
  Error e = mapNew(sharedGenesMPP, sizeof(Map*), geneHistoP->nDistinctShareds + 2);  // + 2 for window and renderer!
  if (!e) {
    Map *sharedGenesMP = *sharedGenesMPP;
    XHistoElem *xHistoElemP    = &geneHistoP->histoXElemA[0];
    XHistoElem *xHistoElemEndP = xHistoElemP + arrayGetNElems(geneHistoP->histoXElemA);
    Map *sharedGenesSubmapP = NULL;
    for (; !e && xHistoElemP < xHistoElemEndP; xHistoElemP++)
      if (xHistoElemP->geneClass == SHARED_GENE && xHistoElemP->count) {
        e = mapNew(&sharedGenesSubmapP, xHistoElemP->size, xHistoElemP->count);
        if (!e)
          e = mapSet(sharedGenesMP, xHistoElemP->geneType, (void**) &sharedGenesSubmapP);
      }
  }
  return e;
}

static Error _geneHistoIni(GeneHisto *geneHistoP, Entity nEntities, Entity nSystemsMax) {
  geneHistoP->nDistinctMedia   = 0;
  geneHistoP->nDistinctShareds = 0;
  Error e = arrayNew((void**) &geneHistoP->histoSpawnMutations, sizeof(Key), nSystemsMax * nEntities);
  if (!e) {
    e = arrayNew((void**) &geneHistoP->histoXElemA, sizeof(XHistoElem), nSystemsMax);
  }
  if (!e) {
    e = arrayNew((void**) &geneHistoP->histoBbElemA, sizeof(BbGeneHistoElem), KEY_MAX);
  }
  if (!e) {
    e = arrayNew((void**) &geneHistoP->histoCompositeElemA, sizeof(CompositeGeneHistoElem), KEY_MAX);
  }
  return e;
}

static void _geneHistoClr(GeneHisto *geneHistoP) {
  if (geneHistoP) {
    geneHistoP->nDistinctMedia   = 0;
    geneHistoP->nDistinctShareds = 0;
    arrayDel((void**) &geneHistoP->histoSpawnMutations);
    arrayDel((void**) &geneHistoP->histoBbElemA);
    arrayDel((void**) &geneHistoP->histoXElemA);
    arrayDel((void**) &geneHistoP->histoCompositeElemA);

  }
}

static void _distributeSharedGenesToSubsystems(System *masterSysP, Map *sharedGenesMP) {
  System **subSysPP = masterSysP->cF;
  System **subSysEndPP = subSysPP + arrayGetNElems((void*) masterSysP->cF);
  for (; subSysPP < subSysEndPP; subSysPP++)   
    if ((*subSysPP)->getShare)
      ((*subSysPP)->getShare)(*subSysPP, sharedGenesMP);
}

// This is for adding the window and renderer to master's map of shared genes.
static Error _addSpecialSharedGene(Map *sharedGeneMPMP, void *geneP, Key geneType) {
  Map *mapP = NULL;
  Error e = mapNew(&mapP, sizeof(void*), 1);
  if (!e)
    e = mapSet(mapP, 1, &geneP);  // only one window
  if (!e)
    e = mapSet(sharedGeneMPMP, geneType, (void**) &mapP);
  return e;
}

static void _mutationMapArrayDel(Biome *biomeP) {
  Spawn *spawnP = biomeP->spawnA;
  Spawn *spawnEndP = spawnP + biomeP->nSpawns;
  for (; spawnP < spawnEndP; ++spawnP) 
    if (spawnP->geneMutationMPA) {
      Map **mapPP = spawnP->geneMutationMPA;
      Map **mapEndPP = mapPP + arrayGetNElems(spawnP->geneMutationMPA);
      for (; mapPP < mapEndPP; ++mapPP) 
        mapDel(mapPP);
      arrayDel((void**) spawnP->geneMutationMPA);
    }
}

static Error _makeMutationMapArrays(Biome *biomeP, GeneHisto *geneHistoP, Key nSystemsMax) {
  Key nMutationsForCurrGene;
  Genome *genomeP;
  Gene **genePP;
  Gene **geneEndPP;
  Gene gene;
  Map *mutationMapP;

  Spawn *spawnP = biomeP->spawnA;
  Spawn *spawnEndP = spawnP + biomeP->nSpawns;   // pointer to the end of the above array

  Error e = SUCCESS;

  /* For each spawn, we have an array of mutation maps: one for each system.
   * Each mutation map has any number of mutations for the entity's component in that system.
   * First we must allocate all the maps. 
   * Then we must populate each map one gene at a time, given it's not null.
   */
  for (; !e && spawnP < spawnEndP; ++spawnP) {
    e = arrayNew((void**) &spawnP->geneMutationMPA, sizeof(Map*), nSystemsMax);
    // Allocate a map for each mutable gene for this spawn.
    for (Key systemTypeNum = 1; !e && systemTypeNum < nSystemsMax; ++systemTypeNum) {
      nMutationsForCurrGene = geneHistoP->histoSpawnMutations[(spawnP - biomeP->spawnA) * nSystemsMax + systemTypeNum];
      if (nMutationsForCurrGene) 
        e = mapNew(&spawnP->geneMutationMPA[systemTypeNum], 
                   geneHistoP->histoXElemA[systemTypeNum].size,
                   nMutationsForCurrGene);
    }
    genomeP = spawnP->seedP->genomeP;
    genePP = spawnP->seedP->genomeP->genePA;
    geneEndPP = genePP + genomeP->nGenes;
    // Populate all of this spawn's maps of gene mutations.
    for (; !e && genePP < geneEndPP; genePP++) {  // genePP is a pointer to a pointer to a global singleton of a component
      gene = **genePP;
      if (gene.geneClass == EXCLUSIVE_GENE) {
        mutationMapP = spawnP->geneMutationMPA[gene.u.unitary.type];  // current spawn's current mutation map
        // Trusting data's been intialized by now, because it's being full-copied over.
        e = mapSet(mutationMapP, gene.u.unitary.key, gene.u.unitary.dataP);
      }  // add this gene to this spawn's mutation map
    }  // for each gene in this spawn's genome
  }  // for each spawn 
  
  return e;
}

// =====================================================================
// Distribute all genes to their appropriate subsystems.
// =====================================================================
Error _distributeGenes(Biome *biomeP, System *masterSysP, Map **sharedGenesMPP, Window_ *windowP, Renderer_ *rendererP, Key nSystemsMax) {
  Spawn *spawnP = biomeP->spawnA;
  Spawn *spawnEndP = spawnP + biomeP->nSpawns;   // pointer to the end of the above array
  Genome *genomeP;
  Gene **genePP, **geneEndPP;   // pointers to an array of gene header pointers and its end
  Gene gene;  // faster access to double pointer'd gene header
  System *childSysP = NULL;
  Map *innerMapP = NULL;
  Map *bbMP = NULL;
  Map *sharedGenesMP;
  GeneHisto geneHisto = {0};
  if (!biomeP || !masterSysP || !sharedGenesMPP || !nSystemsMax)
    return E_BAD_ARGS;


  Error e = _geneHistoIni(&geneHisto, biomeP->nEntitiesToSpawn, nSystemsMax);
  if (!e) {
    e = _histoGenes(&geneHisto, biomeP, nSystemsMax);
  }
  if (!e) {
    e = _subsystemsIni(masterSysP, &geneHisto);
  }
  if (!e) {
    e = _sharedGenesMapNew(sharedGenesMPP, &geneHisto);
  }
  if (!e) {
    sharedGenesMP = *sharedGenesMPP;
    e = _addSpecialSharedGene(sharedGenesMP, windowP, 1);
  }
  if (!e) {
    e = _addSpecialSharedGene(sharedGenesMP, rendererP, 2);
  }
  if (!e) {
    e = _makeMutationMapArrays(biomeP, &geneHisto, nSystemsMax);
  }

  // For each spawn...
  for (Entity entity = 1; !e && spawnP < spawnEndP; ++spawnP, ++entity) {
    // Make a blackboard map for this entity if it has any genes to put in it.
    bbMP = NULL;
    if (geneHisto.histoBbElemA[entity]) {
      e = mapNew(&bbMP, sizeof(void*), geneHisto.histoBbElemA[entity]);
    }

    genomeP = spawnP->seedP->genomeP;
    genePP = spawnP->seedP->genomeP->genePA;
    geneEndPP = genePP + genomeP->nGenes;
    // For each gene of this spawn...
    for (; !e && genePP < geneEndPP; genePP++) {  // genePP is a pointer to a pointer to a global singleton of a component
      gene = **genePP;
      switch (gene.geneClass) {
        case EXCLUSIVE_GENE:
          childSysP = *((System**) xGetCompPByEntity(masterSysP, gene.u.unitary.type & MASK_COMPONENT_TYPE));
          if (childSysP) {
            // histo's 1D array represents a 2D array: columns are spawn #s, rows are system #s.
            // So we have to index it awkwardly as hell. :)
            for (Entity entityEnd = entity + spawnP->nEntitiesToSpawn; 
                 !e && entity < entityEnd; 
                 ++entity) {
              e = xAddComp(childSysP, entity, gene.u.unitary.type, gene.u.unitary.dataP, spawnP->geneMutationMPA[gene.u.unitary.type]);
            }
          }
          break;
        // Shared objects are kept by the master system. 
        // Child systems who want to use them can get their pointers later.
        case SHARED_GENE: 
          // Outer map is a map of map pointers. The key to it is the enumerated type of shared object.
          // Inner map knows how big gene's header's container is.
          innerMapP = (Map*) mapGet(sharedGenesMP, gene.u.unitary.type);  
          // Inner map is a map of components. 
          // Map knows how big gene's header's container is.
          if (innerMapP) {
            for (Entity entityEnd = entity + spawnP->nEntitiesToSpawn; entity < entityEnd; ++entity) {
              e = mapSet(innerMapP, entity, (const void*) &gene);  
            }
          }
          break;
        case BB_GENE:
          if (!e && !gene.u.unitary.dataP) {
            e = E_NULL_GENE_DATA;
          }
          // Fill up the blackboard with the gene pointers
          if (!e) {
            for (Entity entityEnd = entity + spawnP->nEntitiesToSpawn; entity < entityEnd; ++entity) {
              e = mapSet(bbMP, gene.u.unitary.type, &gene.u.unitary.dataP);
            }
          }
          break;
        // Media are the only things needing inflating and unpacking.
        case MEDIA_GENE:
          break;
        case COMPOSITE_GENE:
          break;
        default:
          e = E_BAD_COMPONENT_TYPE;
          break;
      }
    }
    // If this entity has a blackboard, stick it into the map of entity blackboards.
    if (!e && bbMP) 
      e = mapSet(masterSysP->switchMPMP, entity, &bbMP);
  }

  // Shared genes can't be distributed till components that use them exist in the first place.
  // Components don't exist until exclusive genes trigger an xAddComp() call.
  if (!e) 
    _distributeSharedGenesToSubsystems(masterSysP, sharedGenesMP);

  _mutationMapArrayDel(biomeP);
  _geneHistoClr(&geneHisto);
  return e;
}

static Error _distributeQuirks(Biome *biomeP, System *masterSysP, System *goSysP) {
  if (!biomeP) {
    return E_BAD_ARGS;
  }
  // Add all components to system.
  Personality *personalityP = NULL;
  Activity activity = {0};
  Map *activityMP = NULL;
  // Initialize Go system.
  Error e = xIniSys(goSysP, biomeP->nEntitiesToSpawn, NULL);
  // Init seed pointers.
  Spawn *spawnP = biomeP->spawnA;
  Spawn *spawnEndP = spawnP + biomeP->nSpawns;
  Activity emptyActivity = {0};
  // First, fill up XGo's map of gene-switches.
  for (Entity entity = 1; !e && spawnP < spawnEndP; ++spawnP, ++entity) {
    personalityP = spawnP->seedP->personalityP;
    // Because of this line, you must run _distributeGenes() before this.
    Map **bbMPP = (Map**) mapGet(masterSysP->switchMPMP, entity);
    // Allocate current entity's map of activities to switch to on different triggers.
    e = mapNew(&activityMP, sizeof(Activity), (personalityP->nQuirks));
    if (!e) {
      Quirk *quirkP = personalityP->quirkA;
      Quirk *quirkEndP = quirkP + personalityP->nQuirks;
      // Store each activity in the map of activities.
      for (; !e && quirkP < quirkEndP; ++quirkP) {
        activity.btP = quirkP->btP;
        e = btIni(activity.btP);  // Skips singleton trees that're already initialized.
        if (!e) {
          e = btStatNew(&activity.btStatP, activity.btP->rootP);
        }
        if (!e) {
          activity.priority = quirkP->priority;
          // Same blackboard pointer goes everywhere for easy memcpys.
          activity.bbMP = bbMPP ? *bbMPP : NULL;
          e = mapSet(activityMP, quirkP->triggerKey, &activity);
        }
      }
      // Start entity out with empty activity component while storing inner activity map in outer map.
      e = xAddComp(goSysP, entity, goSysP->id, &emptyActivity, activityMP);
    }
  }
  return e;
}

Error biomeExtract(Biome *biomeP, System *masterSysP, System *goSysP, Map **sharedGenesMPP, Window_ *windowP, Renderer_ *rendererP, Map **hivemindMPP, Key nSystemsMax) {
  _countEntitiesToSpawn(biomeP);
  Error e = _distributeGenes(biomeP, masterSysP, sharedGenesMPP, windowP, rendererP, nSystemsMax);
  if (!e) {
    e = _distributeQuirks(biomeP, masterSysP, goSysP);
  }
  if (!e) {
    e = _distributeHiveminds(hivemindMPP, biomeP);
  }
  return e;
}

void hivemindDel(Map **hivemindMPP) {
  if (hivemindMPP) {
    Map *hivemindMP = *hivemindMPP;
    if (hivemindMP && hivemindMP->mapA) {
      Entity **entityAP = hivemindMP->mapA;
      Entity **entityEndAP = hivemindMP->mapA + arrayGetNElems(hivemindMP->mapA);
      for (; entityAP < entityEndAP; entityAP++) {
        arrayDel((void**) entityAP);
      }
    }
  }
  mapDel(hivemindMPP);
}
/* TODO add postprocessing to the steps in  gene distribution
 *      \make x check only lower 6 bits for system ID match
 *      make biome do so also when picking out the system
 *      
 *
 */
