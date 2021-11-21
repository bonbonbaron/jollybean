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
inline static U8 _validateEcIdx(const System *sP, Activity *aP, const U8 ecIdx) { 
	return (ecIdx >= (U8) arrayGetNElems(aP->ecA));
}

/* EC Getters and Setters */
/* When activating or deactivating, you have to update the system's directory to its new location. */
inline static Entity _getEntity(const void *cmpP) {
	return ((CmpHeader*) cmpP)->owner;
}


inline static void* _getEcPByIndex(const System *sP, Activity *aP, U8 ecIdx) {
  return (void*) (((U8*) aP->ecA) + (sP->ecSz * ecIdx));
}

/* Returns a component's location in the jagged array of components.  */
inline static ECLocation* _getECLocation(System *sP, Entity entity) {
	return (ECLocation*) mapGet(sP->ecLocationMapP, entity);
}

void* sysGetComponent(System *sP, Entity entity) {
	ECLocation *eclP = (ECLocation*) _getECLocation(sP, entity);
	if (eclP)
		return eclP->ecP;
	return NULL;
}

/* Checks if the component, wherever it is in the jagged array, is before the function's stopping point in its array. */
inline static U8 _cmpIsActive(const System *sP, const ECLocation *eclP) {
	return (eclP->ecIdx < sP->activityA[eclP->activityIdx].firstInactiveIdx);
}

static Error  _updateECLocation(System *sP, Entity entity, U8 dstActIdx, U8 dstEcIdx, void *dstEcP) {
	ECLocation *eclP = (ECLocation*) mapGet(sP->ecLocationMapP, entity);
	// This component exists in this system's directory, which means it's moving around right now.
	if (eclP) {
		eclP->activityIdx = dstActIdx;
		eclP->ecIdx       = dstEcIdx;
		eclP->ecP         = dstEcP;
	}
	// This component hasn't been added to map yet, which implies we're in initialization phase.
	else
		return E_BAD_KEY;
	return SUCCESS;
}

static Error _mvEC(System *sP, void *srcEcP, U8 dstActIdx, U8 dstEcIdx) {
  /* First, move the component to its destination. */
	Entity entity = _getEntity(srcEcP);
  ECLocation *eclP = _getECLocation(sP, entity);
  void *dstEcP = _getEcPByIndex(sP, &sP->activityA[dstActIdx], dstEcIdx);
  memcpy(dstEcP, eclP, sP->ecSz);
  /* Then update its location in the system's EC directory. */
  return _updateECLocation(sP, entity, dstActIdx, dstEcIdx, dstEcP);
}

static Error _swapECs(System *sP, void *srcEcP, U8 dstActIdx, U8 dstEcIdx) {
	memcpy(sP->swapPlaceholderP, (const void*) srcEcP, sP->ecSz);
  ECLocation *srcEclP = _getECLocation(sP, _getEntity(srcEcP));
  void *dstEcP = _getEcPByIndex((const System*) sP, &sP->activityA[dstActIdx], dstEcIdx);
	Error e = _mvEC(sP, dstEcP, srcEclP->activityIdx, srcEclP->ecIdx);
	if (!e)
		return _mvEC(sP, sP->swapPlaceholderP, dstActIdx, dstEcIdx);
	return e;
}

/* Deactivates component by swapping it with the last active slot and decrementing the first active index beforehand. */
Error sysDeactivateCmp(System *sP, Entity entity) {
  ECLocation *eclP = _getECLocation(sP, entity);
	if (eclP) 
		return _swapECs(sP, eclP->ecP, eclP->activityIdx, --sP->activityA[eclP->activityIdx].firstInactiveIdx);  /* swap w/ last active EC */
	return E_BAD_INDEX;
}

/* Activates component by swapping it with the first inactive slot and incrementing the first active index afterward. */
inline static Error _activateEC(System *sP, Entity entity) {
  ECLocation *eclP = _getECLocation(sP, entity);
  return _swapECs(sP, eclP->ecP, eclP->activityIdx, sP->activityA[eclP->activityIdx].firstInactiveIdx++);  /* swap w/ first inactive EC */
}

/* Kind of a no-brainer that if you're transferring something, it's going to be active in its destination activity. */
inline static Error _startEcActivity(System *sP, Entity entity, U8 dstActivity) {
	Error e = SUCCESS;
  ECLocation *eclP = _getECLocation(sP, entity);
	if (eclP != NULL)
		return E_BAD_KEY;
  if (eclP->activityIdx == dstActivity)
    e = _activateEC(sP, entity);
  else {
    /* Move EC in destination out of the way then fill vacated spot with source EC. */
    void *ecP = _getEcPByIndex(sP, &sP->activityA[dstActivity], sP->activityA[dstActivity].firstInactiveIdx - 1);
    e = _mvEC(sP, ecP, dstActivity, sP->activityA[dstActivity].firstEmptyIdx++);  /* You should check if destination is already emtpy first. */
		if (!e)
			e = _mvEC(sP, eclP->ecP, dstActivity, sP->activityA[dstActivity].firstInactiveIdx++);
  }
	return e;
}

Error sysIniActivity(System *sP, Activity *aP, U32 nComps) {
	Error e = arrayNew(&aP->ecA, sP->ecSz, nComps);
	if (!e) 
		aP->ownerP = sP;
	return e;
}

