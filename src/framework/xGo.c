#include "xGo.h"

Error xGoIniSys(System *sP, void *sParamsP) {
	if (!sP || !sParamsP)
		return E_BAD_ARGS;

	XGo *xGoSysP = (XGo*) sP;
	XGoIniSeedPkg *seedPkgP = (XGoIniSeedPkg*) sParamsP;

	//TODO use nDistinctIndividualQuirks to allocate triggerMP
	//TODO use nDistinctHiveMindQuirks to allocate hiveMindMP
	//TODO change hivemind to an array of Entities. 
	//TODO move trigger() and triggerGroup() to here. 
	//TODO figure out how to populate hcmP in compDirectory.
	//TODO Then make trigger() call mapGet() on each entity's hcmP of BTreeSingletons for BT.

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

	if (e)
		xGoClr(sP);

	return e;
}
/*
typedef struct {
	System system;           // done
	Key          nBBsSet;    // done
	Blackboard **bbPA;			 // done
	Map         *triggerMP;
	Map         *hiveMindMP; 
} XGo;
*/

Error xGoIniComp(System *sP, XGoComp *cP) {
	XGo *xGoSysP = (XGo*) sP;
	cP->bbIdx = xGoSysP->nBBsSet++;  // Assuming the caller populates sP->bbA.
	cP->priority = 0;
	cP->btP = NULL;  // Nobody starts out knowing what they'll do. 
	return SUCCESS;	 // That's what the START event at scene-start is for.
}

void xGoClr(System *sP) {
	XGo *xGoSysP = (XGo*) sP;
	arrayDel((void**) &xGoSysP->bbPA);
	mapDel(&xGoSysP->hiveMindMP);
	mapDel(&xGoSysP->triggerMP);
}


// Entity acts on message if it's more urgent than its current activity.
Error xGoProcessMessage(System *sP, Message *msgP) {
	Error e = SUCCESS;
	Entity entity = msgP->topic;
	XGo *xGoSysP = (XGo*) sP;

	Reaction *reactionP = (Reaction*) mapGet(xGoSysP->triggerMP, msgP->msg);
	// Make sure the above reaction is higher priority than any active task.
	XGoComp *compP = (XGoComp*) xGetComp(sP, entity);
	if (!xCompIsActive(sP, entity) || reactionP->priority > compP->priority) {
		compP->priority = reactionP->priority;
		compP->btP = reactionP->btP;
	}
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
