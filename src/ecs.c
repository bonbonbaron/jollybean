#include "ecs.h"

/* Go ahead and provide these functions here:
  Histogramming entitieSeeds' genes
*/

/**************/
/* Components */
/**************/
const static GetGeneFP getGeneFPA[] = {getUnaryGene, getBinaryGene, getNonbinaryGene};

/* Unary gene-selection */
void* getUnaryGene(U8 *geneHeaderP, Key *overrideKeyP) {
  UNUSED_(overrideKeyP);
  return ((UnaryGene*) geneHeaderP)->unaryP;
}

/* Binary gene-selection */
void* getBinaryGene(U8 *geneHeaderP, Key *overrideKeyP) {
  BinaryGene *bP = (BinaryGene*) geneHeaderP;
  if (overrideKeyP && *overrideKeyP)
    return arrayGetVoidElemPtr(bP->binaryA, ((*overrideKeyP & bP->mask) == bP->expVal));
  else
    return arrayGetVoidElemPtr(bP->binaryA, ((*bP->tgtP & bP->mask) == bP->expVal));
}

/* Nonbinary gene-selection */
void* getNonbinaryGene(U8 *geneHeaderP, Key *overrideKeyP) {
  NonbinaryGene *nP = (NonbinaryGene*) geneHeaderP;
  if (overrideKeyP)
    return mapGet(nP->mapP, *overrideKeyP);
  else
    return mapGet(nP->mapP, *nP->keyP);
}

void* getGene(U8 *geneHeaderP, Key *overrideKeyP) {
  /* No need for internal NULL-checks since wrapper does it here. */
  if (geneHeaderP != NULL)
    return (*getGeneFPA[*geneHeaderP])(geneHeaderP, overrideKeyP);
  return NULL;
}


/***********/
/* Systems */
/***********/
/* Only inspect every possible messaging error at load-time. Do not inspect every single message-reading. */
inline static U8 _validateActivityIdx(const System *sP, const U8 activityIdx) { 
  return (activityIdx < sP->nActivities);
}

/* Only inspect every possible messaging error at load-time. Do not inspect every single message-reading. */
inline static U8 _validateEcIdx(const System *sP, Activity *aP, const U8 cIdx) { 
  return (cIdx >= (U8) arrayGetNElems(aP->cA));
}

/* EC Getters and Setters */
/* When activating or deactivating, you have to update the system's directory to its new location. */
inline static Entity _getE(const void *cmpP) {
  return ((XHeader*) cmpP)->owner;
}


inline static void* _getCPByIndex(const System *sP, Activity *aP, U8 cIdx) {
  return (void*) (((U8*) aP->cA) + (sP->cSz * cIdx));
}

/* Returns a component's location in the jagged array of components.  */
inline static CDirEntry* _getCDirEntry(System *sP, Entity entity) {
  return (CDirEntry*) mapGet(sP->cDirectoryP, entity);
}

inline static ActDirEntry* _getActDirEntry(System *sP, Key actID) {
  return (ActDirEntry*) mapGet(sP->actDirectoryP, actID);
}

void* sGetC(System *sP, Entity entity) {
  CDirEntry *cdeP = (CDirEntry*) _getCDirEntry(sP, entity);
  if (cdeP)
    return cdeP->cP;
  return NULL;
}

Activity* sGetActivity(System *sP, Key actID) {
  ActDirEntry *adeP = _getActDirEntry(sP, actID);
  if (adeP)
    return adeP->activityP;
  return NULL;
}

Activity* sGetActivityFromE(System *sP, Entity entity) {
  CDirEntry *cdeP = (CDirEntry*) _getCDirEntry(sP, entity);
  if (cdeP) 
    return sGetActivity(sP, cdeP->activityID);
  return NULL;
}

/* Checks if the component, wherever it is in the jagged array, is before the function's stopping point in its array. */
U8 sComponentIsActive(System *sP, Entity entity) {
  CDirEntry* cdeP = _getCDirEntry(sP, entity);
  Activity *aP = sGetActivityFromE(sP, entity);
  return (cdeP->cIdx < aP->firstInactiveIdx);
}

static Error  _updateCDirEntry(System *sP, Entity entity, U8 dstActID, U8 dstCIdx, void *dstCP) {
  CDirEntry *cdeP = _getCDirEntry(sP, entity);
  // This component exists in this system's directory, which means it's moving around right now.
  if (cdeP) {
    cdeP->activityID = dstActID;
    cdeP->cIdx       = dstCIdx;
    cdeP->cP         = dstCP;
  }
  // This component hasn't been added to map yet, which implies we're in initialization phase.
  else
    return E_BAD_KEY;
  return SUCCESS;
}