static void _sysClrActivities(System *sP) {
	if (sP->activityA != NULL)
		for (U8 i = 0, nActivities = sP->nActivities; i < nActivities; i++) 
			arrayDel((void**) &sP->activityA[i].ecA);
}


// Components are initially spread out across the entity seeds, 
// so we'll initialize those one at a time later into the first activity as deactivated.
// Then the Behavior System will take care of putting those in the proper activity.
Error sysIni(System *sP, U32 nComps) {
	// Sytems with system components need to initialize maps in sysIniFP().
	Error e =  sysNewECMap(sP, nComps);
	// Then allocate enough room for all components in every activity. 
	if (!e) 
		for (U8 i = 0; !e && i < sP->nActivities; i++) 
			e = sysIniActivity(sP, &sP->activityA[i], nComps);

	// Finally call the system's unique initializer.
	if (!e)
		e = (*sP->sysIniSFP)();

	// Clean up if there are any problems
	if (e) {
		_sysClrActivities(sP);
		if (sP->ecLocationMapP != NULL)
			mapDel(&sP->ecLocationMapP);
	}
	
	return e;
}

Error sysNewECMap(System *sP, U8 nElems) {
  return mapNew(&sP->ecLocationMapP, sizeof(ECLocation), nElems);
}

Error sysAddComponent(System *sP, Entity entity, XHeader *xhP) {
	if (!sP || !xhP || !sP->ecLocationMapP)
		return E_BAD_ARGS;

	// Skip entities who already have a component in this system.
	if (mapGet(sP->ecLocationMapP, entity))
		return SUCCESS;

	// Make sure the component belongs to this system. This is only checked at load-time.
	if (xhP->type != sP->id)
		return E_SYS_CMP_MISMATCH;
	
	// Assign this component to its entity and, if necessary, prepare it for the system.
	xhP->owner = entity;
	Error e = (*sP->sysIniCFP)(xhP);  // Inflatable components must be initialized before
	                                  // being added to any activity.

	// If index falls within system's array of activities...
  if (_validateActivityIdx(sP, FIRST_ACTIVITY)) {
    Activity *aP = &sP->activityA[FIRST_ACTIVITY];
		// Copy component to first emtpy slot in specified activity
		void *dstEcP = _getEcPByIndex(sP, aP, aP->firstEmptyIdx);
  	memcpy(dstEcP, xhP, sP->ecSz);
		// Add component's entity-lookup entry to the system's directory.
		ECLocation ecl = {
			.activityIdx = FIRST_ACTIVITY,
			.ecIdx       = aP->firstEmptyIdx++,  // this increments firstEmptyIdx 
			.ecP         = dstEcP
		};
		e = mapSet(sP->ecLocationMapP, entity, &ecl);
		aP->active = 1;  // TODO remove after developing the behavior tree
		aP->firstInactiveIdx++;  // TODO remove after developing the behavior tree
  }
	else 
		e = E_BAD_INDEX;

	return e;
}


Error sysNewMailbox(Mailbox **mailboxPP, U32 nMailSlots) {
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

static void _sysClrMailboxes(System *sP) {
	arrayDel((void**)&sP->inbox.msgA);
	arrayDel((void**)&sP->outbox.msgA);
	sP->inbox.nMsgs = 0;
	sP->outbox.nMsgs = 0;
}

void sysReset(System *sP) {
	/* TODO */
}

void sysClr(System *sP) {
	_sysClrMailboxes(sP);
	_sysClrActivities(sP);
	mapDel(&sP->ecLocationMapP);
}

// TODO:when you have some valid system-wide functions, add them here
#if 0
SysBasicFP sysBasicFuncs[] = {

};
#endif

inline static void _sysReadMessage(System *sP, Message *msgP) {
	switch(msgP->cmdType) {
#if 0
		/* Message intends a system-wide response. All systems share the same pool of systemwide functions. */
		case SYSTEMWIDE_CMD:
			(sysBasicFuncs[msgP->cmd])(sP);
			break;
#endif
		/* Message intends a repeating function call on a component. */
		case REPEATING_CMP_CMD:
			_startEcActivity(sP, msgP->toID, msgP->cmd);
			break;
		/* Message intends a one-time function call on a component. */
		case ONE_OFF_CMP_CMD:
			(*sP->oneOffFPA[msgP->cmd])(sP, msgP->toID, msgP->misc);
			break;
		default:
			break;
	}
}

void _sysReadInbox(System *sP) {
	if (sP != NULL && sP->inbox.msgA != NULL) {
		Message *msgP, *msgLastP;
		arrayIniPtrs(sP->inbox.msgA, (void**) &msgP, (void**) &msgLastP, sP->inbox.nMsgs);
		while (msgP < msgLastP)
			_sysReadMessage(sP, msgP++);
	}
}

/* sysWriteOutbox() is called by implemented systems when specific events occur that may interest the outside world. */
void sysWriteOutbox(System *sP, Message *msgP) {
	memcpy((void*) &sP->outbox.msgA[sP->outbox.nMsgs++], msgP, sizeof(Message));
}

/* This is how the entire ECS framework works. */
void sysRun(System *sP) {
	_sysReadInbox(sP);
	_clrMailbox(&sP->inbox);

	Activity *aP = &sP->activityA[0];
	Activity *aEndP = aP + sP->nActivities;

	for (; aP < aEndP; aP++) 
		if (aP->active)
			(*aP->sysFP)(aP);
}
