#include "x.h"

// Copied these from data.c since I don't know how to inline across files.
inline static Key* _getCompIdxPByEntity(System *sP, Entity entity) {
  return (Key*) mapGet(sP->e2cIdxMP, entity);
}

inline static Entity _getEntityByCompIdx(System *sP, Key compIdx) {
  return sP->cIdx2eA[compIdx];
}

void* xGetCompPByEntity(System *sP, Entity entity) {
  if (!sP || !entity) {
    return NULL;
  }
  Key *elemIdxP = _getCompIdxPByEntity(sP, entity);
  if (!elemIdxP) {
    return NULL;
  }
	return (void*) ((U8*) sP->cF + *elemIdxP * arrayGetElemSz(sP->cF));
}

Entity xGetEntityByCompIdx(System *sP, Key compIdx) {
  return _getEntityByCompIdx(sP, compIdx);
}

inline static void __xSwap(System *sP, S32 origIdx, S32 newIdx) {
  assert (sP && newIdx >= 0 && origIdx >= 0);
  if (origIdx != newIdx) {
    Entity entity1 = _getEntityByCompIdx(sP, origIdx);
    Entity entity2 = _getEntityByCompIdx(sP, newIdx);
    Key *origIdxP  = (Key*) mapGet(sP->e2cIdxMP, entity1);
    Key *newIdxP   = (Key*) mapGet(sP->e2cIdxMP, entity2);
    assert(origIdxP && newIdxP);
    swap_(sP->cIdx2eA[newIdx], sP->cIdx2eA[*origIdxP]);
    swap_(*origIdxP, *newIdxP);
  }
}

static void _xSwap(System *sP, FrayChanges *changesP) {
  // If nothing changed, just return success; e.g. activating an element already in index 0
  if (changesP->newIdx != changesP->origIdx) {
    // Existence of paused elements requires an additional, intermediate swap.
    if (changesP->newIdx >= 0) {
      if (changesP->intermediateIdx == 0) {
        __xSwap(sP, changesP->origIdx, changesP->newIdx);
      }
      else {
        __xSwap(sP, changesP->origIdx, changesP->intermediateIdx);
        __xSwap(sP, changesP->intermediateIdx, changesP->newIdx);
      }
    }
  }
}

void xActivateComponentByEntity(System *sP, Entity entity) {
  assert (sP && entity);
  // We use a pointer instead of a new one to swap its actual location later.
  FrayChanges changes;
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, entity);
  assert(compOrigIdxP);
  frayChangesIni_(changes, *compOrigIdxP);
  frayActivate(sP->cF, *compOrigIdxP, &changes);
  _xSwap(sP, &changes);
}

void xDeactivateComponentByEntity(System *sP, Entity entity) {
  assert (sP && entity);
  FrayChanges changes;
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, entity);
  assert (compOrigIdxP);
  frayChangesIni_(changes, *compOrigIdxP);
  frayDeactivate(sP->cF, *compOrigIdxP, &changes);
  _xSwap(sP, &changes);
}

void xPauseComponentByEntity(System *sP, Entity entity) {
  assert (sP && entity);
  FrayChanges changes;
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, entity);
  assert (compOrigIdxP);
  frayChangesIni_(changes, *compOrigIdxP);
  frayPause(sP->cF, *compOrigIdxP, &changes);
  _xSwap(sP, &changes);
}

void xUnpauseComponentByEntity(System *sP, Entity entity) {
  assert (sP && entity);
  FrayChanges changes;
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, entity);
  assert (compOrigIdxP);
  frayChangesIni_(changes, *compOrigIdxP);
  frayUnpause(sP->cF, *compOrigIdxP, &changes);
  _xSwap(sP, &changes);
}

U32 xGetNComps(System *sP) {
  assert(sP && sP->cF);
	return arrayGetNElems(sP->cF);
}

// Insert inner mutation map (maps trigger to mutation) into outer mutation map (maps Entity to Map*)
void xAddMutationMap(System *sP, Entity entity, Map *mutationMP) {
  assert(entity &&  sP);  // null mutation map is okay
  // If the component is immutable, that's fine, don't worry about it. 
  // If the user intended to mutate the gene, ensure we have both pieces of data.
  if (mutationMP && sP->mutationMPMP) {
    assert(arrayGetElemSz(mutationMP->mapA) == sP->mutationSz);
    return mapSet(sP->mutationMPMP, entity, &mutationMP);
  }
  // Otherwise just return successfully, assuming they never intended to mutate this entity's component in the first place.
}


