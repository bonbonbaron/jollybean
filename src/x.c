#include "x.h"
#include "data.h"

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

static Error __xSwap(System *sP, S32 origIdx, S32 newIdx) {
  if (!sP || newIdx < 0) {
    return E_BAD_ARGS;
  }
  if (origIdx == newIdx) {
    return SUCCESS;  // _swap() zeros out identical operands
  }
  Entity entity1 = _getEntityByCompIdx(sP, origIdx);
  Entity entity2 = _getEntityByCompIdx(sP, newIdx);
  Key *origIdxP  = (Key*) mapGet(sP->e2cIdxMP, entity1);
  Key *newIdxP   = (Key*) mapGet(sP->e2cIdxMP, entity2);
  if (!origIdxP || !newIdxP) {
    return E_ENTITY_NOT_IN_SYSTEM;
  }
  swap_(sP->cIdx2eA[newIdx], sP->cIdx2eA[*origIdxP]);
  swap_(*origIdxP, *newIdxP);
  return SUCCESS;
}

static Error _xSwap(System *sP, FrayChanges *changesP) {
  // If nothing changed, just return success; e.g. activating an element already in index 0
  if (changesP->newIdx == changesP->origIdx) {
    return SUCCESS;
  }
  Error e = SUCCESS;
  // Existence of paused elements requires an additional, intermediate swap.
  if (changesP->newIdx > 0) {
    if (changesP->intermediateIdx < 0) {
      e = __xSwap(sP, changesP->origIdx, changesP->newIdx);
    }
    else {
      e = __xSwap(sP, changesP->origIdx, changesP->intermediateIdx);
      if (!e) {
        e = __xSwap(sP, changesP->intermediateIdx, changesP->newIdx);
      }
    }
  }
  return e;
}

Error xActivateComponentByEntity(System *sP, Entity entity) {
  if (!sP || !entity) {
    return E_BAD_ARGS;
  }
  // We use a pointer instead of a new one to swap its actual location later.
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, entity);
  if (!compOrigIdxP) {
    return E_ENTITY_NOT_IN_SYSTEM;
  }
  FrayChanges changes;
  frayChangesIni_(changes, *compOrigIdxP);
  frayActivate(sP->cF, &changes);
  return _xSwap(sP, &changes);
}

Error xDeactivateComponentByEntity(System *sP, Entity entity) {
  if (!sP || !entity) {
    return E_BAD_ARGS;
  }
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, entity);
  if (!compOrigIdxP) {
    return E_ENTITY_NOT_IN_SYSTEM;
  }
  FrayChanges changes;
  frayChangesIni_(changes, *compOrigIdxP);
  frayDeactivate(sP->cF, &changes);
  return _xSwap(sP, &changes);
}

Error xPauseComponentByEntity(System *sP, Entity entity) {
  if (!sP || !entity) {
    return E_BAD_ARGS;
  }
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, entity);
  if (!compOrigIdxP) {
    return E_ENTITY_NOT_IN_SYSTEM;
  }
  FrayChanges changes;
  frayChangesIni_(changes, *compOrigIdxP);
  frayPause(sP->cF, &changes);
  return _xSwap(sP, &changes);
}

Error xUnpauseComponentByEntity(System *sP, Entity entity) {
  if (!sP || !entity) {
    return E_BAD_ARGS;
  }
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, entity);
  if (!compOrigIdxP) {
    return E_ENTITY_NOT_IN_SYSTEM;
  }
  FrayChanges changes;
  frayChangesIni_(changes, *compOrigIdxP);
  frayUnpause(sP->cF, &changes);
  return _xSwap(sP, &changes);
}

Error xActivateComponentByIdx(System *sP, Key compOrigIdx) {
  if (!sP) {
    return E_BAD_ARGS;
  }
  return xActivateComponentByEntity(sP, sP->cIdx2eA[compOrigIdx]);
}

Error xDeactivateComponentByIdx(System *sP, Key compOrigIdx) {
  if (!sP) {
    return E_BAD_ARGS;
  }
  return xDeactivateComponentByEntity(sP, sP->cIdx2eA[compOrigIdx]);
}

U32 xGetNComps(System *sP) {
  if (!sP || !sP->cF) {
    return E_BAD_ARGS;
  }
	return arrayGetNElems(sP->cF);
}

// Insert inner mutation map (maps trigger to mutation) into outer mutation map (maps Entity to Map*)
Error xAddMutationMap(System *sP, Entity entity, Map *mutationMP) {
  if (!entity || !sP) { // null mutation map is okay
    return E_BAD_ARGS;
  }
  // If the component is immutable, that's fine, don't worry about it. 
  // If the user intended to mutate the gene, ensure we have both pieces of data.
  if (mutationMP && sP->mutationMPMP) {
    assert(arrayGetElemSz(mutationMP->mapA) == sP->mutationSz);
    return mapSet(sP->mutationMPMP, entity, &mutationMP);
  }
  // Otherwise bomb out.
  return SUCCESS;
}


