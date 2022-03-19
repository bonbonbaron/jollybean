#include "xGo.h"

static Error _histoHiveMindTriggers(U32 **histoAP, XGoIniSeedPkg *seedPkgP, U32 *totalDistinctHiveMindsP) {
	// An  N-byte number can represent 1 << (N * 8) values.
	U32 N = 1 << (sizeof(Key) * 8);
	Error e = histoNew(histoAP, N);
	U32 *histoA;

	if (!e)
		histoA = *histoAP;

	if (!e) {
		XGoIniSeed *seedP = seedPkgP->seedA;
		XGoIniSeed *seedEndP = seedP + seedPkgP->nSeeds;
		for (; seedP < seedEndP; seedP++) {
			Quirk *quirkP = seedP->personalityP->quirkPA[0];
			Quirk *quirkEndP = quirkP + seedP->personalityP->nQuirks;
			for (; quirkP < quirkEndP; quirkP++) {
				*totalDistinctHiveMindsP += (!histoA[quirkP->trigger]);
				histoA[quirkP->trigger]++;
			}
		}
	}

	if (e && histoA)
		histoDel(histoAP);

	return e;
}

static Error _distributeHiveMinds(XGo *xGoSysP, XGoIniSeedPkg *seedPkgP) {
	if (!xGoSysP || !seedPkgP)
		return E_BAD_ARGS;

	U32 *histoA = NULL;
	U32 nDistinctHiveMinds = 0;
	// Histo the number of quirks existing for each trigger.
	Error e = _histoHiveMindTriggers(&histoA, seedPkgP, &nDistinctHiveMinds);
	if (!nDistinctHiveMinds)
		goto nevermind;
	// New hiveMind map
	if (!e)
		e = mapNew(&xGoSysP->hiveMindMP, sizeof(void*), nDistinctHiveMinds);
	// New array of entities for each trigger key's value
	if (!e) {
		U32 nElems = arrayGetNElems(histoA);
		for (U32 i = 0; !e && i < nElems; i++) 
			if (histoA[i]) {
				Entity *entityA = NULL;
				e = arrayNew((void**) &entityA, sizeof(Entity), histoA[i]);
				if (!e) {
					void *voidPtr = mapGet(xGoSysP->hiveMindMP, i);
					if (voidPtr)
						voidPtr = entityA;
				}
			}
	}
	// Form hive minds out of entities listening for same triggers.
	if (!e) {
		XGoIniSeed *seedP = seedPkgP->seedA;
		XGoIniSeed *seedEndP = seedP + seedPkgP->nSeeds;
		for (; seedP < seedEndP; seedP++) {
			Quirk *quirkP = seedP->personalityP->quirkPA[0];
			Quirk *quirkEndP = quirkP + seedP->personalityP->nQuirks;
			for (; quirkP < quirkEndP; quirkP++) {
				Entity *entityA = (Entity*) mapGet(xGoSysP->hiveMindMP, quirkP->trigger);
				// Stick entity in last empty slot for efficiency.
				if (entityA)
					entityA[--histoA[quirkP->trigger]] = seedP->entity;
			}
		}
	}

	nevermind:
	if (histoA)
		histoDel(&histoA);

	return e;
}

Error xGoIniSys(System *sP, void *sParamsP) {
	if (!sP || !sParamsP)
		return E_BAD_ARGS;
	XGo *xGoSysP = (XGo*) sP;
	XGoIniSeedPkg *seedPkgP = (XGoIniSeedPkg*) sParamsP;
	// Allocate array of blackboard pointers.
	Error e = arrayNew((void**) &xGoSysP->bbPA, sizeof(Blackboard*), seedPkgP->nSeeds);
	// Add all components to system.
	XGoIniSeed *seedP = seedPkgP->seedA;
	XGoIniSeed *seedEndP = seedP + seedPkgP->nSeeds;;
	XGoComp dummyComp = {.xHeader = { .type = sP->id }};
	for (; !e && seedP < seedEndP; seedP++) {
		e = xAddComp(sP, seedP->entity, &dummyComp.xHeader);
		if (!e)
			xGoSysP->bbPA[seedP->entity] = seedP->bbP;
	}
	// Init hive minds.
	if (!e)
		e = _distributeHiveMinds(xGoSysP, seedPkgP);
	if (e)
		xGoClr(sP);
	return e;
}

