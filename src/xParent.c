#include "ecs.h"
//***************************************************
//** Parent systems *********************************
//***************************************************
static Error sParentReact(Activity *aP);
static Error sParentTick(Activity *aP);
static Map **_entityReactionMA;
static Map *_subscriberAMP;
static System **_sPA;

NEW_SYS_(Parent, 0, NULL,
	ACTIVITY_(REACT, sParentReact),
	ACTIVITY_(TICK, sParentTick),
);
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
	for (Entity entityCounter = N_SYS_TYPES; !e && gPP < gEndPP; gPP++, entityCounter++) {
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
Error xParentIniS(void *sParamsP) {
	if (!sParamsP)
		return E_BAD_ARGS;

	SParentIniSParams *sParentIniSPrmsP = (SParentIniSParams*) sParamsP;
	// Histogram gene types
	U32 *histoP = NULL;
	Error e = histoNew(&histoP, N_SYS_TYPES);
	if (!e)
		e = _histoGeneTypes(histoP, sParentIniSPrmsP->genomeGroupP);
	
	// Add subsystems as components to Parent system.
	extern System sControl;
	sAddC(&sParent, sControl.id, &sControl.xHeader); // Add control sys no matter what
	for (U32 i = 0; !e && i < sParentIniSPrmsP->nSystems; i++) 
		if (_sPA[i])
			e = sAddC(&sParent, _sPA[i]->id, &_sPA[i]->xHeader);

	// Initialize Parent's component subsystems before we throw genes in.
	if (!e) {
		U32 *hP, *hEndP;
		arrayIniPtrs(histoP, (void**) &hP, (void**) &hEndP, -1);
		for (Entity sID = 0; !e && hP < hEndP; hP++, sID++) {
			if (*hP)  {  // i.e. if any entities exist having components for this system...
				System *sP = (System*) sGetC(&sParent, sID);
				if (sP)
					e = sIni(sP, *hP, sP->sIniSParamsP);  // makes subsystem's EC map and activities
			}
		}
	}
	
	// Distribute the genes to the proper systems.
	if (!e)
		e = _distributeGenes(&sParent, sParentIniSPrmsP->genomeGroupP);
	
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
// msgP may contain information the triggered callback finds pertinent; e.g. "Who collided w/ me?"
//TODO: make a way to stop all active components of an entity, and no more than that, in order to efficiently switch between an entity's reactions
static Error _trigger(System *sParentP, Message *msgP) {
	U32 nComponents = sGetNComponents(sParentP); // TODO: get rid of this in release version
	// Requested reaction
	assert(msgP->to >= nComponents && msgP->to >= N_SYS_TYPES);  // highest system will have entity ID <= N_SYS_TYPES
	Map *mapP = _entityReactionMA[msgP->to];  // toID is an entity
	Reaction *requestedReactionP = (Reaction*) mapGet(mapP, msgP->contents.event.id);
	// Ongoing reaction, if one exists
	assert(requestedReactionP);
	Reaction *ongoingReactionP = (Reaction*) sGetC(&sParent, msgP->to);
	assert(ongoingReactionP);
	// If entity is busy, see if we're allowed to interrupt it.
	Activity *currActivityP = sGetActivityFromE(&sParent, msgP->to);
	assert(currActivityP);
	if (currActivityP->id == TICK && sComponentIsActive(&sParent, msgP->to)) 
		if (requestedReactionP->priority < ongoingReactionP->priority)  // request is lower priority than current activity
			return SUCCESS;
	// Entity is either idle or doing something less important. Give it something better to do.
	*ongoingReactionP = *requestedReactionP;
	ongoingReactionP->msg = *msgP;
	// The callbacks will be called in the REACT activity.
	return sStartCActivity(&sParent, msgP->to, REACT);
}

static Error _triggerGroup(System *sParentP, Message *msgP) {
	assert(msgP);
	Error e = SUCCESS;
	Entity *eA = (Entity*) mapGet(_subscriberAMP, msgP->contents.event.id);
	assert(eA);
	Entity *eP, *eEndP;
	arrayIniPtrs((void*) eA, (void**) &eP, (void**) &eEndP, -1);
	for (; !e && eP < eEndP; eP++)
		e = _trigger(sParentP, msgP);
	return e;
}

Error sParentReact(Activity *aP) {
	assert(aP);
	XParentC *cP, *cEndP;
	Error e = SUCCESS;
	// Check active subsystems' outboxes. Their callbacks populate systems and JB's reaction activity.
	Activity *rcaP = sGetActivity(&sParent, TICK);  // the other activity's components are Systems
	assert(rcaP);
	arrayIniPtrs(rcaP->cA, (void**) &cP, (void**) &cEndP, rcaP->firstInactiveIdx);
	for (; !e && cP < cEndP; cP++) {
		for (Message *msgP = cP->s.outbox.msgA, *msgEndP = cP->s.outbox.msgA + cP->s.outbox.nMsgs;
				 !e && msgP < msgEndP;
				 msgP++) {
			if (msgP->to) 
				e = _trigger(aP->ownerP, msgP);
			else
				e = _triggerGroup(aP->ownerP, msgP);
		}
	}
	
	// Run active reactions until they're complete, at which point we deactivate them.
	arrayIniPtrs(rcaP->cA, (void**) &cP, (void**) cEndP, rcaP->firstInactiveIdx);
	for (; !e && cP < cEndP; cP++)
		if (cP->r.cb(&cP->r.msg, cP->r.paramsP) == COMPLETE) 
			sDeactivateC(&sParent, cP->r.msg.to);

	return SUCCESS;
}

// This runs Parent's subsystems.
Error sParentTick(Activity *aP) {
	assert(aP);
	XParentC *cP, *cEndP;
	Error e = SUCCESS;
	arrayIniPtrs(aP->cA, (void**) &cP, (void**) &cEndP, aP->firstInactiveIdx);
	for (cP = aP->cA; !e && cP < cEndP; cP++) {
		sRun(&cP->s);
		// Put idle systems to sleep.
		if (cP->s.firstInactiveActIdx == 0)
			e = sDeactivateC(&sParent, cP->s.id);
	}
	return e;
}

// To change the hokey static variable assignment, I'd like to change the def of sIniSFP():
//		number of systems
//
Error xIni(System **sPA, U16 nSystems, GenomeGrp *genomeGroupP) {
	// sIni() takes no arguments, yet the parent system needs to know some outside factors.
	// So we use static variables here. 
	if (!sPA || nSystems <= 1 || !genomeGroupP)
		return E_BAD_ARGS;

	SParentIniSParams sParentIniSPrms = {
		.nSystems = nSystems,
		.genomeGroupP = genomeGroupP,
		.sysPA = sPA
	};

	return sIni(&sParent, N_SYS_TYPES, (void*) &sParentIniSPrms);
	// TODO: kick off the parent system activities here
}

Error xRun() {
	sAct(&sParent);
	return SUCCESS;
}