static Error _mvC(System *sP, void *srcCP, void *dstCP, Key dstActID, U8 dstCIdx) {
  memcpy((void*) dstCP, (const void*) srcCP, sP->cSz);  
  return _updateCDirEntry(sP, _getE(srcCP), dstActID, dstCIdx, dstCP);
}

// Used for deactivating a C within the same activity
Error sDeactivateC(System *sP, Entity entity) {
  // Directory entry for source component
  CDirEntry *srcComponentDirectoryEntryP = _getCDirEntry(sP, entity);
  if (!srcComponentDirectoryEntryP)
    return E_BAD_KEY;
  // Directory entry for source component's activity
  ActDirEntry *activityDirectoryEntryP = _getActDirEntry(sP, srcComponentDirectoryEntryP->activityID);
  if (!activityDirectoryEntryP)
    return E_BAD_KEY;
  // Pointer to destination component
  void *dstComponentP = _getCPByIndex((const System*) sP, activityDirectoryEntryP->activityP, --activityDirectoryEntryP->activityP->firstInactiveIdx);
  // Move source to placeholder
  memcpy(sP->swapPlaceholderP, (const void*) srcComponentDirectoryEntryP->cP, sP->cSz);
  // Move destination to source's original place
  Error e = _mvC(sP, dstComponentP, srcComponentDirectoryEntryP->cP, srcComponentDirectoryEntryP->activityID, srcComponentDirectoryEntryP->cIdx);
  // Move placeholder to destination
  if (!e)
    e = _mvC(sP, sP->swapPlaceholderP, dstComponentP, srcComponentDirectoryEntryP->activityID, activityDirectoryEntryP->activityP->firstInactiveIdx);
  return e;
}

// Used for activating a C within the same activity
Error sActivateC(System *sP, Entity entity) {
	Error e = SUCCESS;
  // Directory entry for source component
  CDirEntry *srcComponentDirectoryEntryP = _getCDirEntry(sP, entity);
  if (!srcComponentDirectoryEntryP)
    return E_BAD_KEY;
  // Directory entry for source component's activity
  ActDirEntry *activityDirectoryEntryP = _getActDirEntry(sP, srcComponentDirectoryEntryP->activityID);
  if (!activityDirectoryEntryP)
    return E_BAD_KEY;
	// Wake activity up if it's dormant.
  if (activityDirectoryEntryP->activityP->id >= sP->firstInactiveActIdx)
    e = sActivateActivity(sP, activityDirectoryEntryP->activityP->id);
  // Pointer to destination component
	if (!e) {
		void *dstComponentP = _getCPByIndex((const System*) sP, activityDirectoryEntryP->activityP, activityDirectoryEntryP->activityP->firstInactiveIdx);
		// Move source to placeholder
		memcpy(sP->swapPlaceholderP, (const void*) srcComponentDirectoryEntryP->cP, sP->cSz);
		// Move destination to source's original place
		e = _mvC(sP, dstComponentP, srcComponentDirectoryEntryP->cP, srcComponentDirectoryEntryP->activityID, srcComponentDirectoryEntryP->cIdx);
		// Move placeholder to destination
		if (!e)
			e = _mvC(sP, sP->swapPlaceholderP, dstComponentP, srcComponentDirectoryEntryP->activityID, activityDirectoryEntryP->activityP->firstInactiveIdx++);
	}
  return e;
}

static Error _updateActDirEntry(System *sP, Key actID, U8 dstActIdx, Activity *dstActP) {
  ActDirEntry *adeP = (ActDirEntry*) mapGet(sP->actDirectoryP, actID);
  // This component exists in this system's directory, which means it's moving around right now.
  if (adeP) {
    adeP->activityIdx = dstActIdx;
    adeP->activityP   = dstActP;
  }
  // This component hasn't been added to map yet, which implies we're in initialization phase.
  else
    return E_BAD_KEY;
  return SUCCESS;
}

static Error _mvActivity(System *sP, Activity *srcActP, U8 dstActIdx) {
  Activity *dstActP = &sP->activityA[dstActIdx];
  memcpy((void*) dstActP, (const void*) srcActP, sizeof(Activity));
  return _updateActDirEntry(sP, srcActP->id, dstActIdx, dstActP);
}

