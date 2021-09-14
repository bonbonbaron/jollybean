#include "ecs.h"


/* Components in Arrays: Thought Process
 *     ecs.c sees entities and components as a unit. System *implementations*, however, usually see only the components. They do this by jumping over the entities as they loop through. The components are pulled out of storage and lumped together with entities as they enter systems' bowels. They swap together, move together, delete together; they always stay together. */

/* Only inspect every possible messaging error at load-time. Do not inspect every single message-reading. */
inline static U8 _validateActivityIdx(const System *sP, const U8 activityIdx) { 
	return (activityIdx < (U8) arrayGetNElems(sP->activityA));
}

/* I repeat; Only inspect every possible messaging error at load-time. Do not inspect every single message-reading. */
inline static U8 _validateCmpIdx(const System *sP, const U8 activityIdx, const U8 cmpIdx) { 
	return (cmpIdx < (U8) arrayGetNElems(sP->activityA[activityIdx].cmpsA);
}

/* Wait... does this mean that every single component needs to have an entity member? If I store a Velocity in the database, I write a MotionSystem to work with Velocity components. It knows to jump, and ecs.c doubtless comes packaged with an inlined "getEntity()" utility. So no, it doesn't need an Entity member; they just get injected awkwardly between the gaps of the components by ecs.c lol. */

static void iniActivity(const Activity *aP, const SysFP *funcPtr) {
	aP->sysFP = funcPtr;
	aP->active = 0;
	aP->firstInactiveIdx = 0;
	aP->firstEmptyIdx = 0;
	aP->cmpsA = NULL;
}
	
Error sysNew(System **sPP, U8 sysID, const U8 activityIdx, SysFP *funcPtrA, const U8 szCmp) {
	Error e = jbAlloc(sPP, sizeof(System), 1);
	/* Allocate activity array. */
	if (!e) {
		SysFP *funcP = funcPtrA;
		for (U8 nFuncs = 0; funcP != NULL; ++funcP, ++nFuncs);
		e = arrayNew(&(*sPP)->activityA, sizeof(Activity), nFuncs);
	}

	/* These are left to parent system to allocate for better performance. */
	(*sPP)->cmpLocationMapP = NULL;
	(*sPP)->inboxA = NULL;
	(*sPP)->outboxA = NULL;

	return e;
}

void sysIni(System *sP, SysFP *funcPtrA) {
		/* Populate activity array's function pointers. */
	if (!e) {
		Activity *aP = &sP->activityA[0];
		Activity *aEndP = arrayGetEndPtr(sP->activityA);
		SysFP *funcP = funcPtrA;
		for (; aP < aEndP; ++aP, ++funcP)
			iniActivity(aP, funcP);
	}
	/* For performance, the parent system is in charge of allocating the component arrays within the activity array elements. */
	/* Set individual settings. */
	sP->active = 0;
	sP->cmpStepSz = szCmp + sizeof(Entity);  /* Entities are squeezed between components */
	sP->cmpLocationMapP = NULL;
}

/* Parent system calls this on each system's activities after histogramming the entity seed data. */
Error sysNewCmpsA(const System *sP, const U8 activityIdx, const U8 nCmps) {
	/* Include Entity between every Component. */
	if (_validateActivityIdx(sP, activityIdx))
		return arrayNew(&sP->activityA[activityIdx].cmpsA, sP->cmpStepSz, nCmps);
	return BAD_ARGS;
}

static void sysIniCmp(const System *sP, const U8 activityIdx, const U8 cmpIdx, const void *cmpP) {
	if (_validateActivityIdx(sP, activityIdx))
		if (_validateCmpIdx(sP, activityIdx, cmpIdx))
			memcpy(&sP->activityA[activityIdx].cmpsA[cmpIdx], cmpP, szCmp);
}

static void _sysDelActivities(const System *sP) {
	for (U8 i = 0, nActivities = arrayGetNElems(sP->activityA); i < nActivities; i++) 
		arrayDel(&sP->activityA[i]);
	arrayDel(&sP->activityA);
}	

Error sysNewMailbox(Msg **mailboxPP, U32 nMailSlots) {
	return arrayNew(mailboxPP, sizeof(Message), nMailSlots);
}
		
static void _sysClrMailbox(const Message *mailboxP) {
	memset(mailboxP, 0, sizeof(Message) * arrayGetNElems(mailboxP));
}

static void _sysDelMailboxes(const System *sP) {
	arrayDel(&sP->inboxA);
	arrayDel(&sP->outboxA);
}

void sysReset(const System *sP) {
	_sysDelCmpJInners(sP);
	_sysDelMailboxes(sP);
}

void sysToggleActive(const System *sP) {
	sP->active != sP->active;
}

/* Validate message before processing. */
inline static Error _validateMessage(const System *sP, const Message *msgP, const ECSType expectedECSType, const U32 expectedID) {
	if (msgP->toECSType != expectedECSType) 
		return E_MSG_TO_ECS_TYPE_MISMATCH;
	switch(msgP->toECSType) {
		case SYSTEM:
			if (msgP->toECSType == SYSTEM && msgP->toID != expectedID)
				return E_MSG_TO_ID_MISMATCH;
			if (msgP->cmd >= _MAX_SYSTEMWIDE_CMD_ID)  // every system has its own max enum for system-specific commands
				return E_MSG_INVALID_CMD;
			break;
		case COMPONENT:
			if (msgP->cmd >= sP->nFuncs)
				return E_MSG_INVALID_CMD;
			if (sP->activityA != NULL && msgP->cmd >= arrayGetNElems(sP->activityA[cmd]))
				return E_MSG_INVALID_CMD;
			break;
		//TODO: handle messages targeting Entities; honestly, when will it be C instead of E? Isn't E more important?
		default:
			E_MSG_TO_ECS_TYPE_MISMATCH;
	}
	return SUCCESS;
}

Error sysReadMessage(System *sP, Message *msgP, U8 maxCmdID) {
	Error e = SUCCESS;
	if (sP != NULL && msgP != NULL) {
		switch(msgP->cmdType) {
			/* Message intends a system-wide response. All systems share the same pool of systemwide functions. */
			case SYSTEMWIDE_CMD:
				e = _validateMessage(sP, msgP, SYSTEM, sP->id);
				if (!e)
					(*sysBasicFuncs[msgP->cmd])();
				break;
			/* Message intends a repeating function call on a component. */
			case REPEATING_CMP_CMD:
				e = _validateMessage(sP, msgP, COMPONENT, 0);
				if (!e) {
					CmpLocation *clP = _getCmpLocation(sP, msgP->toID);
					if (clP != NULL) {
						// if cmp is already active, you have to swap it with full knowledge of its whereabouts later.
						if (_cmpIsActive(sP, clP)) {

						} else 
					}
				}
				break;
			/* Message intends a one-time function call on a component. */
			case ONE_OFF_CMP_CMD:
				e = _validateMessage(sP, msgP, COMPONENT, 0);
				// TODO: Change this to be one-off functions. One-offs are simply toggling  activeness or moving cmp to a new loop.
				break;
			default:
				break;
		}
	}
}

/* Returns a component's location in the jagged array of components. Each row in the J-array corresponds to a function. */
inline static CmpLocation* _getCmpLocation(System *sP, U8 entityID) {
	CmpLocation *clP = (CmpLocation*) mapGet(&sP->cmpAddressMap, entityID);
}

/* When activating or deactivating, you have to update the system's directory to its new location. */
inline static void _updateCmpLocation(const System *sP, const void *cmpP, const U8 activityIdx, const U8 cmpIdx) {kk
	Entity entity = getEntity((CmpHeader*) cmpP);
	CmpLocation *clP = _getCmpLocation(sP, entity);  
	clP->activityIdx = activityIdx;
	clP->cmpIdx = cmpIdx;
}

/* Checks if the component, wherever it is in the jagged array, is before the function's stopping point in its array. */
inline static U8 _cmpIsActive(const System *sP, const CmpLocation *clP) {
	return sP->activityA[clP->activityIdx][clP->cmpIdx] < sP->firstInactiveIdx[clP->activityIdx];
}

inline static Entity getEntity(const CmpHeader *cmpHeaderP) {
	return cmpHeaderP->owner;
}

inline static _mvCmp(

/* TODO
 * I can see you getting into a big mess with swapping. 
 * When activating locally:
 *		swap them immediately ("swap" entails placeholding one component and then performing the move + update pair on each)
 *		update their locations with their component header's entity ID, the current row's index, and the index of each component's new index in this row
 *		increment firstInactiveIdx
 * When activating globally:
 *		move the destination row's first inactive element with the first empty slot 
 *		update its location with " "  (see similar step above)
 *		post-increment the destination row's firstEmptyIdx in the above step
 *		move the activated component from source row to the destination row's firstInactiveIdx
 *		update its location with " "  (see similar step above)
 *		post-increment the destination row's firstInactiveIdx in the above step
 *		in source row, swap the
 *		decrement the source row's firstEmptyIdx
 *
 *	It's coming clear move + update location is a unit. So perform both steps in a mvCmp() function; makes enough sense to do so. 
 *		
 * For example, you have to move components from one activity to another, but then you have to update the firstInactiveIdx in both activities, update the components' locations in their maps, and swap to fill the hole in the source activity. So before you do anything, be thoughtful about it and write it out on paper. Then after you figure it out, you'll be good to go. */

/* TODO:
 *		when initializing a component, you have to allocate the size of the component plus the component's header. You also have to pass in the entity to populate the hdr->entity, and then you have to add the header size to the raw component data size. Make sure the map allocates memory with this same information. */
/* Moves a component to a different slot without regard for contents of destination. Clears source slot after. */
static void _mvCmp(void *srcCmpP, void *dstCmpP, U8 cSz) {

}

static void _swapCmps(void *c1P, void *c2P, U8 cSz) {
	void *swapPlaceholderP;
	Error e = jbAlloc(&swapPlaceholderP, cSz, 1);
	if (!e) {
		memcpy(swapPlaceholderP, c1P, cSz);
		memcpy(c2P, c1P, cSz);
		memcpy(c1P, swapPlaceholderP, cSz);
	}
}

/* Deactivates component by swapping it with the last active slot and decrementing the first active index beforehand. */
inline static void _deactivateCmp(System *sP, CmpLocation, *clP) {
	Activity *aP = &sP->activityA[clP->activityIdx];
	_swapCmps(&aP->cmpsA[clP->cmpIdx], 
						&aP->cmpsA[aP->firstInactiveIdx - 1]);
	_updateCmpLocation(sP, clP->activityIdx, --(aP->firstInactiveIdx));
}

/* Activates component by swapping it with the first inactive slot and incrementing the first active index afterward. */
inline static void _activateCmp(System *sP, CmpLocation, *clP) {
	Activity *aP = &sP->activityA[clP->activityIdx];
	_swapCmps(&aP->cmpsA[clP->cmpIdx], 
						&aP->cmpsA[aP->firstInactiveIdx + 1]);
	_updateCmpLocation(sP, clP->activityIdx, aP->firstInactiveIdx++);
}

// sysIniPtrs: move iniPtr past 1st entity and  calculate endPtr w/ firstInactiveIdx
// sysReadMsg  
// sysReadInbox
// sysWriteOutbox   <-- this is called by system functions
// sysRun... automate away the mail I/O and activity-processing inside it :)