Error xAddComp(System *sP, Entity entity, void *compDataP) {
  if (!sP || !entity || !compDataP) {
    return E_BAD_ARGS;
  }
  // Skip entities who already have a component in this system.
  if (mapGet(sP->e2cIdxMP, entity)) {
    return SUCCESS;
  }
  // Put component in first empty slot. (Will be garbage if this is a map. That's okay.)
  U32 cIdx; 
  Error e = frayAdd(sP->cF, compDataP, &cIdx); 
  // Add lookups from C -> E and E -> C.
  if (!e) {
    e = mapSet(sP->e2cIdxMP, entity, &cIdx);
  }
  if (!e) {
    sP->cIdx2eA[cIdx] = entity;
  }
  return e;
}

inline static Error _iniSubcompOwner(System *sP, Entity entity, Key subcompType, void *dataP) {
  // If subcomponent type doesn't fall into subcomponent mask
  if (!((subcompType & MASK_COMPONENT_SUBTYPE) == subcompType)) {
    return E_BAD_ARGS;
  }
  SubcompOwner newSubcompOwner = {0};
  newSubcompOwner.owner = entity;
  newSubcompOwner.subcompA[getSubcompIdx_(subcompType)] = dataP;
  return mapSet(sP->subcompOwnerMP, entity, &newSubcompOwner);
}


Error xAddEntityData(System *sP, Entity entity, Key compType, void *entityDataP) {
  if (!sP || !entity || !compType | !entityDataP) {
    return E_BAD_ARGS;
  }
  // Make sure the component belongs to this system. This is only checked at load-time.
  if ((compType & MASK_COMPONENT_TYPE) != sP->id) {
    printf("system id %d doesnt' matchh with compType %d\n", sP->id, compType & MASK_COMPONENT_TYPE);
    return E_SYS_CMP_MISMATCH;
  }
  Error e = SUCCESS;
  // If upper two bits are nonzero, this is a subcomponent. 
  if (compType & MASK_COMPONENT_SUBTYPE) {
    // If entity doesn't own any subcomponents yet, make a slot for it in subcomp ownership map.
    SubcompOwner *subcompOwnerP = mapGet(sP->subcompOwnerMP, entity);
    Key subcompType = compType & MASK_COMPONENT_SUBTYPE;
    if (!subcompOwnerP) {
      e = _iniSubcompOwner(sP, entity, subcompType, entityDataP);
    }
    // Otherwise, update its existing onwership record.
    else {
      subcompOwnerP->subcompA[getSubcompIdx_(subcompType)] = entityDataP;
    }
    // Now you can operate on the subcomponent in the system carefree.
    if (!e) {
      e = sP->iniSubcomp(sP, entity, compType & MASK_COMPONENT_SUBTYPE, entityDataP);
    }
    return e;
  }
  // Else it's the main component; feed it straight in baby.
  else if (!(sP->flags & FLG_DONT_ADD_COMP)) {
    e = xAddComp(sP, entity, entityDataP);
  }
  return e;
}

Error xIniSys(System *sP, U32 nComps, void *miscP) {
  // Sytems with special parts need to initialize maps in sIniU().
  Error e = frayNew((void**) &sP->cF, sP->compSz, nComps);
  if (!e) {
    e = arrayNew((void**) &sP->cIdx2eA, sizeof(Entity), nComps);
  }
  if (!e) {
    e = frayNew((void**) &sP->deactivateQueueF, sizeof(Entity), nComps);
  }
  if (!e) {
    e = frayNew((void**) &sP->pauseQueueF, sizeof(Entity), nComps);
  }
  if (!e) {
    e = mapNew(&sP->e2cIdxMP, RAW_DATA, sizeof(Key), nComps);
  }
  if (!e && !(sP->flags & FLG_NO_MUTATIONS_) && sP->mutationSz) {
    e = mapNew(&sP->mutationMPMP, MAP_POINTER, sizeof(Map*), nComps);
  }
	// Only allocate one mailbox; it serves as input and output.
	if (!e) {
		e = frayNew((void**) &sP->mailboxF, sizeof(Message), nComps);
  }
  if (!e) {
    e = mapNew(&sP->subcompOwnerMP, RAW_DATA, sizeof(SubcompOwner), nComps);
  }
  // Finally, call the system's unique initializer.
  if (!e) {
    e = (*sP->iniSys)(sP, miscP);
  }
  // Clean up if there are any problems.
  if (e) {
    xClr(sP);
  }

  return e;
}

// Don't erase everything in a system. Some things should be permanent.
void xClr(System *sP) {
  if (!sP) {
    return;
  }
  sP->clr(sP);  // This MUST run first as it may rely on things we're about to erase
  frayDel((void**) &sP->cF);
  frayDel((void**) &sP->mailboxF);
  frayDel((void**) &sP->deactivateQueueF);
  frayDel((void**) &sP->pauseQueueF);
  mapDel(&sP->subcompOwnerMP);
  mapDel(&sP->e2cIdxMP);
  // DOn't delete inner maps as there may be multiple copies pointing at the same thing.
  //mapOfNestedMapsDel(&sP->mutationMPMP);
  mapDel(&sP->mutationMPMP);
  arrayDel((void**) &sP->cIdx2eA);
}