static Error _swapActivities(System *sP, Key srcActID, U8 dstActIdx) {
  ActDirEntry *srcAdeP = (ActDirEntry*) mapGet(sP->actDirectoryP, srcActID);
  if (!srcAdeP)
    return E_BAD_KEY;
  Activity tmpActivity = *srcAdeP->activityP;
  Error e = _mvActivity(sP, &sP->activityA[dstActIdx], srcAdeP->activityIdx);
  if (!e)
    _mvActivity(sP, &tmpActivity, dstActIdx);
  return e;
}

Error sActivateActivity(System *sP, Key activityID) {
  return _swapActivities(sP, activityID, sP->firstInactiveActIdx++);
}

Error sDeactivateActivity(System *sP, Key activityID) {
  return _swapActivities(sP, activityID, --sP->firstInactiveActIdx);
}

/* Kind of a no-brainer that if you're transferring something, it's going to be active in its destination activity. */
Error sStartCActivity(System *sP, Entity entity, Key dstActivityID) {
  Error e = SUCCESS;
  CDirEntry *cdeP = _getCDirEntry(sP, entity);
  if (cdeP != NULL)
    return E_BAD_KEY;
  if (cdeP->activityID == dstActivityID && !(sComponentIsActive(sP, entity)))
			e = sActivateC(sP, entity);
  else {
    /* Move component in destination out of the way then fill vacated spot with source EC. */
    Activity *dstActivityP = sGetActivity(sP, dstActivityID);
    void *firstInactiveDstCP = _getCPByIndex(sP, dstActivityP, dstActivityP->firstInactiveIdx);
    void *firstEmptyDstCP    = _getCPByIndex(sP, dstActivityP, dstActivityP->firstEmptyIdx);
    e = _mvC(sP, firstInactiveDstCP, firstEmptyDstCP, dstActivityID, dstActivityP->firstEmptyIdx++);  /* You should check if destination is already emtpy first. */
    if (!e)
      e = _mvC(sP, cdeP->cP, firstInactiveDstCP, dstActivityID, dstActivityP->firstInactiveIdx++);
  }
  return e;
}

Error sIniActivity(System *sP, Activity *aP, U32 nComps) {
  Error e = arrayNew(&aP->cA, sP->cSz, nComps);
  if (!e) 
    aP->ownerP = sP;
  return e;
}

static void _sClrActivities(System *sP) {
  if (sP->activityA != NULL)
    for (U8 i = 0, nActivities = sP->nActivities; i < nActivities; i++) 
      arrayDel((void**) &sP->activityA[i].cA);
}


Error sNewCDirectory(System *sP, U8 nElems) {
  return mapNew(&sP->cDirectoryP, sizeof(CDirEntry), nElems);
}

Error sNewActDirectory(System *sP) {
  return mapNew(&sP->actDirectoryP, sizeof(ActDirEntry), sP->nActivities);
}


// Components are initially spread out across the entity seeds, 
// so we'll initialize those one at a time later into the first activity as deactivated.
// Then the Behavior System will take care of putting those in the proper activity.
Error sIni(System *sP, U32 nComps) {
  // Sytems with system components need to initialize maps in sIniFP().
  Error e =  sNewCDirectory(sP, nComps);
  // Then allocate enough room for all components in every activity. 
  if (!e) 
    for (U8 i = 0; !e && i < sP->nActivities; i++) 
      e = sIniActivity(sP, &sP->activityA[i], nComps);
  
  // Make activity directory.
  if (!e)
    e = sNewActDirectory(sP);
  for (Key i = 1; !e && i < sP->nActivities; i++) {
    ActDirEntry ade = {i, &sP->activityA[i]};
    e = mapSet(sP->actDirectoryP, sP->activityA[i].id, &ade);
  }

  // Finally, call the system's unique initializer.
  if (!e)
    e = (*sP->sIniSFP)();

  // Clean up if there are any problems
  if (e) {
    _sClrActivities(sP);
    if (sP->cDirectoryP != NULL)
      mapDel(&sP->cDirectoryP);
    if (sP->actDirectoryP != NULL)
      mapDel(&sP->actDirectoryP);
  }
  
  return e;
}

