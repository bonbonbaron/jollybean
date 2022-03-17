#include "xGo.h"

Error xGoIniSys(System *sP, void *sParamsP) {
	// I don't knwo what sParams are yet; an array of personalities?
	// A biome is an array of seeds, each with one personality. 
	// So I have to receive the biome. Only way to eliminate needing
	// to know about the biome is to unpackage the seed into parallel
	// arrays of genomes and personalities. Packaging is safer and
	// saner. 
	//
	// Yet I don't want to have the XMain system worry about all the
	// unpackaging and distributing. But then who's going to histo the
	// genomes? No one system is responsible for histoing the genes of
	// its peers. That's truly an XMain job.
	//
	// And look: XGo has to have an array of Reactions already. So for
	// the sanity of the developer, make XMain do the work.
	//
	return SUCCESS;
}

Error xGoIniComp(System *sP, XGoComp *cP) {
	XGo *xGoSysP = (XGo*) sP;
	cP->bbIdx = xGoSysP->nBBsSet++;  // Assuming the caller populates sP->bbA.
	cP->btP = NULL;  // Nobody starts out knowing what they'll do. 
	return SUCCESS;	 // That's what the START event at scene-start is for.
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
		btRun(cP->btP, &xGoSysP->bbA[cP->bbIdx], xGoSysP->system.outboxP);

	return SUCCESS;
}