void xAddComp(System *sP, Entity entity, void *compDataP) {
  assert (sP &&  entity);
  // Skip entities who already have a component in this system.
  if (!mapGet(sP->e2cIdxMP, entity)) {
    // Put component in first empty slot. (Will be zeros if this is a map. That's okay.)
    U32 cIdx; 
    if (compDataP) {
      frayAdd(sP->cF, compDataP, &cIdx); 
    }
    else {
      frayAddEmpty(sP->cF, &cIdx);
    }
    // Add lookups from C -> E and E -> C.
    mapSet(sP->e2cIdxMP, entity, &cIdx);
    sP->cIdx2eA[cIdx] = entity;
  }
}

inline static void _iniSubcompOwner(System *sP, Entity entity, Key subcompType, void *dataP) {
  // If subcomponent type doesn't fall into subcomponent mask
  assert ((subcompType & MASK_COMPONENT_SUBTYPE) == subcompType);
  SubcompOwner newSubcompOwner = {0};
  newSubcompOwner.owner = entity;
  newSubcompOwner.subcompA[getSubcompIdx_(subcompType)] = dataP;
  mapSet(sP->subcompOwnerMP, entity, &newSubcompOwner);
}


void xAddEntityData(System *sP, Entity entity, Key compType, void *entityDataP) {
  assert (sP &&  entity &&  compType);   // entityDataP is allowed to be null so xAddComp() can call frayAddEmpty() in such a case.
  // Make sure the component belongs to this system. This is only checked at load-time.
  assert ((compType & MASK_COMPONENT_TYPE) == sP->id);
  // If upper two bits are nonzero, this is a subcomponent. 
  if (compType & MASK_COMPONENT_SUBTYPE) {
    // If entity doesn't own any subcomponents yet, make a slot for it in subcomp ownership map.
    SubcompOwner *subcompOwnerP = mapGet(sP->subcompOwnerMP, entity);
    Key subcompType = compType & MASK_COMPONENT_SUBTYPE;
    if (!subcompOwnerP) {
      _iniSubcompOwner(sP, entity, subcompType, entityDataP);
    }
    // Otherwise, update its existing onwership record.
    else {
      subcompOwnerP->subcompA[getSubcompIdx_(subcompType)] = entityDataP;
    }
    // Now you can operate on the subcomponent in the system carefree.
    sP->iniSubcomp(sP, entity, compType & MASK_COMPONENT_SUBTYPE, entityDataP);
    /* Let's take the burden of adding components off our individual systems 
     * (unless they insist otherwise with their flags) so they only have to 
     * worry about mutations. Some systems will want to do so in postProcess(). */
    // TODO was this a mistake?
    //if (!e && ) {
      //e = xAddComp(sP, entity, NULL);
    //} 
  }
  // Else it's the main component; feed it straight in baby.
  else if (!(sP->flags & FLG_DONT_ADD_COMP)) {
    xAddComp(sP, entity, entityDataP);
  }
}

void xIniSys(System *sP, U32 nComps, void *miscP) {
  // Sytems with special parts need to initialize maps in sIniU().
  sP->cF = frayNew(sP->compSz, nComps);
  sP->cIdx2eA = arrayNew(sizeof(Entity), nComps);
  sP->deactivateQueueF = frayNew(sizeof(Entity), nComps);
  sP->pauseQueueF = frayNew( sizeof(Entity), nComps);
  sP->e2cIdxMP = mapNew( RAW_DATA, sizeof(Key), nComps);
  if (!(sP->flags & FLG_NO_MUTATIONS_) && sP->mutationSz) {
    sP->mutationMPMP = mapNew( MAP_POINTER, sizeof(Map*), nComps);
  }
	// Only allocate one mailbox; it serves as input and output.
  // Also, give it ample room to handle multiple messages per entity.
#define MAILBOX_MULTIPLY_NUM_SLOTS (3)
  sP->mailboxF = frayNew(sizeof(Message), nComps * MAILBOX_MULTIPLY_NUM_SLOTS );
  sP->subcompOwnerMP = mapNew(RAW_DATA, sizeof(SubcompOwner), nComps);
  // Finally, call the system's unique initializer.
  (*sP->iniSys)(sP, miscP);  // fail-assert if this bombs
}

// Don't erase everything in a system. Some things should be permanent.
void xClr(System *sP) {
  if (sP) {
    sP->clr(sP);  // This MUST run first as it may rely on things we're about to erase
    frayDel((void**) &sP->cF);
    frayDel((void**) &sP->mailboxF);
    frayDel((void**) &sP->deactivateQueueF);
    frayDel((void**) &sP->pauseQueueF);
    mapDel(&sP->subcompOwnerMP);
    mapDel(&sP->e2cIdxMP);
    // Don't delete inner maps as there may be multiple copies pointing at the same thing.
    // You're required to have deleted or acquired a pointer to the mutation map's inner maps by this point.
    //mapOfNestedMapsDel(&sP->mutationMPMP);  <-- don't do this.  
    mapDel(&sP->mutationMPMP);
    arrayDel((void**) &sP->cIdx2eA);
  }
}

