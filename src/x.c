#include "x.h"

// Copied these from data.c since I don't know how to inline across files.
inline static U32 _fast_arrayGetElemSz(const void *arryP) {
	return *((U32*) arryP - 2);
}

inline static Key* _getCompIdxPByEntity(System *sP, Entity entity) {
  return (Key*) mapGet(sP->e2cIdxMP, entity);
}

inline static void* _getCompPByEntity(System *sP, Entity entity) {
	return (void*) ((U8*) sP->cF + (*_getCompIdxPByEntity(sP, entity) * _fast_arrayGetElemSz(sP->cF)));
}

inline static Entity _getEntityByCompIdx(System *sP, Key compIdx) {
  return sP->cIdx2eA[compIdx];
}

void* xGetCompPByEntity(System *sP, Entity entity) {
  return _getCompPByEntity(sP, entity);
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
    if (!frayGetNPaused(sP->cF)) 
      _xSwap(sP, compOrigIdxP, compNewIdx);
    else {
      Key intermediateIdx = frayGetFirstInactiveIdx(sP->cF) - 1;
      _xSwap(sP, compOrigIdxP, intermediateIdx);
      _xSwap(sP, &intermediateIdx, compNewIdx);
    }
    // Example: If somebody starts moving, coll sys and trackers will want to know!
    mailboxWrite(sP->outboxF, 0, entity, sP->id, ACTIVATED);  
  }
}

void xDeactivateComponentByEntity(System *sP, Entity entity) {
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, entity);
  Key  compNewIdx   = frayDeactivate(sP->cF, *compOrigIdxP);
  if (compNewIdx != *compOrigIdxP) {
    // Existence of paused elements requires an additional, intermediate swap.
    Key nPaused = frayGetNPaused(sP->cF);
    if (nPaused) 
      _xSwap(sP, compOrigIdxP, compNewIdx);
    else {
      Key intermediateIdx = frayGetFirstInactiveIdx(sP->cF) - nPaused;
      _xSwap(sP, compOrigIdxP, intermediateIdx);
      _xSwap(sP, &intermediateIdx, compNewIdx);
    }
    // Example: If somebody starts moving, coll sys and trackers will want to know!
    mailboxWrite(sP->outboxF, 0, entity, sP->id, DEACTIVATED);
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

U32 xGetNComponents(System *sP) {
	return arrayGetNElems(sP->cF);
}

Error xAddComp(System *sP, Entity entity, Key compType, void *compDataP, Map *switchMP) {
  if (!sP || !compDataP)
    return E_BAD_ARGS;
  // Skip entities who already have a component in this system.
  if (mapGet(sP->e2cIdxMP, entity))
    return SUCCESS;
  // Make sure the component belongs to this system. This is only checked at load-time.
  if (compType != sP->id)
    return E_SYS_CMP_MISMATCH;
  
  // Put component in first empty slot. (Will be garbage if this is a map. That's okay.)
  U32 cIdx; 
  Error e = frayAdd(sP->cF, compDataP, &cIdx); 
  // Assign this component to its entity and, if necessary, prepare it for the system.
  if (!e && switchMP)   
    e = mapSet(sP->switchMPMP, entity, &switchMP);
  // Add lookups from C -> E and E -> C.
  if (!e)
    e = mapSet(sP->e2cIdxMP, entity, &cIdx);
  if (!e)
    sP->cIdx2eA[cIdx] = entity;

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
    e = mapNew(&sP->e2cIdxMP, sizeof(Key), nComps);
  }
  if (!e && !(sP->flags & FLG_NO_SWITCHES_)) {
    e = mapNew(&sP->switchMPMP, sizeof(XSwitchCompU), nComps);
  }
	// Allocate inbox ONLY. Caller points sP->outboxF at another system's inboxF.
	if (!e) {
		e = frayNew((void**) &sP->inboxF, sP->id, nComps);
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
  frayDel((void**) &sP->inboxF);
  frayDel((void**) &sP->deactivateQueueF);
  frayDel((void**) &sP->pauseQueueF);
  mapDel(&sP->e2cIdxMP);
  if (sP->switchMPMP) {
    Map **mapPP = sP->switchMPMP->mapA;
    Map **mapEndPP = sP->switchMPMP->mapA + arrayGetNElems(sP->switchMPMP->mapA);
    for (; mapPP < mapEndPP; ++mapPP) 
      mapDel(mapPP);
  }
  mapDel(&sP->switchMPMP);
  arrayDel((void**) &sP->cIdx2eA);
  sP->outboxF = NULL;
}

void xSwitchComponent(System *sP, Entity entity, Key newCompKey) {
  Map **switchMPP = (Map**) mapGet(sP->switchMPMP, entity);
  if (switchMPP) {
    Map *switchMP = *switchMPP;
    if (switchMP) {
      void* activeCompP = _getCompPByEntity(sP, entity);
      if (activeCompP) {
        void **tmpP = mapGet(switchMP, newCompKey);
        if (tmpP)
          memcpy(activeCompP, *tmpP, sP->compSz);
      }
    }
  }
}

void _xReadInbox(System *sP) {
  Error e = SUCCESS;
  Message *msgP = sP->inboxF;
  Message *msgEndP = msgP + frayGetFirstInactiveIdx(sP->inboxF);
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
  frayClr(sP->inboxF);
}

static void _pauseQueue(System *sP) {
  U32 nPausedEntities = *frayGetFirstEmptyIdxP(sP->pauseQueueF);
  if (nPausedEntities) {
    Entity *entityP = sP->pauseQueueF;
    Entity *entityEndP = entityP + nPausedEntities;
    for (; entityP < entityEndP; ++entityP) {
      xPauseComponentByEntity(sP, *entityP);
    }
    frayClr(sP->pauseQueueF);
  }
}

static void _deactivateQueue(System *sP) {
  U32 nDeactivatedEntities = *frayGetFirstEmptyIdxP(sP->deactivateQueueF);
  if (nDeactivatedEntities) {
    Entity *entityP = sP->deactivateQueueF;
    Entity *entityEndP = entityP + nDeactivatedEntities;
    for (; entityP < entityEndP; ++entityP) {
      xPauseComponentByEntity(sP, *entityP);
    }
    frayClr(sP->deactivateQueueF);
  }
}

inline static Entity _getEntityByVoidComponentPtr(System *sP, void *componentP) {
  Entity compIdx = ((U32) componentP - (U32) sP->cF) / sP->compSz;
  return xGetEntityByCompIdx(sP, compIdx);
}


void xQueuePause(System *sP, void *componentP) {
  Entity entity = _getEntityByVoidComponentPtr(sP, componentP);
  frayAdd(sP->pauseQueueF, &entity, NULL);
}

void xQueueDeactivate(System *sP, void *componentP) {
  Entity entity = _getEntityByVoidComponentPtr(sP, componentP);
  frayAdd(sP->deactivateQueueF, &entity, NULL);
}


/* This is how the entire ECS framework works. */
Error xRun(System *sP) {
  _xReadInbox(sP);
  Error e = sP->run(sP);
  // Pause and deactivate all components that need to be.
  if (!e) {
    _pauseQueue(sP);
    _deactivateQueue(sP);
  }
	return e;
}
