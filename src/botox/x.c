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
	return (void*) ((U8*) sP->cF + (*_getCompIdxPByEntity(sP, entity) * _fast_arrayGetElemSz(sP->cF)));
}

inline static Entity _getEntityByCompIdx(System *sP, Key compIdx) {
  return sP->cIdx2eA[compIdx];
}

inline static HardCodedMap** _getMultiValMapPP(System *sP, Entity entity) {
  return (HardCodedMap**) mapGet(sP->multiValMP, entity);
}

inline static Map* _getMultiValMapP(System *sP, Entity entity) {
  HardCodedMap **hcmPP = _getMultiValMapPP(sP, entity);
  if (hcmPP) 
    return (*hcmPP)->mapP;
  return NULL;
}

void* xGetMultiValP(System *sP, Entity entity, Key key) {
  Map *mapP = _getMultiValMapP(sP, entity);
  if (mapP)
    return mapGet(mapP, key);
  return NULL;
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

inline static Error _iniMultiValMap(System *sP, Entity entity, HardCodedMap **srcHcmPP) {
  Error e = mapIni(*srcHcmPP);
  if (!e)
    e = mapSet(sP->multiValMP, entity, (const void*) srcHcmPP); 
  return e;
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
  // Put component in first empty slot. (Will be garbage if this is a map. That's okay.)
  U32 cIdx; 
  Error e = frayAdd(sP->cF, srcCompRawDataP, &cIdx); 
  // Assign this component to its entity and, if necessary, prepare it for the system.
  if (!e)
    e = sP->iniComp(sP, (void*) srcCompRawDataP);  
  if (!e && isMap)   
    e = _iniMultiValMap(sP, entity, (HardCodedMap**) srcCompRawDataP);
  // Add component's entity-lookup entry to the system's directory.
  if (!e)
    e = mapSet(sP->e2cIdxMP, entity, &cIdx);
  if (!e)
    sP->cIdx2eA[cIdx] = entity;

  return e;
}

Error xIniSys(System *sP, U32 nComps, void *miscP) {
  // Sytems with special parts need to initialize maps in sIniFP().
  Error e = frayNew((void**) &sP->cF, sP->compSz, nComps);
  if (!e)
    e = arrayNew((void**) &sP->cIdx2eA, sizeof(Entity), nComps);
  if (!e)
    e = mapNew(&sP->e2cIdxMP, sizeof(Key), nComps);
  if (!e)
    e = mapNew(&sP->multiValMP, sizeof(Map), nComps);
  if (!e)
    e = frayNew((void**) &sP->checkF, sizeof(Check), nComps);
	// Allocate inbox ONLY. Caller points sP->outboxF at another system's inboxF.
	if (!e)
		e = frayNew((void**) &sP->inboxF, sP->id, nComps);
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
  mailboxDel(&sP->inboxF);
  sP->outboxF = NULL;
  HardCodedMap **hcmPP = sP->multiValMP->mapA;
  HardCodedMap **hcmEndPP = hcmPP + arrayGetNElems(sP->multiValMP->mapA);
  for (; hcmPP <  hcmEndPP; hcmPP++)
    mapClr(*hcmPP);
  mapDel(&sP->multiValMP);
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
      e = mailboxWrite(sP->outboxF, entity, checkP->root, 0, 0);
      // Toggle to alternate check if necessary. Otherwise, deactivate this check.
      if (checkP->doesToggle) { 
        checkP->currCbIdx = !checkP->currCbIdx;
        checkP++;
      }
      else {
        frayDeactivate(sP->checkF, checkP - sP->checkF);
        --checkEndP;
      }
    }
  }
  return e;
}

void _xReadInbox(System *sP) {
  Message *msgP = sP->inboxF;
  Message *msgEndP = msgP + frayGetFirstInactiveIdx(sP->inboxF);
  while (msgP < msgEndP)
    sP->processMessage(sP, msgP++);
}

/* This is how the entire ECS framework works. */
Error xRun(System *sP) {
  _xReadInbox(sP);
  mailboxClr(sP->inboxF);
  Error e = sP->run(sP);
  if (!e)
    e = _xDoChecks(sP);
	return e;
}
