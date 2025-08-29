#include "x/x.h"
#include "data/share.h"

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

Key xGetCompIdxByEntity( System *sP, Entity entity ) {
  assert( sP && entity );
#ifndef NDEBUG
  return *( _getCompIdxPByEntity( sP, entity ) );
#else
  Key* idxP = _getCompIdxPByEntity( sP, entity );
  assert( idxP );
  return *idxP;
#endif
}


void __xSwap(System *sP, S32 origIdx, S32 newIdx) {
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
  sP->postActivate( sP, &changes );
}

void xDeactivateComponentByEntity(System *sP, Entity entity) {
  assert (sP && entity);
  FrayChanges changes;
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, entity);
  assert (compOrigIdxP);
  frayChangesIni_(changes, *compOrigIdxP);
  frayDeactivate(sP->cF, *compOrigIdxP, &changes);
  _xSwap(sP, &changes);
  sP->postDeactivate( sP, &changes );
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
  assert (mutationMP && sP->mutationMPMP);
  assert(arrayGetElemSz(mutationMP->mapA) == sP->mutationSz);
  // If the component is immutable, that's fine, don't worry about it. 
  // If the user intended to mutate the gene, ensure we have both pieces of data.
  mapSet(sP->mutationMPMP, entity, &mutationMP);
  // Otherwise just return successfully, assuming they never intended to mutate this entity's component in the first place.
}


void xIniSys(System *sP, U32 nComps, void *miscP) {
  // Sytems with special parts need to initialize maps in sIniU().
  sP->cF = frayNew(sP->compSz, nComps, GENERAL );
  sP->cIdx2eA = arrayNew(sizeof(Entity), nComps, GENERAL );
  sP->e2cIdxMP = mapNew( RAW_DATA, sizeof(Key), nComps, GENERAL );
  sP->deactivateQueueF = frayNew(sizeof(Entity), nComps, GENERAL );
  sP->pauseQueueF = frayNew( sizeof(Entity), nComps, GENERAL );
  if (!(sP->flags & FLG_NO_MUTATIONS_) && sP->mutationSz) {
    sP->mutationMPMP = mapNew( MAP_POINTER, sizeof(Map*), nComps, GENERAL );
  }
	// Only allocate one mailbox; it serves as input and output.
  // TODO make this smarter than a raw constant
  // Also, give it ample room to handle multiple messages per entity.
#define MAILBOX_MULTIPLY_NUM_SLOTS (3)
  sP->mailboxF = shareNewInbox( sP->id, nComps * MAILBOX_MULTIPLY_NUM_SLOTS );
  // Finally, call the system's unique initializer.
  (*sP->iniSys)(sP, miscP);  // fail-assert if this bombs
}

void xMutateComponent(System *sP, Entity entity, Key newCompKey) {
  // Make sure entity, system, and the key to the new component are all valid parameters.
  assert (sP &&  entity &&  newCompKey);
  // Make sure the system was set up for mutations in the first place.
  if (!(sP->flags & FLG_NO_MUTATIONS_)) {
    // Get the nested map of mutations for this particular entity.
    Map *mutationMP = mapGetNestedMapP(sP->mutationMPMP, entity);
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
  Entity compIdx = ((void*) componentP - (void*) sP->cF) / sP->compSz;
  return xGetEntityByCompIdx(sP, compIdx);
}

Bln xIsEntityActive( System  *sP, Entity entity ) {
  Key* idxP = _getCompIdxPByEntity(sP, entity);
  assert( idxP );
  return _frayElemIsActive( sP->cF, *idxP );
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