Error xGoIniComp(System *sP, XGoComp *cP) {
	XGo *xGoSysP = (XGo*) sP;
	Entity entity = cP->xHeader.owner;
	cP->bbIdx = xGoSysP->nBBsSet++;  // Assuming the caller populates sP->bbA.
	cP->priority = 0;
	cP->btP = NULL;  // Nobody starts out knowing what they'll do. 
	// Initialize entity's hard-coded map of reactions if not already (note: singleton!).
	return xIniCompMapP(sP, entity);
}

void xGoClr(System *sP) {
	XGo *xGoSysP = (XGo*) sP;
	arrayDel((void**) &xGoSysP->bbPA);
	if (xGoSysP->hiveMindMP && xGoSysP->hiveMindMP->mapA) {
		Entity **entityPP = xGoSysP->hiveMindMP->mapA;
		Entity **entityEndPP = xGoSysP->hiveMindMP->mapA;
		for (; entityPP < entityEndPP; entityPP++)
			arrayDel((void**) entityPP);
	}
	mapDel(&xGoSysP->hiveMindMP);
	mapDel(&xGoSysP->triggerMP);
}

inline static U8 _isHigherPriority(U8 newPriority, U8 existingPriority) {
	return newPriority > existingPriority;
}

static Error _triggerIndividual(XGo *xGoSysP, Message *msgP) {
	Entity entity = msgP->topic;
	// Get entity's reaction to message's trigger.
	XGoComp *compP = (XGoComp*) xGetComp(&xGoSysP->system, entity);
	Map *compMapP = xGetCompMapP(&xGoSysP->system, entity);
	if (!compMapP)
		return E_BAD_KEY;
	Reaction *reactionP = (Reaction*) mapGet(compMapP, msgP->msg);
	if (!reactionP)
		return E_BAD_KEY;

	// Queue new action if it's higher priority than old or entity's inactive.
	if (!xCompIsActive(&xGoSysP->system, entity) || 
			_isHigherPriority(reactionP->priority, compP->priority)) {
		compP->priority = reactionP->priority;
		compP->btP = reactionP->btP;
	}
	return SUCCESS;
}

static Error _triggerHiveMind(XGo *xGoSysP, Message *msgP) {
	assert(msgP);
	Error e = SUCCESS;
	Entity *eA = (Entity*) mapGet(xGoSysP->hiveMindMP, msgP->msg);
	if (!eA)
		return E_BAD_KEY;
	assert(eA);
	Entity *eP, *eEndP;
	arrayIniPtrs((void*) eA, (void**) &eP, (void**) &eEndP, -1);
	for (; !e && eP < eEndP; eP++) {
    msgP->to = *eP;
		e = _triggerIndividual(xGoSysP, msgP);
  }
	return e;
}

// Entity acts on message if it's more urgent than its current activity.
Error xGoProcessMessage(System *sP, Message *msgP) {
	Error e;
	XGo *xGoSysP = (XGo*) sP;
	if (msgP->topic)
		e = _triggerIndividual(xGoSysP, msgP);
	else
		e = _triggerHiveMind(xGoSysP, msgP);

	// Make sure the above reaction is higher priority than any active task.
	return e;
}

Error xGoRun(Focus *fP) {
	XGo *xGoSysP = (XGo*) fP->ownerP;
	XGoComp *cP, *cEndP;
	cP = (XGoComp*) fP->compA;
	cEndP = cP + fP->firstInactiveIdx;

	// TODO: add logic for putting tree to sleep based on returned NodeStatus.
	for (; cP < cEndP; cP++) 
		btRun(cP->btP, xGoSysP->bbPA[cP->bbIdx], xGoSysP->system.outboxP);

	return SUCCESS;
}
