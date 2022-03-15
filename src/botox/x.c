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

Error xAddComp(System *sP, Entity entity, XHeader *xhP) {
  if (!sP || !xhP || !sP->compDirectoryP)
    return E_BAD_ARGS;

  // Skip entities who already have a component in this system.
  if (mapGet(sP->compDirectoryP, entity))
    return SUCCESS;

  // Make sure the component belongs to this system. This is only checked at load-time.
  if (xhP->type != sP->id)
    return E_SYS_CMP_MISMATCH;
  
  // Assign this component to its entity and, if necessary, prepare it for the system.
  xhP->owner = entity;
  Error e = (*sP->sIniCFP)(xhP);  // Inflatable components must be initialized before
                                    // being added to any focus.

  // If index falls within system's array of activities...
  if (_validateFocusIdx(sP, 0)) {
    Focus *fP = &sP->focusA[0];
    // Copy component to first emtpy slot in specified focus
    void *dstCP = _getCompPByIdx(fP, fP->firstEmptyIdx);
    memcpy(dstCP, xhP, sP->compSz);
    // Add component's entity-lookup entry to the system's directory.
    CompLocation compLocation = {
      .focusID  = 0,
      .compIdx        = fP->firstEmptyIdx++,  // this increments firstEmptyIdx 
      .compP          = dstCP
    };
    e = mapSet(sP->compDirectoryP, entity, &compLocation);
    fP->firstInactiveIdx++;  // TODO remove after developing the behavior tree
  }
  else 
    e = E_BAD_INDEX;

  return e;
}

void* xGetComp(System *sP, Entity entity) {
  CompLocation *compLocationP = (CompLocation*) _getCompLocation(sP, entity);
  if (compLocationP)
    return compLocationP->compP;
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

static void _xClrFocuses(System *sP) {
  if (sP->focusA != NULL)
    for (U8 i = 0, nFocuses = sP->nFocuses; i < nFocuses; i++) 
      arrayDel((void**) &sP->focusA[i].compA);
}

static Error xNewCompDirectory(System *sP, Key nElems) {
  return mapNew(&sP->compDirectoryP, sizeof(CompLocation), nElems);
}

static Error xNewFocusDirectory(System *sP) {
  return mapNew(&sP->focusDirectoryP, sizeof(FocusLocation), sP->nFocuses);
}

Error xIniSys(System *sP, U32 nComps, void *miscP) {
  // Sytems with system components need to initialize maps in sIniFP().
  Error e = xNewCompDirectory(sP, nComps);
  // Then allocate enough room for all components in every focus. 
  if (!e) 
    for (U8 i = 0; !e && i < sP->nFocuses; i++) 
      e = xIniFocus(sP, &sP->focusA[i], nComps);

	// Allocate maiboxes.
	if (!e)
		e = mailboxNew(&sP->inboxP, sP->id, nComps);
	if (!e)  //TODO: may need more than nComps outboxP->slots to accommodate check messages
		e = mailboxNew(&sP->outboxP, sP->id, nComps);  
  
  // Make focus directory.
  if (!e)
    e = xNewFocusDirectory(sP);
  for (Key i = 1; !e && i < sP->nFocuses; i++) {
    FocusLocation focusLocation = {i, &sP->focusA[i]};
    e = mapSet(sP->focusDirectoryP, sP->focusA[i].id, &focusLocation);
  }

  // Finally, call the system's unique initializer.
  if (!e)
    e = (*sP->sIniSFP)(sP, miscP);

  // Clean up if there are any problems
  if (e) {
    _xClrFocuses(sP);
    if (sP->compDirectoryP != NULL)
      mapDel(&sP->compDirectoryP);
    if (sP->focusDirectoryP != NULL)
      mapDel(&sP->focusDirectoryP);
  }
  
  return e;
}

static void _xReadMessage(System *sP, Message *msgP) {
	// If message tells you to change the component, do it.
	if (msgP->msg) {
    // Get entity's directory listing.
		CompLocation *compLocationP = (CompLocation*) mapGet(sP->compDirectoryP, msgP->to);
		assert(compLocationP && compLocationP->hcmP && compLocationP->hcmP->mapP);
		const void *newComponentValP = mapGet(compLocationP->hcmP->mapP, 
				                                  msgP->msg);
    if (newComponentValP)
      memcpy(compLocationP->compP, newComponentValP, sP->compSz);
	}
	xStartFocus(sP, msgP->to, msgP->attn);
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
    // TODO add checks array traversal 
    // Move dead activities out of the way.
    if (fP->firstInactiveIdx == 0)
      xDeactivateFocus(fP);  
  }

	return e;
}
