#include "ecs.h"

/* Go ahead and provide these functions here:
	1) Histogramming entitieSeeds' genes
	2) Validating entitySeeds' behaviors (e.g. every system function-index is lower than the system's highest function index enum)
*/

/**************/
/* Components */
/**************/
const static GetGeneFP getGeneFPA[] = {getUnaryGene, getBinaryGene, getNonbinaryGene};

/* Unary gene-selection */
void* getUnaryGene(U8 *geneHeaderP, Key *overrideKeyP) {
	_UNUSED(overrideKeyP);
	return ((UnaryGene*) geneHeaderP)->unaryP;
}

/* Binary gene-selection */
void* getBinaryGene(U8 *geneHeaderP, Key *overrideKeyP) {
	BinaryGene *bP = (BinaryGene*) geneHeaderP;
	if (overrideKeyP)
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
inline static U8 _getNActivities(const System *sP) {
  return (U8) arrayGetNElems((void*) sP->activityA);
}

/* Only inspect every possible messaging error at load-time. Do not inspect every single message-reading. */
inline static U8 _validateActivityIdx(const System *sP, const U8 activityIdx) { 
	return (activityIdx < _getNActivities(sP));
}

/* Only inspect every possible messaging error at load-time. Do not inspect every single message-reading. */
inline static U8 _validateEcIdx(const System *sP, Activity *aP, const U8 ecIdx) { 
	return (ecIdx < (U8) arrayGetNElems(aP->ecA));
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

/* Checks if the component, wherever it is in the jagged array, is before the function's stopping point in its array. */
inline static U8 _cmpIsActive(const System *sP, const ECLocation *eclP) {
	return (eclP->ecIdx < sP->activityA[eclP->activityIdx].firstInactiveIdx);
}

inline static void _updateECLocation(ECLocation *eclP, U8 dstActIdx, U8 dstEcIdx, void *dstEcP) {
  eclP->activityIdx = dstActIdx;
  eclP->ecIdx       = dstEcIdx;
  eclP->ecP         = dstEcP;
}

static void _mvEC(System *sP, void *srcEcP, U8 dstActIdx, U8 dstEcIdx) {
  /* First, move the component to its destination. */
  ECLocation *eclP = _getECLocation(sP, _getEntity(srcEcP));
  void *dstEcP = _getEcPByIndex(sP, &sP->activityA[dstActIdx], dstEcIdx);
  memcpy(dstEcP, eclP, sP->ecSz);
  /* Then update its location in the system's EC directory. */
  _updateECLocation(eclP, dstActIdx, dstEcIdx, dstEcP);
}

static void _swapECs(System *sP, void *srcEcP, U8 dstActIdx, U8 dstEcIdx) {
	memcpy(sP->swapPlaceholderP, (const void*) srcEcP, sP->ecSz);
  ECLocation *srcEclP = _getECLocation(sP, _getEntity(srcEcP));
  void *dstEcP = _getEcPByIndex((const System*) sP, &sP->activityA[dstActIdx], dstEcIdx);
	_mvEC(sP, dstEcP, srcEclP->activityIdx, srcEclP->ecIdx);
	_mvEC(sP, sP->swapPlaceholderP, dstActIdx, dstEcIdx);
}

/* Deactivates component by swapping it with the last active slot and decrementing the first active index beforehand. */
inline static void _deactivateEC(System *sP, Entity entity) {
  ECLocation *eclP = _getECLocation(sP, entity);
  _swapECs(sP, eclP->ecP, eclP->activityIdx, --sP->activityA[eclP->activityIdx].firstInactiveIdx);  /* swap w/ last active EC */
}

/* Activates component by swapping it with the first inactive slot and incrementing the first active index afterward. */
inline static void _activateEC(System *sP, Entity entity) {
  ECLocation *eclP = _getECLocation(sP, entity);
  _swapECs(sP, eclP->ecP, eclP->activityIdx, sP->activityA[eclP->activityIdx].firstInactiveIdx++);  /* swap w/ first inactive EC */
}

/* Kind of a no-brainer that if you're transferring something, it's going to be active in its destination activity. */
inline static void _startEcActivity(System *sP, Entity entity, U8 dstActivity) {
  ECLocation *eclP = _getECLocation(sP, entity);
	if (eclP != NULL)
		return;
  if (eclP->activityIdx == dstActivity)
    _activateEC(sP, entity);
  else {
    /* Move EC in destination out of the way then fill vacated spot with source EC. */
    void *ecP = _getEcPByIndex(sP, &sP->activityA[dstActivity], sP->activityA[dstActivity].firstInactiveIdx - 1);
    _mvEC(sP, ecP, dstActivity, sP->activityA[dstActivity].firstEmptyIdx++);  /* You should check if destination is already emtpy first. */
		_mvEC(sP, eclP->ecP, dstActivity, sP->activityA[dstActivity].firstInactiveIdx++);
  }
}

static void iniActivity(SysFP funcPtr, Activity *aP) {
	aP->sysFP = funcPtr;
	aP->active = 0;
	aP->firstInactiveIdx = 0;
	aP->firstEmptyIdx = 0;
	aP->ecA = NULL;
}
	
Error sysNew(System **sPP, U8 sysID, const U8 ecSz, U8 nFuncs) {
	Error e = jbAlloc((void**) sPP, sizeof(System), 1);
	/* Allocate activity array. */
	if (!e) {
		e = arrayNew((void**) &(*sPP)->activityA, sizeof(Activity), nFuncs);
  }
  if (!e) 
    e = jbAlloc((void**)&(*sPP)->swapPlaceholderP, ecSz, 1);
	/* These are left to parent system to allocate for better performance. */
  if (!e) 
	  (*sPP)->ecLocationMapP = NULL;
  else {
    /* If something bad happened, clean up. */
    arrayDel((void**) &(*sPP)->activityA);
    jbFree((void**) &(*sPP)->swapPlaceholderP);
    jbFree((void**) sPP);
  }
	return e;
}

void sysIni(System *sP, U8 sysID, SysFP *funcPtrA, U8 ecSz) {
		/* Populate activity array's function pointers. */
	Activity *aP, *aEndP;
	SysFP *funcP = funcPtrA;
  arrayIniPtrs((void*) sP->activityA, (void**) &aP, (void**) &aEndP, -1);
	while (aP < aEndP)
		iniActivity(*funcP++, aP++);
	/* For performance, the parent system is in charge of allocating the component arrays within the activity array elements. */
	/* Set individual settings. */
	sP->active = 0;
  sP->ecSz = ecSz;
	sP->id = sysID;
	sP->ecLocationMapP = NULL;
}

/* Parent system calls this on each system's activities after histogramming the entity seed data. */
Error sysNewCmpsA(const System *sP, const U8 activityIdx, const U8 nCmps) {
	/* Include Entity between every Component. */
	if (_validateActivityIdx(sP, activityIdx))
		return arrayNew((void**) &sP->activityA[activityIdx].ecA, sP->ecSz, nCmps);
	return E_BAD_ARGS;
}

Error sysNewCmpMap(System *sP, U8 elemSz, U8 nElems) {
  return mapNew(&sP->ecLocationMapP, elemSz, nElems);
}

#if 0
static void sysIniCmp(const System *sP, const U8 activityIdx, const Entity entity, const void *cmpP) {
  if (_validateActivityIdx(sP, activityIdx)) {
    Activity *aP = &sP->activityA[activityIdx];
  	memcpy(_getEcPByIndex(sP, aP, aP->firstEmptyIdx++), cmpP, sP->ecSz);
  }
}
#endif

static void _sysDelActivities(System *sP) {
	for (U8 i = 0, nActivities = _getNActivities(sP); i < nActivities; i++) 
		arrayDel((void**) &sP->activityA[i].ecA);
	arrayDel((void**) &sP->activityA);
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
		
#if 0  /* get rid of compiler warnings for now */
static void _clrMailbox(Mailbox *mailboxP) {
	memset(mailboxP->msgA, 0, sizeof(Message) * arrayGetNElems(mailboxP));
}
#endif

static void _sysDelMailboxMsgs(System *sP) {
	arrayDel((void**)&sP->inbox.msgA);
	arrayDel((void**)&sP->outbox.msgA);
}

void sysReset(System *sP) {
	/* TODO */
}

void sysDel(System **sPP) {
	_sysDelMailboxMsgs(*sPP);
	_sysDelActivities(*sPP);
	mapDel(&(*sPP)->ecLocationMapP);
	jbFree((void**) sPP);
}

void sysToggleActive(System *sP) {
	sP->active = !(sP->active);
}

SysBasicFP sysBasicFuncs[] = {
	sysToggleActive
};

/* This used to validate messages. Now, we boost performance validating entity behaviors externally only once at load-time instead. */
inline static void sysReadMessage(System *sP, Message *msgP) {
	switch(msgP->cmdType) {
		/* Message intends a system-wide response. All systems share the same pool of systemwide functions. */
		case SYSTEMWIDE_CMD:
			(sysBasicFuncs[msgP->cmd])(sP);
			break;
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

void sysReadInbox(System *sP) {
	if (sP != NULL && sP->inbox.msgA != NULL) {
		Message *msgP, *msgLastP;
		arrayIniPtrs(sP->inbox.msgA, (void**) &msgP, (void**) &msgLastP, sP->inbox.nMsgs);
		while (msgP < msgLastP)
			sysReadMessage(sP, msgP++);
	}
}

/* sysWriteOutbox() is called by implemented systems when specific events occur that may interest the outside world. */
void sysWriteOutbox(System *sP, Message *msgP) {
	memcpy((void*) &sP->outbox.msgA[sP->outbox.nMsgs++], msgP, sizeof(Message));
}

/* This is how the entire ECS framework works. */
void sysRun(System *sP) {
	sysReadInbox(sP);
	Activity *aP, *aEndP;
	arrayIniPtrs(sP->activityA, (void**) &aP, (void**) &aEndP, -1);
	while (aP < aEndP) 
		(*aP->sysFP)(sP, aP, aP->ecA);
}
