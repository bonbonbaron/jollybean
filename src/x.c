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

static void _xSwap(System *sP, Key *origIdxP, Key newIdx) {
  Key *newIdxP  = mapGet(sP->e2cIdxMP, _getEntityByCompIdx(sP, newIdx));
  swap_(sP->cIdx2eA[newIdx], sP->cIdx2eA[*origIdxP]);
  swap_(*origIdxP, *newIdxP);
}

void xActivateComponentByEntity(System *sP, Entity entity) {
  // We use a pointer instead of a new one to swap its actual location later.
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, entity);
  Key  compNewIdx   = frayActivate(sP->cF, *compOrigIdxP);
  if (compNewIdx != *compOrigIdxP) {
    // Existence of paused elements requires an additional, intermediate swap.
    if (!*_frayGetNPausedP(sP->cF)) {
      _xSwap(sP, compOrigIdxP, compNewIdx);
    }
    else {
      Key intermediateIdx = *_frayGetFirstInactiveIdxP(sP->cF) - 1;
      _xSwap(sP, compOrigIdxP, intermediateIdx);
      _xSwap(sP, &intermediateIdx, compNewIdx);
    }
  }
}

void xDeactivateComponentByEntity(System *sP, Entity entity) {
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, entity);
  if (!compOrigIdxP) {
    printf("got a null pointer\n");
    return;
  }
  Key  compNewIdx   = frayDeactivate(sP->cF, *compOrigIdxP);
  if (compNewIdx != *compOrigIdxP) {
    // Existence of paused elements requires an additional, intermediate swap.
    Key nPaused = *_frayGetNPausedP(sP->cF);
    if (nPaused) 
      _xSwap(sP, compOrigIdxP, compNewIdx);
    else {
      Key intermediateIdx = *_frayGetFirstInactiveIdxP(sP->cF) - nPaused;
      _xSwap(sP, compOrigIdxP, intermediateIdx);
      _xSwap(sP, &intermediateIdx, compNewIdx);
    }
  }
}

void xPauseComponentByEntity(System *sP, Entity entity) {
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, entity);
  _xSwap(sP, compOrigIdxP, frayPause(sP->cF, *compOrigIdxP));
}

void xUnpauseComponentByEntity(System *sP, Entity entity) {
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, entity);
  _xSwap(sP, compOrigIdxP, frayUnpause(sP->cF, *compOrigIdxP));
}

void xActivateComponentByIdx(System *sP, Key compOrigIdx) {
  xActivateComponentByEntity(sP, sP->cIdx2eA[compOrigIdx]);
}

void xDeactivateComponentByIdx(System *sP, Key compOrigIdx) {
  xDeactivateComponentByEntity(sP, sP->cIdx2eA[compOrigIdx]);
}

U32 xGetNComps(System *sP) {
	return arrayGetNElems(sP->cF);
}

Error xAddMutationMap(System *sP, Entity entity, Map *mutationMP) {
  if (!entity || !sP) { // null mutation map is okay
    return E_BAD_ARGS;
  }
  // If the component is immutable, that's fine, don't worry about it. 
  // If the user intended to mutate the gene, ensure we have both pieces of data.
  if (mutationMP && sP->mutationMPMP) {
    return mapSet(sP->mutationMPMP, entity, &mutationMP);
  }
  // Otherwise bomb out.
  return SUCCESS;
}


