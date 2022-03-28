#include "x.h"

/* Only inspect every possible messaging error at load-time. Do not inspect every single message-reading. */
inline static U8 _validateFocusIdx(const System *sP, const U8 focusIdx) { 
  return (focusIdx < sP->nFocuses);
}

inline static void* _getCompPByIdx(Focus *fP, U8 compIdx) {
  return (void*) (((U8*) fP->compA) + (fP->ownerP->compSz * compIdx));
}

/* Returns a component's location in the jagged array of components.  */
inline static CompLocation* _getCompLocation(System *sP, Entity entity) {
  return (CompLocation*) mapGet(sP->compDirectoryP, entity);
}

inline static FocusLocation* _getFocusLocation(System *sP, Key focusID) {
  return (FocusLocation*) mapGet(sP->focusDirectoryP, focusID);
}

U32 xGetNComponents(System *sP) {
	return arrayGetNElems(sP->compDirectoryP->mapA);
}

static Error _xIniCompMapP(CompLocation *compLocationP, HardCodedMap *srcHcmP) {
	if (compLocationP && srcHcmP) {
    compLocationP->hcmP = srcHcmP;
		return mapIni(compLocationP->hcmP);
  }
	return SUCCESS;
}

inline static void _compLocationIni(CompLocation *compLocationP, void *dstCP, Focus *fP) {
  compLocationP->checkIdx = 0;
  compLocationP->compIdx = fP->firstEmptyIdx++;
  compLocationP->compP = dstCP;
  compLocationP->focusID = fP->id;
  //compLocationP->hcmP;
}

