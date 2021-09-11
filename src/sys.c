#include "jb.h"

inline static U8 _sysValidateCmpsJBase(const System *sP, const U8 jBaseIdx) { 
	return (jBaseIdx < (U8) arrayGetNElems(sP->componentsJ) && sP->componentsJ[jBaseIdx] == NULL);
}

inline static U8 _sysValidateCmpsJInner(const System *sP, const U8 jBaseIdx, const U8 jInnerIdx) { 
	return (jBaseIdx < (U8) arrayGetNElems(sP->componentsJ[jBaseIdx]));
}

/* Jagged array's base level is statically allocated. */
Error sysNewCmpsJRow(const System *sP, const U8 nCmps, const U8 szCmp, const U8 jBaseIdx) {
	if (_sysValidateCmpsJBase(sP, jBaseIdx))
		return arrayNew(&sP->componentsJ[jBaseIdx], szCmp, nCmps);
	return BAD_ARGS;
}

static void sysIniCmp(const System *sP, const U8 jBaseIdx, const U8 jInnerIdx, const U8 szCmp, const void *cmpP) {
	if (_sysValidateCmpsJBase(sP, jBaseIdx))
		if (_sysValidateCmpsJInner(sP, jInnerIdx))
			memcpy(&sP->componentsJ[jBaseIdx][jInnerIdx], cmpP, szCmp);
}

static void _sysDelCmpsJInners(const System *sP) {
	U8 i = 0, jBaseLen = arrayGetNElems(sP->componentsJ);
	for (; i < jBaseLen; i++) 
		arrayDel(&sP->componentsJ[i]);
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

typedef void (*SysBasicFP)(System *sP);

void sysToggleActive(const System *sP) {
	sP->active != sP->active;
}

typedef enum {
	SYS_TOGGLE_ACTIVE,
	_MAX_SYSTEMWIDE_CMD_ID
} SysCmdID;

SysBasicFP sysBasicFuncs[] = {
	sysToggleActive
};

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
			if (sP->componentsJ != NULL && msgP->cmd >= arrayGetNElems(sP->componentsJ[cmd]))
				return E_MSG_INVALID_CMD;
			break;
		//TODO: handle messages targeting Entities
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
						if (_cmpIsActive(sP, clP)) {
							if (_cmdIsHigherPriority(msP->cmd, clP))

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
// if all the above is true, issue command

/* returns a component's location in the jagged array of components. Each row in the J-array corresponds to a function. */
inline static CmpLocation* _getCmpLocation(System *sP, U8 entityID) {
	CmpLocation *clP = (CmpLocation*) mapGet(&sP->cmpAddressMap, entityID);
}

/* Checks if the component, wherever it is in the jagged array, is before the function's stopping point in its array. */
inline static U8 _cmpIsActive(System *sP, CmpLocation *clP) {
	return sP->componentsJ[clP->jBaseIdx][clP->jInnerIdx] < sP->firstInactiveIdx[clP->jBaseIdx];
}

/* Asks if the requested command is higher priority than on that may already be running on a component. */
inline static U8 _cmdIsHigherPriority(U8 cmd, CmpLocation *clP) {
	return cmd <= clP->jBaseIdx;
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

/* Deactivates component by moving it to the first empty slot. Empty slot is always in the inactive zone of a row. */
inline static U8 _deactivateCmp(System *sP, CmpLocation, *clP) {
	_swapCmps(&sP->componentsJ[clP->jBaseIdx][clP->jInnerIdx], &sP->componentsJ[clP->jBaseIdx][sP->firstInactiveIdxA[
	



	/* If C is in target row and is inactive, swap it with the firstInactiveIdx element and increment firstInactiveIdx.
	 * If C is in a different row, 
	memcpy(sP->firstInactiveIdx[cmd]++
	*/
}


// deactivate component
// activate component
// sysSetInactive
// sysUpdateCmpLocation
// sysReadInbox
// sysWriteMessage   <-- this is called by system functions
// sysRun... automate away the mail I/O and jagged function processing :)
// the actual system functions are on a per-system basis

// sysReadMsg  

/* how sys starts up:
 * T-TO-C MAPPING
 *		B: entities' behaviors
 *		S: System
 *		C: Cmp
 *		Si: System index
 *		Ci: Cmp index
 *		E: Entity
 *		J: Jagged array mapping triggers to Cs
 *		T: Trigger
 *			"histo T and C"
 *			/0)  make histo() function
 *			1)  histo B's Ts  and alias them in an array the same size as the histo. 
 *			2)  histo Es' components, C in each E's G via their C- headers (for checking system compatibility),
 *			    and alias them in array same size as histo for Si.
 *			"make S array"
 *			3)  allocate array of S based on # nonzeros in (2)
 *			4)  create S based on order of nonzeros and tell those S'es what their aliases are
 *			"make T jagged array"
 *			5)  allocate the base of a jagged array, J with the highest alias assigned in (1) 
 *			6)  rn each element of (5), allocate the count from the histo                  
 *			7)  for each E, go through its G and make J[1.alias][counter++] = {Si, Ci}. 
 *			"make E-to-S:C mapping"
 *			8)  Collision S spits out a msg saying E1 collided with a type. So we go to the E array, index E1, and look at its trigger array. It may not respond to collisions; we don't know yet. A flag could say so, and the number of 1s up to the bit can indicate where in that E's behavior array the response to the T is. This limits an E to 32 Cs. But if the game has more than 32 Cs, you have to have an array of integers. All you have to do is mask out the bits beyond the desired one and use a LUT.. 
 *
 *
 *			This is going to be such an incredible game engine. Here are 2 reasons why:
 *			* Writing behaviors is easy: I just need to give the sys key, function enum, parameter key, and desired output when it's done for entire sets of behaviors. That's programming by tiny data instead of huge, 64-bit machine instructions. 
 *			* Writing entities is going to be easy too, because all I have to do there is write an array of pointers to component maps. 
 *
 *			In terms of development, having pre-existing components to borrow from helps a ton. 
