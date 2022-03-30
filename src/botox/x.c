#include "x.h"
#include "data.h"

// Copied these from data.c since I don't know how to inline across files.
inline static U32 _fast_arrayGetElemSz(const void *arryP) {
	return *(((U32*)arryP) - 2);
}

inline static void* _getCompPByIdx(const void *arryP, Key idx) {
	return (void*) ((U8*) arryP + (idx * _fast_arrayGetElemSz(arryP)));
}

inline static Key* _getCompIdxPByEntity(System *sP, Entity entity) {
  return (Key*) mapGet(sP->e2cIdxMP, entity);
}

inline static void* _getCompPByEntity(System *sP, Entity entity) {
  register Key *cIdxP = _getCompIdxPByEntity(sP, entity);
	return (void*) ((U8*) sP->cF + (*cIdxP * _fast_arrayGetElemSz(sP->cF)));
}

inline static Entity _getEntityByCompIdx(System *sP, Key compIdx) {
  return sP->cIdx2eA[compIdx];
}

#define swap_(a, b) {a^=b; b^=a; a^=b;}
void xActivateComponentByEntity(System *sP, Entity e1) {
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, e1);
  Key  compNewIdx   = frayActivate(sP->cF, *compOrigIdxP);
  Key *compNewIdxP  = mapGet(sP->e2cIdxMP, _getEntityByCompIdx(sP, compNewIdx));
  swap_(sP->cIdx2eA[compNewIdx], sP->cIdx2eA[*compOrigIdxP]);
  swap_(*compOrigIdxP, *compNewIdxP);
}