Error sAddC(System *sP, Entity entity, XHeader *xhP) {
  if (!sP || !xhP || !sP->cDirectoryP)
    return E_BAD_ARGS;

  // Skip entities who already have a component in this system.
  if (mapGet(sP->cDirectoryP, entity))
    return SUCCESS;

  // Make sure the component belongs to this system. This is only checked at load-time.
  if (xhP->type != sP->id)
    return E_SYS_CMP_MISMATCH;
  
  // Assign this component to its entity and, if necessary, prepare it for the system.
  xhP->owner = entity;
  Error e = (*sP->sIniCFP)(xhP);  // Inflatable components must be initialized before
                                    // being added to any activity.

  // If index falls within system's array of activities...
  if (_validateActivityIdx(sP, FIRST_ACTIVITY)) {
    Activity *aP = &sP->activityA[FIRST_ACTIVITY];
    // Copy component to first emtpy slot in specified activity
    void *dstCP = _getCPByIndex(sP, aP, aP->firstEmptyIdx);
    memcpy(dstCP, xhP, sP->cSz);
    // Add component's entity-lookup entry to the system's directory.
    CDirEntry cde = {
      .activityID  = FIRST_ACTIVITY,
      .cIdx        = aP->firstEmptyIdx++,  // this increments firstEmptyIdx 
      .cP          = dstCP
    };
    e = mapSet(sP->cDirectoryP, entity, &cde);
    aP->firstInactiveIdx++;  // TODO remove after developing the behavior tree
  }
  else 
    e = E_BAD_INDEX;

  return e;
}

Error sNewMailbox(Mailbox **mailboxPP, U32 nMailSlots) {
  Error e = jbAlloc((void**)mailboxPP, sizeof(Mailbox), 1);
  if (!e)
    e = arrayNew((void**) mailboxPP, sizeof(Message), nMailSlots);
  if (e) {
    arrayDel((void**) mailboxPP);
    jbFree((void**)mailboxPP);
  }
  return e;
}
    
static void _clrMailbox(Mailbox *mailboxP) {
  memset(mailboxP->msgA, 0, sizeof(Message) * arrayGetNElems(mailboxP));
}

static void _sClrMailboxes(System *sP) {
  arrayDel((void**)&sP->inbox.msgA);
  arrayDel((void**)&sP->outbox.msgA);
  sP->inbox.nMsgs = 0;
  sP->outbox.nMsgs = 0;
}

void sReset(System *sP) {
  /* TODO */
}

void sClr(System *sP) {
  _sClrMailboxes(sP);
  _sClrActivities(sP);
  mapDel(&sP->cDirectoryP);
}

// TODO:when you have some valid system-wide functions, add them here
#if 0
SysBasicFP sBasicFuncs[] = {

};
#endif

inline static void _sReadMessage(System *sP, Message *msgP) {
  switch(msgP->type) {
#if 0
    /* Message intends a system-wide response. All systems share the same pool of systemwide functions. */
    case SYSTEMWIDE_CMD:
      (sBasicFuncs[msgP->cmd])(sP);
      break;
#endif
    /* Message intends a repeating function call on a component. */
    case REPEATING_CMP_CMD:
      sStartCActivity(sP, msgP->toID, msgP->cmd);
      break;
    /* Message intends a one-time function call on a component. */
    case ONE_OFF_CMP_CMD:
      (*sP->oneOffFPA[msgP->cmd])(sP, msgP->toID, msgP->event);
      break;
    default:
      break;
  }
}

void _sReadInbox(System *sP) {
  if (sP != NULL && sP->inbox.msgA != NULL) {
    Message *msgP, *msgLastP;
    arrayIniPtrs(sP->inbox.msgA, (void**) &msgP, (void**) &msgLastP, sP->inbox.nMsgs);
    while (msgP < msgLastP)
      _sReadMessage(sP, msgP++);
  }
}

// Outbox messages... Child systems may not deliver messages.
void sSendMessage(System *sP, Message *msgP) {
  memcpy((void*) &sP->outbox.msgA[sP->outbox.nMsgs++], msgP, sizeof(Message));
}

void sAct(System *sP) {
  Activity *aP = &sP->activityA[0];
  Activity *aEndP = aP + sP->firstInactiveActIdx;

  // Run all live activities.
  for (; aP < aEndP; aP++) {
    (*aP->sFP)(aP);
    // Move dead activities out of the way.
    if (aP->firstInactiveIdx == 0)
      sDeactivateActivity(sP, aP->id);  
  }
}

/* This is how the entire ECS framework works. */
void sRun(System *sP) {
  _sReadInbox(sP);
  _clrMailbox(&sP->inbox);
	sAct(sP);
}