void xMutateComponent(System *sP, Entity entity, Key newCompKey) {
  // Make sure entity, system, and the key to the new component are all valid parameters.
  assert (sP &&  entity &&  newCompKey);
  // Make sure the system was set up for mutations in the first place.
  if (!(sP->flags & FLG_NO_MUTATIONS_)) {
    // Get the nested map of mutations for this particular entity.
    Map *mutationMP = mapGetNestedMapP(sP->mutationMPMP, entity);
    // Ensure the nested map has raw data-- nothing funky like pointers or (more) inner maps.
    assert(mutationMP->elemType == RAW_DATA);
    // Get a pointer to the entity's component.
    void* cP = xGetCompPByEntity(sP, entity);
    if (cP) {
      // Get a pointer to the mutation.
      void *tmpP = mapGet(mutationMP, newCompKey);
      if (tmpP) {
        // Mutate the only part of the component that should change.
        memcpy((U8*) cP + sP->mutationOffset, tmpP, arrayGetElemSz(mutationMP->mapA));
        return sP->postMutate(sP, cP);
      }
      // It's a design decision to not error out when mutation doesn't exist. 
    }
  }
}

static void _xReadInbox(System *sP) {
  assert (sP &&  sP->mailboxF);
  Message *msgP = sP->mailboxF;
  Message *msgEndP = msgP + *_frayGetFirstEmptyIdxP(sP->mailboxF);
  for (; msgP < msgEndP; msgP++) {
    assert (msgP->address == sP->id);
    switch(msgP->cmd) {
      case MUTATE_AND_ACTIVATE: 
        xMutateComponent(sP, msgP->attn, msgP->arg);
      case ACTIVATE:
        xActivateComponentByEntity(sP, msgP->attn);
        break;
      case MUTATE_AND_DEACTIVATE: 
        xMutateComponent(sP, msgP->attn, msgP->arg);
      case DEACTIVATE:
        xDeactivateComponentByEntity(sP, msgP->attn);
        break;
      case PAUSE:
        xPauseComponentByEntity(sP, msgP->attn);
        break;
      case UNPAUSE:
        xUnpauseComponentByEntity(sP, msgP->attn);
        break;
      case MUTATE:
        xMutateComponent(sP, msgP->attn, msgP->arg);
        break;
      default:
        sP->processMessage(sP, msgP);  // some systems require more specific treatment
        break;
    }
  }
  _frayClr(sP->mailboxF);
}

static void _pauseQueue(System *sP) {
  assert (sP);
  U32 nPausedEntities = *_frayGetFirstEmptyIdxP(sP->pauseQueueF);
  if (nPausedEntities) {
    Entity *entityP = sP->pauseQueueF;
    Entity *entityEndP = entityP + nPausedEntities;
    for (; entityP < entityEndP; ++entityP) {
      xPauseComponentByEntity(sP, *entityP);
    }
    _frayClr(sP->pauseQueueF);
  }
}

static void _deactivateQueue(System *sP) {
  assert (sP);
  U32 nDeactivatedEntities = *_frayGetFirstEmptyIdxP(sP->deactivateQueueF);
  if (nDeactivatedEntities) {
    Entity *entityP = sP->deactivateQueueF;
    Entity *entityEndP = entityP + nDeactivatedEntities;
    for (; entityP < entityEndP; ++entityP) {
      xDeactivateComponentByEntity(sP, *entityP);
    }
    _frayClr(sP->deactivateQueueF);
  }
}

Entity xGetEntityByVoidComponentPtr(System *sP, void *componentP) {
  assert (sP && componentP);
  Entity compIdx = ((U32) componentP - (U32) sP->cF) / sP->compSz;
  return xGetEntityByCompIdx(sP, compIdx);
}


void xQueuePause(System *sP, void *componentP) {
  Entity entity = xGetEntityByVoidComponentPtr(sP, componentP);
  frayAdd(sP->pauseQueueF, &entity, NULL);
}

void xQueueDeactivate(System *sP, void *componentP) {
  Entity entity = xGetEntityByVoidComponentPtr(sP, componentP);
  frayAdd(sP->deactivateQueueF, &entity, NULL);
}

/* This is how the entire ECS framework works. */
void xRun(System *sP) {
  _xReadInbox(sP);
  sP->run(sP);
  _deactivateQueue(sP);
  _pauseQueue(sP);
}
