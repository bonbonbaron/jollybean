#include "jb.h"
//***************************************************
//** System systems *********************************
//***************************************************
static Error sRunSystems(Activity *aP);
static Error sRunTasks(Activity *aP);
//static Map **_entityReactionMA;
static Map *_subscriberAMP;
static Map *_sharedMP = NULL;         // map of maps of shared "components"
static System **_sPA = NULL;  // an array of pointers to all the children systems

System_(System, 0, 
	Activity_(REACT, sRunSystems),
	Activity_(TICK, sRunTasks),
);

typedef struct {
	U8 size;
	U8 count; 
	U8 type;
	U8 geneClass;
} XHistoElem;

// =====================================================================
// Loop through each genome's genes and histogram their component types.
// =====================================================================
static Error _histoGeneTypes(XHistoElem *metaA, Biome *biomeP) {
	if (!metaA || !biomeP)
		return E_BAD_ARGS;

	Seed **seedPP = biomeP->seedPA;
	Seed **seedEndPP = seedPP + biomeP->nEntities;   // pointer to the end of the above array
	Genome *genomeP;
	GeneHeader **ghPP, **ghEndPP;   // pointers to an array of gene header pointers and its end
	GeneHeader gh;  // faster access to double pointer'd gene header
	// Loop through genomes
	for (; seedPP < seedEndPP; seedPP++) {
		genomeP = (*seedPP)->genomeP;
		ghPP = &genomeP->genePA[0];
		ghEndPP = ghPP + genomeP->nGenes;
		// Histo genome's genes that aren't blackboard items.
		for (; ghPP < ghEndPP; ghPP++) {
			gh = **ghPP;
			if (gh.geneClass != BLACKBOARD) {
				metaA[gh.type].count++;
				metaA[gh.type].size = gh.size;
				metaA[gh.type].geneClass = gh.geneClass;
			}
		}
	}
	return SUCCESS;
}

// =====================================================================
// Distribute all genes to their appropriate subsystems.
// =====================================================================
static Error _distributeGenes(System *sSystemP, Biome *biomeP) {
	Seed **seedPP = biomeP->seedPA;
	Seed **seedEndPP = seedPP + biomeP->nEntities;   // pointer to the end of the above array
	Genome *genomeP;
	GeneHeader **ghPP, **ghEndPP;   // pointers to an array of gene header pointers and its end
	GeneHeader gh;  // faster access to double pointer'd gene header
	XHeader xh = {0};
	U8 componentType;
	Error e = SUCCESS;
	// Loop through seeds
	for (Entity entityCounter = N_COMPONENT_TYPES; !e && seedPP < seedEndPP; seedPP++, entityCounter++) {
		genomeP = (*seedPP)->genomeP;
		ghPP = (*seedPP)->genomeP->genePA;
		ghEndPP = ghPP + genomeP->nGenes;
		// Loop through current genome's genes
		for (System *sP = NULL; !e && ghPP < ghEndPP; ghPP++) {  // ghPP is a pointer to a pointer to a global singleton of a component
			gh = **ghPP;
			switch (gh.geneClass) {
				case ECS_COMPONENT:
					componentType = (*ghPP)->type;
					sP = (System*) sGetC(sSystemP, componentType);  // We don't set the owner of the gene pool.
					xh.type = componentType;
					e = sAddC(sP, entityCounter, &xh);
					break;
				case ECS_SHARED:
					e = mapSet(_sharedMP, gh.type, (const void*) *ghPP);  //*ghPP expands out to the size of the gene.
					break;
				case BLACKBOARD:

					break;
				default:
					break;
			}
		}
	}
	return e;
}