Error xAddComp(System *sP, Entity entity, void *compDataP) {
  // Skip entities who already have a component in this system.
  if (mapGet(sP->e2cIdxMP, entity)) {
    return SUCCESS;
  }
  // Put component in first empty slot. (Will be garbage if this is a map. That's okay.)
  U32 cIdx; 
  Error e = frayAdd(sP->cF, compDataP, &cIdx); 
  if (!e) {
    printf("[x] system %d added entity %d to fray idx %d!\n", sP->id, entity, cIdx);
  }
  // Add lookups from C -> E and E -> C.
  if (!e) {
    e = mapSet(sP->e2cIdxMP, entity, &cIdx);
  }
  if (!e) {
    sP->cIdx2eA[cIdx] = entity;
  }
  if (!e) {
    printf("[x] system %d cIdx2eA[%d] = %d! \n", sP->id, cIdx, sP->cIdx2eA[cIdx]);
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
  // If it's the main component, feed it straight in.
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
  if (!e && !(sP->flags & FLG_NO_SWITCHES_)) {
    e = mapNew(&sP->mutationMPMP, FUNCTION_POINTER, sizeof(XSwitchCompU), nComps);
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
  sP->clr(sP);  // This MUST run first as it may rely on things we're about to erase,
  frayDel((void**) &sP->cF);         // ... like maps of switches with cleanup cases.
  frayDel((void**) &sP->mailboxF);
  frayDel((void**) &sP->deactivateQueueF);
  frayDel((void**) &sP->pauseQueueF);
  mapDel(&sP->subcompOwnerMP);
  mapDel(&sP->e2cIdxMP);
  if (sP->mutationMPMP) {
    Map **mapPP = sP->mutationMPMP->mapA;
    Map **mapEndPP = sP->mutationMPMP->mapA + sP->mutationMPMP->population;
    for (; mapPP < mapEndPP; ++mapPP) {
      mapDel(mapPP);
    }
  }
  mapDel(&sP->mutationMPMP);
  arrayDel((void**) &sP->cIdx2eA);
}

void xSwitchComponent(System *sP, Entity entity, Key newCompKey) {
  Map **mutationMPP = (Map**) mapGet(sP->mutationMPMP, entity);
  if (mutationMPP) {
    Map *mutationMP = *mutationMPP;
    if (mutationMP) {
      void* activeCompP = xGetCompPByEntity(sP, entity);
      if (activeCompP) {
        void **tmpP = mapGet(mutationMP, newCompKey);
        if (tmpP)
          memcpy(activeCompP, *tmpP, sP->compSz);
      }
    }
  }
}

static Error _xReadInbox(System *sP) {
  Error e = SUCCESS;
  Message *msgP = sP->mailboxF;
  Message *msgEndP = msgP + *_frayGetFirstEmptyIdxP(sP->mailboxF);
  for (; !e && msgP < msgEndP; msgP++) {
    switch(msgP->cmd) {
      case SWITCH_AND_ACTIVATE: 
        xSwitchComponent(sP, msgP->attn, msgP->arg);
      case ACTIVATE:
        xActivateComponentByEntity(sP, msgP->attn);
        break;
      case SWITCH_AND_DEACTIVATE: 
        xSwitchComponent(sP, msgP->attn, msgP->arg);
      case DEACTIVATE:
        xDeactivateComponentByEntity(sP, msgP->attn);
        break;
      case PAUSE:
        xPauseComponentByEntity(sP, msgP->attn);
        break;
      case UNPAUSE:
        xUnpauseComponentByEntity(sP, msgP->attn);
        break;
      case SWITCH:
        xSwitchComponent(sP, msgP->attn, msgP->arg);
        break;
      default:
        e = sP->processMessage(sP, msgP);  // some systems require more specific treatment
        break;
    }
  }
  _frayClr(sP->mailboxF);
  return e;
}

static void _pauseQueue(System *sP) {
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
  U32 nDeactivatedEntities = *_frayGetFirstEmptyIdxP(sP->deactivateQueueF);
  if (nDeactivatedEntities) {
    Entity *entityP = sP->deactivateQueueF;
    Entity *entityEndP = entityP + nDeactivatedEntities;
    for (; entityP < entityEndP; ++entityP) {
      printf("trying to deactivate entity %d\n", *entityP);
      xDeactivateComponentByEntity(sP, *entityP);
    }
    _frayClr(sP->deactivateQueueF);
  }
}

Entity xGetEntityByVoidComponentPtr(System *sP, void *componentP) {
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
Error xRun(System *sP) {
  Error e =_xReadInbox(sP);
  if (!e) {
    e = sP->run(sP);
  }
  // Pause and deactivate all components that need to be.
  if (!e) {
    _deactivateQueue(sP);
    _pauseQueue(sP);
  }
	return e;
}