//***************************************************
//** Parent systems *********************************
//***************************************************

static Entity entityCounter;

NEW_SYS_(Parent, 0, 
	ACTIVITY_(REACT, xParentReact),
	ACTIVITY_(TICK, xParentTick),
);

static Map **_entityReactionMA;
static Map *_subscriberAMP;
static U32 _nEntities;
static U16 _nSystems;
static GenomeGrp *_currGenomeGrpP;
static System **_sPA;
//TODO: consider static variables for system array and seed array

// =====================================================================
// Loop through each genome's genes and histogram their component types.
// =====================================================================
Error _histoGeneTypes(U32 *histoA, GenomeGrp *ggP) {
	Genome **gPP = &ggP->genomePA[0];					// pointer to an array of genomes
	Genome **gEndPP = gPP + ggP->nEntities;   // pointer to the end of the above array
	XHeader **xhPP, **xhEndPP;   // pointers to an array of X-header pointers and its end
	// Loop through genomes
	for (; gPP < gEndPP; gPP++) {
		xhPP = &(*gPP)->genePA[0];
		xhEndPP = xhPP + (*gPP)->nGenes;
		// Loop through current genome's genes
		for (; xhPP < xhEndPP; xhPP++)
			histoA[(*xhPP)->type]++;
	}
	return SUCCESS;
}

// =====================================================================
// Distribute all genes to their appropriate subsystems.
// =====================================================================
static Error _distributeGenes(System *sParentP, GenomeGrp *ggP) {
	Genome **gPP = &ggP->genomePA[0];					// pointer to an array of genomes
	Genome **gEndPP = gPP + ggP->nEntities;   // pointer to the end of the above array
	XHeader **xhPP, **xhEndPP;   // pointers to an array of X-header pointers and its end
	Error e = SUCCESS;
	// Loop through genomes
	for (entityCounter = N_SYS_TYPES; !e && gPP < gEndPP; gPP++, entityCounter++) {
		xhPP = &(*gPP)->genePA[0];
		xhEndPP = xhPP + (*gPP)->nGenes;
		// Loop through current genome's genes
		for (System *sP = NULL; !e && xhPP < xhEndPP; xhPP++) {  // xhPP is a pointer to a pointer to a global singleton of a component
			sP = (System*) sGetC(sParentP, (*xhPP)->type);  // We don't set the owner of the gene pool.
			if (sP)
				e = sAddC(sP, entityCounter, *xhPP);
		}
	}
	return SUCCESS;
}

// =====================================================================
// Initialize the Parent system.
// =====================================================================
Error xParentIniS() {
	// Histogram gene types
	U32 *histoP = NULL;
	Error e = histoNew(&histoP, N_SYS_TYPES);
	if (!e)
		e = _histoGeneTypes(histoP, _currGenomeGrpP);
	
	// Add subsystems as components to Parent system.
	extern System sControl;
	sAddC(&sParent, sControl.id, &sControl.xHeader); // Add control sys no matter what
	for (U32 i = 0; !e && i < _nSystems; i++) 
		if (_sPA[i])
			e = sAddC(&sParent, _sPA[i]->id, &_sPA[i]->xHeader);

	// Initialize Parent's component subsystems before we throw genes in.
	if (!e) {
		U32 *hP, *hEndP;
		arrayIniPtrs(histoP, (void**) &hP, (void**) &hEndP, -1);
		for (Entity sID = 0; !e && hP < hEndP; hP++, sID++) {
			if (*hP)  {
				System *sP = (System*) sGetC(&sParent, sID);
				if (sP)
					e = sIni(sP, *hP);  // makes subsystem's EC map and activities
			}
		}
	}
	
	// Distribute the genes to the proper systems.
	if (!e)
		e = _distributeGenes(&sParent, _currGenomeGrpP);
	
	sParent.firstInactiveActIdx = 2;
	// Clean up.
	histoDel(&histoP);
	
	return e;
}

// =====================================================================
// Clear all subsystems and Parent system.
// =====================================================================
void sClrAll() {
	//TODO
}

// ====================================================================================
// Placeholder for component-initialization; this has to be handled in xParentIniS().
// ====================================================================================
Error xParentIniC(XHeader *xhP) {
	UNUSED_(xhP);
	return SUCCESS;
}