// =====================================================================
// Initialize the System system.
// =====================================================================
Error xSystemIniS(void *sParamsP) {
	XHistoElem xHistoA[N_COMPONENT_TYPES] = {0};
	XHistoElem *xheP, *xheEndP;
	SSystemIniSParams *sSystemIniSPrmsP = (SSystemIniSParams*) sParamsP;
	U8 nSharedMaps = 0;

	if (!sParamsP)
		return E_BAD_ARGS;

	Error e = _histoGeneTypes(&xHistoA[0], sSystemIniSPrmsP->biomeP);
	
	/****************************************
	************* ECS Components  ***********
	****************************************/
	// Add subsystems as components to System system.
	for (U32 i = 0; !e && i < sSystemIniSPrmsP->nSystems; i++) 
		if (_sPA[i])
			e = sAddC(&sSystem, _sPA[i]->id, &_sPA[i]->xHeader);

	// Initialize subsystems before we throw genes in.
	if (!e) {
		xheP = &xHistoA[0];
		xheEndP = xHistoA + N_COMPONENT_TYPES;
		for (Entity sID = 0; !e && xheP < xheEndP; xheP++, sID++) 
			if (xheP->geneClass == ECS_COMPONENT && xheP->count)  {  // i.e. if any entities exist having components for this system...
				System *sP = (System*) sGetC(&sSystem, sID);
				if (sP)
					e = sIni(sP, xheP->count, sP->sIniSParamsP);  // makes subsystem's component directory and activities
			}
	}

	/******************************************
	************* Shared Components ***********
	******************************************/
	// Count the number of TYPES of shared items. 
	if (!e) {
		xheP = &xHistoA[0];
		xheEndP = xHistoA + N_COMPONENT_TYPES;
		for (; !e && xheP < xheEndP; xheP++)
			if (xheP->geneClass == ECS_SHARED && xheP->count)
				++nSharedMaps;
	}

	// Allocate map of maps.
	if (!e && nSharedMaps)
		e = mapNew(&_sharedMP, sizeof(Map), nSharedMaps);

	// Make a map of maps, one for each type (one map of positions, another of texts, etc.)
	// If you needed to make a shared map, then fill it. 
	if (!e && _sharedMP) {
		xheP = &xHistoA[0];
		xheEndP = xHistoA + N_COMPONENT_TYPES;
		Map *mapP = NULL;
		for (; !e && xheP < xheEndP; xheP++)
			if (xheP->geneClass == ECS_SHARED && xheP->count) {
				e = mapNew(&mapP, xheP->size, xheP->count);
				if (!e)
					e = mapSet(_sharedMP, xheP->type, (void*) mapP);
			}
	}

	/***************************************
	************* Behavior Trees ***********
	***************************************/
	if (!e) {
		Seed **seedPP = sSystemIniSPrmsP->biomeP->seedPA;
		Seed **seedEndPP = seedPP + sSystemIniSPrmsP->biomeP->nEntities;
		Genome *genomeP;
		GeneHeader **ghPP, **ghEndPP;   // pointers to an array of gene header pointers and its end
		GeneHeader gh;  // faster access to double pointer'd gene header
		Error e = SUCCESS;
		// Loop through seeds
		for (; !e && seedPP < seedEndPP; seedPP++) {
			genomeP = (*seedPP)->genomeP;
			ghPP = (*seedPP)->genomeP->genePA;
			ghEndPP = ghPP + genomeP->nGenes;
			// Loop through current genome's genes
			for (System *sP = NULL; !e && ghPP < ghEndPP; ghPP++) {  // ghPP is a pointer to a pointer to a global singleton of a component

	/************************************
	************* Blackboards ***********
	************************************/
	//TODO btNew() all tree singletons
	//TODO allocate array of blackboards with # elems = # entities
	

	// ===========================================
	// Distribute the genes to the proper systems.
	// ===========================================
	if (!e)
		e = _distributeGenes(&sSystem, sSystemIniSPrmsP->biomeP);

	return e;
}

// ====================================================================================
// Placeholder for component-initialization; this has to be handled in xSystemIniS().
// ====================================================================================
Error xSystemIniC(XHeader *xhP) {
	unused_(xhP);
	return SUCCESS;
}

// msgP may contain information the triggered callback finds pertinent; e.g. "Who collided w/ me?"
//TODO: make a way to stop all active components of an entity, and no more than that, in order to efficiently switch between an entity's reactions
static Error _trigger(System *sSystemP, Message *msgP) {
	U32 nComponents = sGetNComponents(sSystemP); // TODO: get rid of this in release version
	// Requested reaction
	assert(msgP->to >= nComponents && msgP->to >= N_COMPONENT_TYPES);  // highest system will have entity ID <= N_COMPONENT_TYPES
	//Map *mapP = _entityReactionMA[msgP->to];  // toID is an entity
	// The callbacks will be called in the REACT activity.
	return sStartCActivity(sSystemP, msgP->to, REACT);
}

static Error _triggerGroup(System *sSystemP, Message *msgP) {
	assert(msgP);
	Error e = SUCCESS;
	Entity *eA = (Entity*) mapGet(_subscriberAMP, msgP->topic);
	assert(eA);
	Entity *eP, *eEndP;
	arrayIniPtrs((void*) eA, (void**) &eP, (void**) &eEndP, -1);
	for (; !e && eP < eEndP; eP++) {
    msgP->to = *eP;
		e = _trigger(sSystemP, msgP);
  }
	return e;
}

inline static U8 _isHigherPriority(U8 newPriority, U8 existingPriority) {
	return newPriority > existingPriority;
}

static Error _readOutboxes(Activity *aP) {
	assert(aP);
	XSystemC *cP, *cEndP;
	Error e = SUCCESS;
	// Check active subsystems' outboxes. Their callbacks populate systems and JB's reaction activity.
	arrayIniPtrs(aP->cA, (void**) &cP, (void**) &cEndP, aP->firstInactiveIdx);
	for (; !e && cP < cEndP; cP++) 
		// Read outboxes
		for (Message *msgP = cP->outboxP->msgA, *msgEndP = cP->outboxP->msgA + cP->outboxP->nMsgs;
				 !e && msgP < msgEndP;
				 msgP++) 
			continue;
			//if (msgP->to) 

	return e;
}

static Error sRunTasks(Activity *aP) {
	assert(aP);
	XSystemC *cP, *cEndP;
	Error e = SUCCESS;
	// Check active subsystems' outboxes. Their callbacks populate systems and JB's reaction activity.
	arrayIniPtrs(aP->cA, (void**) &cP, (void**) &cEndP, aP->firstInactiveIdx);
	for (; !e && cP < cEndP; cP++) 
		// Read outboxes
		for (Message *msgP = cP->outboxP->msgA, *msgEndP = cP->outboxP->msgA + cP->outboxP->nMsgs;
				 !e && msgP < msgEndP;
				 msgP++) 
			if (msgP->to) 
				e = _trigger(aP->ownerP, msgP);
			else
				e = _triggerGroup(aP->ownerP, msgP);
	
	return SUCCESS;
}

// This runs System's subsystems.
static Error sRunSystems(Activity *aP) {
	assert(aP);
	XSystemC *cP, *cEndP;
	Error e = SUCCESS;
	arrayIniPtrs(aP->cA, (void**) &cP, (void**) &cEndP, aP->firstInactiveIdx);
	for (cP = aP->cA; !e && cP < cEndP; cP++) {
		e = sRun(cP);
		// Put idle systems to sleep.
		if (!e && cP->firstInactiveActIdx == 0)
			e = sDeactivateC(&sSystem, cP->id);
	}
	return e;
}

/* xIni() initializes the parent system as well as its children. */
Error xIni(System **sPA, U16 nSystems, Biome *biomeP) {
	// sIni() takes no arguments, yet the parent system needs to know some outside factors.
	// So we use static variables here. 
	if (!sPA || nSystems < 1 || !biomeP)
		return E_BAD_ARGS;

	_sPA = sPA;

	SSystemIniSParams sSystemIniSPrms = {
		.nSystems = nSystems,
		.biomeP = biomeP,
		.sysPA = sPA
	};

	return sIni(&sSystem, N_COMPONENT_TYPES, (void*) &sSystemIniSPrms);
	// TODO: kick off the parent system activities here
}


/*
 * Okay! We're nearing completion of the core pyramid. As far as I can see (and I'm sure that won't hold for long), there are three (not so) major things left:
 *
 * 1) Biome setup									<-- gonna try to finish tonight
 * 2) X-BT interface							<-- hopefully tomorrow (Sunday) night (depends on #1)
 * 3) Checks add-on to sRun()			<-- also hopefully tomorrow (Sunday) night 
 * 
 * Then next weekend, I'll be able to start knocking out the systems I've been talking for a long time about: animation, motion, collision, rendering, timing, etc. 
 *
 * Anyway, without further ado, let's approach it:
 *
 * 1> Game is designed like this: 
		  ============================
			game
			└── biome
			    └── seed
			        ├── material
			        │   ├── blackboard
			        │   ├── genome
			        │   └── transient
			        └── personality
			            └── quirk
			   							└── response
			   									└── node
			
 *
 * So Goomba #1 should be in (3, 5), #2 in (200, 10), etc. Also, #1 should say, "Hello," while #2 says "Go fuck yourself." So they have to be grouped together. Given that I only allow myself to use arrays or maps as collections in this game, the choice is easy. 
 *
 * When you unpack a transient, it'll be composed of an array of enum'd type-object pairs. Currently, that's how components work: They get distributed to systems by xHeader.type. 
 *
 * But here's what I want to know: a position is a transient, but it isn't being shared necessarily because it's transient. Here's a list of transients, so we can see whether they're all shared:
 *
 * Position	 <-- this would go in a shared space
 * Text      <-- this would go in a blackboard, not shared space
 *
 * So a third field, destinationType, is an enum of DEST_BLACKBOARD and DEST_SHARED_SPACE.
 *	 
 * 
 */

// Genome is now going to house all three types. I'll expand the genome header structure so it'll know how to react to each of the three types. 

// Could I possibly wrap genome up in this as well, so as to zip through all of them at once? Right now I'm using xHeader; how easily replacable is that? It's necessary to do so, because all materials must be associated with the same entity. 