void xDeactivateComponentByEntity(System *sP, Entity e1) {
  Key *compOrigIdxP = _getCompIdxPByEntity(sP, e1);
  Key  compNewIdx   = frayDeactivate(sP->cF, *compOrigIdxP);
  Key *compNewIdxP  = mapGet(sP->e2cIdxMP, _getEntityByCompIdx(sP, compNewIdx));
  swap_(sP->cIdx2eA[compNewIdx], sP->cIdx2eA[*compOrigIdxP]);
  swap_(*compOrigIdxP, *compNewIdxP);
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

static Error _xIniCompMapP(CompLocation *compLocationP, HardCodedMap *srcHcmP) {
	if (compLocationP && srcHcmP) {
    compLocationP->hcmP = srcHcmP;
		return mapIni(compLocationP->hcmP);
  }
	return SUCCESS;
}

Error xAddComp(System *sP, Entity entity, Key compType, Bln isMap, void *srcCompRawDataP) {
  if (!sP || !srcCompRawDataP)
    return E_BAD_ARGS;

  // Skip entities who already have a component in this system.
  if (mapGet(sP->e2cIdxMP, entity))
    return SUCCESS;

  // Make sure the component belongs to this system. This is only checked at load-time.
  if (compType != sP->id)
    return E_SYS_CMP_MISMATCH;
  
  void *dstCP  = NULL;
  dstCP = _getCompPByIdx(sP->cF, sP->firstEmptyIdx++);
  // Copy component to system's first emtpy slot.
  // Assign this component to its entity and, if necessary, prepare it for the system.
  Error e = sP->iniComp(sP, (void*) srcCompRawDataP);  
  // If index 0 falls within system's array of activities...
  if (!e && dstCP) {
    // If there's no hard-coded map, copy a simple component into first available slot.
    if (isMap) {  // example of hard-coded map is animation with WALK_LEFT, RUN_RIGHT, etc. strips
      memset(dstCP, 0, sP->compSz);  // won't be set till later
      e = _xIniCompMapP(&compLocation, (HardCodedMap*) srcCompRawDataP);
    }
    else if (srcCompRawDataP != NULL) {  
      memcpy(dstCompRawDataP, srcCompRawDataP, sP->compSz);
      _compLocationIni(&compLocation, dstCP, fP);
    }
    // Add component's entity-lookup entry to the system's directory.
    if (!e)
      e = mapSet(sP->compDirectoryP, entity, &compLocation);
  }
  else 
    e = E_BAD_INDEX;  // applies to either focus or component index

  return e;
}

void* xGetComp(System *sP, Entity entity) {
  Key *cIdxP = (Key*) mapGet(sP->e2cIdxMP, entity);
  return _getCompPByIdx(sP->cF, *cIdxP);
}

Map* xGetCompMapP(System *sP, Entity entity) {
  CompLocation *compLocationP = (CompLocation*) _getCompLocation(sP, entity);
  if (compLocationP)
    return compLocationP->hcmP->mapP;
  return NULL;
}

Error xIniSys(System *sP, U32 nComps, void *miscP) {
  // Sytems with special parts need to initialize maps in sIniFP().
  Error e = frayNew((void**) &sP->cF, sP->compSz, nComps);
  if (!e)
    e = arrayNew((void**) &sP->cIdx2eA, sizeof(Entity), nComps);
  if (!e)
    e = mapNew(&sP->e2cIdxMP, sizeof(Key), nComps);
  if (!e)
    e = arrayNew((void**) &sP->hcmPA, sizeof(HardCodedMap), nComps);
  if (!e)
    e = frayNew((void**) &sP->checkF, sizeof(Check), nComps);
	// Allocate inbox ONLY. Caller points sP->outboxP at another system's inboxP.
	if (!e)
		e = mailboxNew(&sP->inboxP, sP->id, nComps);
  // Finally, call the system's unique initializer.
  if (!e)
    e = (*sP->iniSys)(sP, miscP);
  // Clean up if there are any problems.
  if (e) 
    xClr(sP);
  
  return e;
}

// Don't erase everything in a system. Some things should be permanent.
void xClr(System *sP) {
  frayDel((void**) &sP->cF);
  frayDel((void**) &sP->checkF);
  mapDel(&sP->e2cIdxMP);
  arrayDel((void**) &sP->cIdx2eA);
  mailboxDel(&sP->inboxP);
  sP->outboxP = NULL;
}

static Error _xDoChecks(System *sP) {
  Error e = SUCCESS;
  Check *checkP = sP->checkF;
  Check *checkEndP = checkP + frayGetFirstInactiveIdx((void*) checkP);
  // For each active check...
  while (checkP < checkEndP) {
    Entity entity = _getEntityByCompIdx(sP, *checkP->cIdxP);
    // Run check. If it returns true, update target flag. 
    if (checkP->cbA[checkP->currCbIdx](entity, checkP->operandP)) {
      *checkP->resultFlagsP |= checkP->outputIfTrueA[checkP->currCbIdx];
      // Tell potentially sleeping tree to wake up.
      e = mailboxWrite(sP->outboxP, entity, checkP->root, 0, 0);
      // Toggle to alternate check if necessary. Otherwise, deactivate this check.
      if (checkP->doesToggle) { 
        checkP->currCbIdx = !checkP->currCbIdx;
        checkP++;
      }
      else {
        Check tmpCheck = *checkP;  // Put check we're deactivating in temp spot
        *checkP = *checkEndP;      // Move last ACTIVE check out of the way.
        *checkEndP = tmpCheck;
        --checkEndP;
        frayDecrFirstInactiveIdx((void*) sP->checkF);
      }
    }
  }
  return e;
}

static void _xReadMessage(System *sP, Message *msgP) {
	// If message tells you to change the component, do it.
	if (msgP->msg) {
		void *cP = _getCompPByEntity(sP, msgP->topic);
		assert(cP && compLocationP->hcmP && compLocationP->hcmP->mapP);
		const void *newComponentValP = mapGet(compLocationP->hcmP->mapP, msgP->msg);
    if (newComponentValP)
      memcpy(compLocationP->compP, newComponentValP, sP->compSz);
	}
	xStartFocus(sP, msgP->topic, msgP->attn);  // topic = Entity, attn = focus
}

void _xReadInbox(System *sP) {
  if (sP != NULL && sP->inboxP->msgA != NULL) {
    Message *msgP, *msgLastP;
    arrayIniPtrs(sP->inboxP->msgA, (void**) &msgP, (void**) &msgLastP, sP->inboxP->nMsgs);
    while (msgP < msgLastP)
      _xReadMessage(sP, msgP++);
  }
}

/* This is how the entire ECS framework works. */
Error xRun(System *sP) {
  _xReadInbox(sP);
  mailboxClr(sP->inboxP);
  Error e = sP->xRun(sP);
  // Perform active checks, if any.
  if (!e && sP->checkers.firstInactiveIdx)
    e = _xDoChecks(sP);
  // Move dead activities out of the way.
  if (fP->firstInactiveIdx == 0)
    xDeactivateFocus(fP);  
	return e;
}