Error xMutateComponent(System *sP, Entity entity, Key newCompKey) {
  if (!sP || !entity || !newCompKey) {
    return E_BAD_ARGS;
  }
  if (sP->flags & FLG_NO_MUTATIONS_) {
    return SUCCESS;
  }
  Map *mutationMP = NULL;
  Error e = mapGetNestedMapP(sP->mutationMPMP, entity, &mutationMP);
  if (!e) {
    assert(mutationMP->elemType == RAW_DATA);
    void* cP = xGetCompPByEntity(sP, entity);
    if (cP) {
      void *tmpP = mapGet(mutationMP, newCompKey);
      if (tmpP) {
        memcpy((U8*) cP + sP->mutationOffset, tmpP, arrayGetElemSz(mutationMP->mapA));
        return sP->postMutate(sP, cP);
      }
    }
  }
  return e;
}

static Error _xReadInbox(System *sP) {
  if (!sP || !sP->mailboxF) {
    return E_BAD_ARGS;
  }
  Error e = SUCCESS;
  Message *msgP = sP->mailboxF;
  Message *msgEndP = msgP + *_frayGetFirstEmptyIdxP(sP->mailboxF);
  for (; !e && msgP < msgEndP; msgP++) {
    switch(msgP->cmd) {
      case MUTATE_AND_ACTIVATE: 
        e = xMutateComponent(sP, msgP->attn, msgP->arg);
      case ACTIVATE:
        if (!e) {
          xActivateComponentByEntity(sP, msgP->attn);
        }
        break;
      case MUTATE_AND_DEACTIVATE: 
        e = xMutateComponent(sP, msgP->attn, msgP->arg);
      case DEACTIVATE:
        if (!e) {
          e = xDeactivateComponentByEntity(sP, msgP->attn);
        }
        break;
      case PAUSE:
        e = xPauseComponentByEntity(sP, msgP->attn);
        break;
      case UNPAUSE:
        e = xUnpauseComponentByEntity(sP, msgP->attn);
        break;
      case MUTATE:
        e = xMutateComponent(sP, msgP->attn, msgP->arg);
        break;
      default:
        e = sP->processMessage(sP, msgP);  // some systems require more specific treatment
        break;
    }
  }
  _frayClr(sP->mailboxF);
  return e;
}

static Error _pauseQueue(System *sP) {
  if (!sP) {
    return E_BAD_ARGS;
  }
  Error e = SUCCESS;
  U32 nPausedEntities = *_frayGetFirstEmptyIdxP(sP->pauseQueueF);
  if (nPausedEntities) {
    Entity *entityP = sP->pauseQueueF;
    Entity *entityEndP = entityP + nPausedEntities;
    for (; !e && entityP < entityEndP; ++entityP) {
      e = xPauseComponentByEntity(sP, *entityP);
    }
    _frayClr(sP->pauseQueueF);
  }
  return e;
}

static Error _deactivateQueue(System *sP) {
  if (!sP) {
    return E_BAD_ARGS;
  }
  Error e = SUCCESS;
  U32 nDeactivatedEntities = *_frayGetFirstEmptyIdxP(sP->deactivateQueueF);
  if (nDeactivatedEntities) {
    Entity *entityP = sP->deactivateQueueF;
    Entity *entityEndP = entityP + nDeactivatedEntities;
    for (; !e && entityP < entityEndP; ++entityP) {
      e = xDeactivateComponentByEntity(sP, *entityP);
    }
    _frayClr(sP->deactivateQueueF);
  }
  return e;
}

Entity xGetEntityByVoidComponentPtr(System *sP, void *componentP) {
  if (!sP || !componentP) {
    return 0;  // invalid entity
  }
  Entity compIdx = ((U32) componentP - (U32) sP->cF) / sP->compSz;
  return xGetEntityByCompIdx(sP, compIdx);
}


Error xQueuePause(System *sP, void *componentP) {
  Entity entity = xGetEntityByVoidComponentPtr(sP, componentP);
  if (entity) {
    return frayAdd(sP->pauseQueueF, &entity, NULL);
  }
  return E_NULL_VAR;
}

Error xQueueDeactivate(System *sP, void *componentP) {
  Entity entity = xGetEntityByVoidComponentPtr(sP, componentP);
  if (entity) {
    return frayAdd(sP->deactivateQueueF, &entity, NULL);
  }
  return E_NULL_VAR;
}

/* This is how the entire ECS framework works. */
Error xRun(System *sP) {
  Error e =_xReadInbox(sP);
  if (!e) {
    e = sP->run(sP);
  }
  // Pause and deactivate all components that need to be.
  if (!e) {
    e = _deactivateQueue(sP);
  }
  if (!e) {
    e = _pauseQueue(sP);
  }
	return e;
}
