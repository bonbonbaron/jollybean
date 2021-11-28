#include "xJollybean.h"

static Entity entityCounter;
//TODO:
//	Make messaging system... Then if it all works, Jollybean is done! 
NEW_GENOME_(bigger, &biggerXRenderC.xHeader);
GENOME_GROUP_(grp1, &bigger, &bigger, &bigger);

NEW_SYS_(Jollybean, JOLLYBEAN, 
	ACTIVITY_(JB_REACT, xJollybeanReact),
	ACTIVITY_(JB_RUN_CHILDREN, xJollybeanRunChildren),
);

Map **entityReactionMA;
U32 nEntities;
Map *subscriberAMP;

static System *sPA[] = {
	&sRender
}; 

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
static Error _distributeGenes(System *sJollybeanP, GenomeGrp *ggP) {
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
			sP = (System*) sGetC(&sJollybean, (*xhPP)->type);  // We don't set the owner of the gene pool.
			if (sP)
				e = sAddC(sP, entityCounter, *xhPP);
		}
	}
	return SUCCESS;
}

// =====================================================================
// Initialize the Jollybean stem.
// =====================================================================
Error xJollybeanIniS() {
	// Histogram gene types
	U32 *histoP = NULL;
	Error e = histoNew(&histoP, N_SYS_TYPES);
	if (!e)
		e = _histoGeneTypes(histoP, &grp1);
	
	// Add subsystems as components to Jollybean stem.
	sAddC(&sJollybean, sControl.id, &sControl.xHeader); // Add control sys no matter what
	for (U32 i = 0; !e && i < sizeof(sPA) / sizeof(sPA[0]); i++) 
		if (sPA[i])
			e = sAddC(&sJollybean, sPA[i]->id, &sPA[i]->xHeader);

	// Initialize Jollybean's component subsystems before we throw genes in.
	if (!e) {
		U32 *hP, *hEndP;
		arrayIniPtrs(histoP, (void**) &hP, (void**) &hEndP, -1);
		for (Entity sID = 0; !e && hP < hEndP; hP++, sID++) {
			if (*hP)  {
				System *sP = (System*) sGetC(&sJollybean, sID);
				if (sP)
					e = sIni(sP, *hP);  // makes subsystem's EC map and activities
			}
		}
	}
	
	// Distribute the genes to the proper stems.
	if (!e)
		e = _distributeGenes(&sJollybean, &grp1);
	// Clean up.
	histoDel(&histoP);
	
	return e;
}

// =====================================================================
// Clear all subsystems and Jollybean stem.
// =====================================================================
void sClrAll() {
	sClr(&sRender);  // EXAMPLE CODE! You're gonna have to loop through a mapped array of stems here. 
}

// ====================================================================================
// Placeholder for component-initialization; this has to be handled in xJollbeanIniS().
// ====================================================================================
Error xJollybeanIniC(XHeader *xhP) {
	UNUSED_(xhP);
	return SUCCESS;
}

// msgP may contain information the callback finds pertinent; e.g. "Who collided w/ me?"
static Error _trigger(Message *msgP) {
	assert(msgP->toID >= nEntities && msgP->toID >= N_SYS_TYPES);  // highest system will have entity ID <= N_SYS_TYPES
	Map *mapP = entityReactionMA[msgP->toID];  // toID is an entity
	Reaction *requestedReactionP = (Reaction*) mapGet(mapP, msgP->event);
	assert(requestedReactionP);
	Reaction *ongoingReactionP = (Reaction*) sGetC(&sJollybean, msgP->toID);
	assert(ongoingReactionP);
	// If entity is busy, see if we're allowed to interrupt it.
	Activity *currActivityP = sGetActivityFromE(&sJollybean, msgP->toID);
	assert(currActivityP);
	if (currActivityP->id == JB_RUN_CHILDREN && sComponentIsActive(&sJollybean, msgP->toID)) {
		if (requestedReactionP->priority < ongoingReactionP->priority)
			return SUCCESS;
	}
	// Entity is either idle or doing something less important. Give it something better to do.
	*ongoingReactionP = *requestedReactionP;
	ongoingReactionP->msg = *msgP;
	return sStartCActivity(&sJollybean, msgP->toID, JB_REACT);
}

static Error _triggerGroup(Message *msgP) {
	assert(msgP);
	Error e = SUCCESS;
	Entity *eA = (Entity*) mapGet(subscriberAMP, msgP->event);
	assert(eA);
	Entity *eP, *eEndP;
	arrayIniPtrs((void*) eA, (void**) &eP, (void**) &eEndP, -1);
	for (; !e && eP < eEndP; eP++)
		e = _trigger(msgP);
	return e;
}

Error xJollybeanReact(Activity *aP) {
	assert(aP);
	XJollybeanC *cP, *cEndP;
	Error e = SUCCESS;
	// Check active subsystems' outboxes. Their callbacks populate systems and JB's reaction activity.
	Activity *rcaP = sGetActivity(&sJollybean, JB_RUN_CHILDREN);
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
			sDeactivateC(&sJollybean, cP->item.r.msg.toID);

	return SUCCESS;
}

// ====================================================================================
// This is THE game engine loop.
// ====================================================================================
Error xJollybeanRunChildren(Activity *aP) {
	assert(aP);
	XJollybeanC *cP, *cEndP;
	Error e = SUCCESS;
	arrayIniPtrs(aP->cA, (void**) &cP, (void**) &cEndP, aP->firstInactiveIdx);
	while (!e) {
		for (cP = aP->cA; cP < cEndP; cP++)
			sRun(&cP->item.s);
	}
	return e;
}

// ====================================================================================
// Main 
// ====================================================================================
int main() {
	Error e = sIni(&sJollybean, N_SYS_TYPES);
	if (!e)
		sRun(&sJollybean);

	System *sRenderP = NULL;
	if (!e)
		sRenderP = sGetC(&sJollybean, RENDER);
	// Test deactivating and activating on all components 
	for (U8 i = 8; !e && i <= 10; i++) {
		sDeactivateC(sRenderP, i);
		sRun(&sJollybean);
		e = sActivateC(sRenderP, i);
		if (!e) 
			sRun(&sJollybean);
	}
	
	sClrAll();
	return e;
}