Error xAddComp(System *sP, Entity entity, Key compType, Bln isMap, void *srcCompRawDataP) {
  if (!sP || !sP->compDirectoryP)
    return E_BAD_ARGS;

  // Skip entities who already have a component in this system.
  if (mapGet(sP->compDirectoryP, entity))
    return SUCCESS;

  // Make sure the component belongs to this system. This is only checked at load-time.
  if (compType != sP->id)
    return E_SYS_CMP_MISMATCH;
  
  XHeader *xhP = NULL;
  Focus *fP    = NULL;
  void *dstCP  = NULL;
  CompLocation compLocation;
  if (_validateFocusIdx(sP, 0)) {
    fP = &sP->focusA[0];
    dstCP = _getCompPByIdx(fP, fP->firstEmptyIdx);
    xhP = (XHeader*) dstCP;
  }
  // Initialize header of destination x-component.
  xhP->owner = entity;
  xhP->type = compType;
  // Copy component to first emtpy slot in first focus. (Moves to proper focus later.)
  // Assign this component to its entity and, if necessary, prepare it for the system.
  Error e = (*sP->sIniCompFP)(sP, xhP);  // Init comp before adding it to focus.
  if (!e && _validateFocusIdx(sP, 0)) 
    fP = &sP->focusA[0];
  // If index 0 falls within system's array of activities...
  if (!e && fP && dstCP) {
    // If there's no hard-coded map, copy a simple component into first available slot.
    if (isMap) {  // example of hard-coded map is animation with WALK_LEFT, RUN_RIGHT, etc. strips
      memset(dstCP, 0, sP->compSz);  // won't be set till later
      _compLocationIni(&compLocation, dstCP, fP);
      e = _xIniCompMapP(&compLocation, (HardCodedMap*) srcCompRawDataP);
    }
    else if (srcCompRawDataP != NULL) {  
      void *dstCompRawDataP = ((U8*) dstCP) + sizeof(XHeader);
      memcpy(dstCompRawDataP, srcCompRawDataP, sP->compSz - sizeof(XHeader));
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
  CompLocation *compLocationP = (CompLocation*) _getCompLocation(sP, entity);
  if (compLocationP)
    return compLocationP->compP;
  return NULL;
}

Map* xGetCompMapP(System *sP, Entity entity) {
  CompLocation *compLocationP = (CompLocation*) _getCompLocation(sP, entity);
  if (compLocationP)
    return compLocationP->hcmP->mapP;
  return NULL;
}

Focus* sGetFocus(System *sP, Key focusID) {
  FocusLocation *focusLocationP = _getFocusLocation(sP, focusID);
  if (focusLocationP)
    return focusLocationP->focusP;
  return NULL;
}

Focus* xGetFocusFromE(System *sP, Entity entity) {
  CompLocation *compLocationP = (CompLocation*) _getCompLocation(sP, entity);
  if (compLocationP) 
    return sGetFocus(sP, compLocationP->focusID);
  return NULL;
}

/* EC Getters and Setters */
inline static Entity _getE(const void *cmpP) {
  return ((XHeader*) cmpP)->owner;
}

/* Checks if the component, wherever it is in the jagged array, is before the function's stopping point in its array. */
U8 xCompIsActive(System *sP, Entity entity) {
  CompLocation* compLocationP = _getCompLocation(sP, entity);
  Focus *fP = xGetFocusFromE(sP, entity);
  return (compLocationP->compIdx < fP->firstInactiveIdx);
}

static Error  _updateCompLocation(System *sP, Entity entity, U8 dstFocusID, U8 dstCIdx, void *dstCP) {
  CompLocation *compLocationP = _getCompLocation(sP, entity);
  // This component exists in this system's directory, which means it's moving around right now.
  if (compLocationP) {
    compLocationP->focusID    = dstFocusID;
    compLocationP->compIdx       = dstCIdx;
    compLocationP->compP         = dstCP;
  }
  // This component hasn't been added to map yet, which implies we're in initialization phase.
  else
    return E_BAD_KEY;
  return SUCCESS;
}

static Error _mvC(System *sP, void *srcCP, void *dstCP, Key dstFocusID, U8 dstCIdx) {
  memcpy((void*) dstCP, (const void*) srcCP, sP->compSz);  
  return _updateCompLocation(sP, _getE(srcCP), dstFocusID, dstCIdx, dstCP);
}

// Used for deactivating a C within the same focus
Error xDeactivateComp(System *sP, Entity entity) {
  // Directory entry for source component
  CompLocation *srcComponentDirectoryEntryP = _getCompLocation(sP, entity);
  if (!srcComponentDirectoryEntryP)
    return E_BAD_KEY;
  // Directory entry for source component's focus
  FocusLocation *focusDirectoryEntryP = _getFocusLocation(sP, srcComponentDirectoryEntryP->focusID);
  if (!focusDirectoryEntryP)
    return E_BAD_KEY;
  // Pointer to destination component
  void *dstComponentP = _getCompPByIdx(focusDirectoryEntryP->focusP, --focusDirectoryEntryP->focusP->firstInactiveIdx);
  // Move source to placeholder
  memcpy(sP->swapPlaceholderP, (const void*) srcComponentDirectoryEntryP->compP, sP->compSz);
  // Move destination to source's original place
  Error e = _mvC(sP, dstComponentP, srcComponentDirectoryEntryP->compP, srcComponentDirectoryEntryP->focusID, srcComponentDirectoryEntryP->compIdx);
  // Move placeholder to destination
  if (!e)
    e = _mvC(sP, sP->swapPlaceholderP, dstComponentP, srcComponentDirectoryEntryP->focusID, focusDirectoryEntryP->focusP->firstInactiveIdx);
  return e;
}

// Used for activating a C within the same focus
Error xActivateComp(System *sP, Entity entity) {
	Error e = SUCCESS;
  // Directory entry for source component
  CompLocation *srcComponentDirectoryEntryP = _getCompLocation(sP, entity);
  if (!srcComponentDirectoryEntryP)
    return E_BAD_KEY;
  // Directory entry for source component's focus
  FocusLocation *focusDirectoryEntryP = _getFocusLocation(sP, srcComponentDirectoryEntryP->focusID);
  if (!focusDirectoryEntryP)
    return E_BAD_KEY;
	// Wake focus up if it's dormant.
  if (focusDirectoryEntryP->focusP->id >= sP->firstInactiveActIdx)
    e = xActivateFocus(focusDirectoryEntryP->focusP);
  // Pointer to destination component
	if (!e) {
		void *dstComponentP = _getCompPByIdx(focusDirectoryEntryP->focusP, focusDirectoryEntryP->focusP->firstInactiveIdx);
		// Move source to placeholder
		memcpy(sP->swapPlaceholderP, (const void*) srcComponentDirectoryEntryP->compP, sP->compSz);
		// Move destination to source's original place
		e = _mvC(sP, dstComponentP, srcComponentDirectoryEntryP->compP, srcComponentDirectoryEntryP->focusID, srcComponentDirectoryEntryP->compIdx);
		// Move placeholder to destination
		if (!e)
			e = _mvC(sP, sP->swapPlaceholderP, dstComponentP, srcComponentDirectoryEntryP->focusID, focusDirectoryEntryP->focusP->firstInactiveIdx++);
	}
  return e;
}

static Error _updateFocusLocation(System *sP, Key focusID, U8 dstFocusIdx, Focus *dstFocusP) {
  FocusLocation *focusLocationP = (FocusLocation*) mapGet(sP->focusDirectoryP, focusID);
  // This component exists in this system's directory, which means it's moving around right now.
  if (focusLocationP) {
    focusLocationP->focusIdx = dstFocusIdx;
    focusLocationP->focusP   = dstFocusP;
  }
  // This component hasn't been added to map yet, which implies we're in initialization phase.
  else
    return E_BAD_KEY;
  return SUCCESS;
}

static Error _mvFocus(Focus *srcFocusP, U8 dstFocusIdx) {
  Focus *dstFocusP = &srcFocusP->ownerP->focusA[dstFocusIdx];
  memcpy((void*) dstFocusP, (const void*) srcFocusP, sizeof(Focus));
  return _updateFocusLocation(srcFocusP->ownerP, srcFocusP->id, dstFocusIdx, dstFocusP);
}

static Error _swapFocuses(Focus *fP, U8 dstFocusIdx) {
	System *sP = fP->ownerP;
  FocusLocation *focusLocationP = (FocusLocation*) mapGet(sP->focusDirectoryP, fP->id);
  if (!focusLocationP)
    return E_BAD_KEY;
  Focus tmpFocus = *focusLocationP->focusP;
  Error e = _mvFocus(&sP->focusA[dstFocusIdx], focusLocationP->focusIdx);
  if (!e)
    _mvFocus(&tmpFocus, dstFocusIdx);
  return e;
}

Error xActivateFocus(Focus *fP) {
  return _swapFocuses(fP, fP->ownerP->firstInactiveActIdx++);
}

Error xDeactivateFocus(Focus *fP) {
  return _swapFocuses(fP, --fP->ownerP->firstInactiveActIdx);
}

/* Kind of a no-brainer that if you're transferring something, it's going to be active in its destination focus. */
Error xStartFocus(System *sP, Entity entity, Key dstFocusID) {
  Error e = SUCCESS;
  CompLocation *compLocationP = _getCompLocation(sP, entity);
  if (compLocationP != NULL)
    return E_BAD_KEY;
  if (compLocationP->focusID == dstFocusID && !(xCompIsActive(sP, entity)))
			e = xActivateComp(sP, entity);
  else {
    /* Move component in destination out of the way then fill vacated spot with source EC. */
    Focus *dstFocusP = sGetFocus(sP, dstFocusID);
    void *firstInactiveDstCP = _getCompPByIdx(dstFocusP, dstFocusP->firstInactiveIdx);
    void *firstEmptyDstCP    = _getCompPByIdx(dstFocusP, dstFocusP->firstEmptyIdx);
    /* You should check if destination is already emtpy first. */
    e = _mvC(sP, firstInactiveDstCP, firstEmptyDstCP, dstFocusID, dstFocusP->firstEmptyIdx++);  
    if (!e)
      e = _mvC(sP, compLocationP->compP, firstInactiveDstCP, dstFocusID, dstFocusP->firstInactiveIdx++);
  }
  return e;
}

Error xIniFocus(System *sP, Focus *fP, U32 nComps) {
  Error e = arrayNew(&fP->compA, sP->compSz, nComps);
  if (!e) 
    fP->ownerP = sP;
  return e;
}

static void _focusAClr(System *sP) {
  if (sP->focusA != NULL)
    for (U8 i = 0, nFocuses = sP->nFocuses; i < nFocuses; i++) 
      arrayDel((void**) &sP->focusA[i].compA);
}

static Error _compDirectoryNew(System *sP, Key nElems) {
  return mapNew(&sP->compDirectoryP, sizeof(CompLocation), nElems);
}

static void _compDirectoryDel(Map **compDirectoryPP) {
  if (compDirectoryPP) {
    Map *compDirectoryP = *compDirectoryPP;
    CompLocation *compLocationP = compDirectoryP->mapA;
    CompLocation *compLocationEndP = compLocationP + arrayGetNElems(compDirectoryP->mapA);
    for (; compLocationP < compLocationEndP; compLocationP++) 
      if (compLocationP->hcmP->mapP)
        mapDel(&compLocationP->hcmP->mapP);
    mapDel(compDirectoryPP);
  }
}

static Error _focusDirectoryNew(System *sP) {
  return mapNew(&sP->focusDirectoryP, sizeof(FocusLocation), sP->nFocuses);
}

static void _focusDirectoryDel(Map **focusDirectoryPP) {
  mapDel(focusDirectoryPP);
}

static Error _focusDirectoryIni(System *sP) {
	Error e = SUCCESS;
  for (Key i = 1; !e && i < sP->nFocuses; i++) {
    FocusLocation focusLocation = {i, &sP->focusA[i]};
    e = mapSet(sP->focusDirectoryP, sP->focusA[i].id, &focusLocation);
  }
	return e;
}

Error xIniSys(System *sP, U32 nComps, void *miscP) {
  // Sytems with system components need to initialize maps in sIniFP().
  Error e = _compDirectoryNew(sP, nComps);
  // Then allocate enough room for all components in every focus. 
	for (U8 i = 0; !e && i < sP->nFocuses; i++) 
		e = xIniFocus(sP, &sP->focusA[i], nComps);

	// Allocate maiboxes.
	if (!e)
		e = mailboxNew(&sP->inboxP, sP->id, nComps);
#if 0  // For efficient message-sending, point outboxes at others' inboxes.
	if (!e)  //TODO: may need more than nComps outboxP->slots to accommodate check messages
		e = mailboxNew(&sP->outboxP, sP->id, nComps);  
#endif
  
  // Make focus directory.
  if (!e)
    e = _focusDirectoryNew(sP);
  if (!e)
		e = _focusDirectoryIni(sP);

  // Finally, call the system's unique initializer.
  if (!e)
    e = (*sP->sIniSysFP)(sP, miscP);

  // Clean up if there are any problems.
  if (e) {
    _focusAClr(sP);
    if (sP->compDirectoryP != NULL)
      mapDel(&sP->compDirectoryP);
    if (sP->focusDirectoryP != NULL)
      mapDel(&sP->focusDirectoryP);
  }
  
  return e;
}

// Don't erase everything in a system. Some things should be permanent.
void xClr(System *sP) {
  _focusAClr(sP);
  _compDirectoryDel(&sP->compDirectoryP);
  arrayDel((void**) &sP->checkers.checkA);
  memset(&sP->checkers, 0, sizeof(Checkers));
  mailboxDel(&sP->inboxP);
  sP->outboxP = NULL;
  _focusDirectoryDel(&sP->focusDirectoryP);
}

static Error _xDoChecks(System *sP) {
  Error e = SUCCESS;
  Check *checkP = sP->checkers.checkA;
  Check *checkEndP = checkP + sP->checkers.firstInactiveIdx;
  // For each active check...
  while (checkP < checkEndP) {
    XHeader *xHeaderP = (XHeader*) checkP->compLocationP->compP;  // XHeader is first member of every component.
    // Run check. If it returns true, update target flag. 
    if (checkP->cbA[checkP->currCbIdx](xHeaderP, checkP->operandP)) {
      *checkP->resultFlagsP |= checkP->outputIfTrueA[checkP->currCbIdx];
      // Tell potentially sleeping tree to wake up.
      e = mailboxWrite(sP->outboxP, checkP->entity, checkP->root, 0, 0);
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
        --sP->checkers.firstInactiveIdx;
      }
    }
  }
  return e;
}

static void _xReadMessage(System *sP, Message *msgP) {
	// If message tells you to change the component, do it.
	if (msgP->msg) {
    // Get entity's directory listing.
		CompLocation *compLocationP = (CompLocation*) mapGet(sP->compDirectoryP, msgP->to);
		assert(compLocationP && compLocationP->hcmP && compLocationP->hcmP->mapP);
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
  Focus *fP = &sP->focusA[0];
  Focus *fEndP = fP + sP->firstInactiveActIdx;
	Error e = SUCCESS;
  // Run all live activities.
  for (; !e && fP < fEndP; fP++) {
    e = (*fP->focusFP)(fP);
    // Perform active checks, if any.
    if (!e && sP->checkers.firstInactiveIdx)
      e = _xDoChecks(sP);
    // Move dead activities out of the way.
    if (fP->firstInactiveIdx == 0)
      xDeactivateFocus(fP);  
  }
	return e;
}