// Only parents are allowed to deliver messages to child systems.
// This is how callbacks will start ECS loops.
Error sDeliverMessage(Key sysID, Message *msgP) {
	// Get the child.
	System *sP = (System*) sGetC(&sParent, sysID);
	assert(sP);
	// Deliver the message.
  memcpy((void*) &sP->inbox.msgA[sP->inbox.nMsgs++], msgP, sizeof(Message));
	// If child system is asleep, wake it up!
	if (!(sComponentIsActive(&sParent, sysID)))
		return sActivateC(&sParent, sysID);
	return SUCCESS;
}
// msgP may contain information the callback finds pertinent; e.g. "Who collided w/ me?"
static Error _trigger(Message *msgP) {
	assert(msgP->toID >= _nEntities && msgP->toID >= N_SYS_TYPES);  // highest system will have entity ID <= N_SYS_TYPES
	Map *mapP = _entityReactionMA[msgP->toID];  // toID is an entity
	Reaction *requestedReactionP = (Reaction*) mapGet(mapP, msgP->event);
	assert(requestedReactionP);
	Reaction *ongoingReactionP = (Reaction*) sGetC(&sParent, msgP->toID);
	assert(ongoingReactionP);
	// If entity is busy, see if we're allowed to interrupt it.
	Activity *currActivityP = sGetActivityFromE(&sParent, msgP->toID);
	assert(currActivityP);
	if (currActivityP->id == TICK && sComponentIsActive(&sParent, msgP->toID)) {
		if (requestedReactionP->priority < ongoingReactionP->priority)  // request is lower priority than current activity
			return SUCCESS;
	}
	// Entity is either idle or doing something less important. Give it something better to do.
	*ongoingReactionP = *requestedReactionP;
	ongoingReactionP->msg = *msgP;
	return sStartCActivity(&sParent, msgP->toID, TICK);
}

static Error _triggerGroup(Message *msgP) {
	assert(msgP);
	Error e = SUCCESS;
	Entity *eA = (Entity*) mapGet(_subscriberAMP, msgP->event);
	assert(eA);
	Entity *eP, *eEndP;
	arrayIniPtrs((void*) eA, (void**) &eP, (void**) &eEndP, -1);
	for (; !e && eP < eEndP; eP++)
		e = _trigger(msgP);
	return e;
}

Error xParentReact(Activity *aP) {
	assert(aP);
	XParentC *cP, *cEndP;
	Error e = SUCCESS;
	// Check active subsystems' outboxes. Their callbacks populate systems and JB's reaction activity.
	Activity *rcaP = sGetActivity(&sParent, TICK);
	assert(rcaP);
	arrayIniPtrs(rcaP->cA, (void**) &cP, (void**) &cEndP, rcaP->firstInactiveIdx);
	for (; !e && cP < cEndP; cP++) {
		for (Message *msgP = cP->item.s.outbox.msgA, *msgEndP = cP->item.s.outbox.msgA + cP->item.s.outbox.nMsgs;
				 !e && msgP < msgEndP;
				 msgP++) {
			if (msgP->toID) 
				e = _trigger(msgP);
			else
				e = _triggerGroup(msgP);
		}
	}
	
	// Run active reactions.
	arrayIniPtrs(rcaP->cA, (void**) &cP, (void**) cEndP, rcaP->firstInactiveIdx);
	for (; !e && cP < cEndP; cP++)
		if (cP->item.r.cb(&cP->item.r.msg, cP->item.r.paramsP) == COMPLETE) 
			sDeactivateC(&sParent, cP->item.r.msg.toID);

	return SUCCESS;
}

// This runs Parent's subsystems.
Error xParentTick(Activity *aP) {
	assert(aP);
	XParentC *cP, *cEndP;
	Error e = SUCCESS;
	arrayIniPtrs(aP->cA, (void**) &cP, (void**) &cEndP, aP->firstInactiveIdx);
	for (cP = aP->cA; !e && cP < cEndP; cP++) {
		sRun(&cP->item.s);
		// Put idle systems to sleep.
		if (cP->item.s.firstInactiveActIdx == 0)
			e = sDeactivateC(&sParent, cP->item.s.id);
	}
	return e;
}

Error xIni(System **sPA, U16 nSystems, GenomeGrp *genomeGroupP) {
	// sIni() takes no arguments, yet the parent system needs to know some outside factors.
	// So we use static variables here. 
	_nSystems = nSystems;
	_currGenomeGrpP = genomeGroupP;
	_sPA = sPA;
	return sIni(&sParent, N_SYS_TYPES);
}

Error xRun() {
	sAct(&sParent);
	return SUCCESS;
}
