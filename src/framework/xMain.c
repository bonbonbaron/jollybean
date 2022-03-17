#include "xMain.h"
//***************************************************
//** System systems *********************************
//***************************************************
static Error xMainRunSystems(Focus *fP);
//static Error xMainRunTasks(Focus *fP);
//static Map **_entityReactionMA;
X_(Main, 1, Focus_(0, xMainRunSystems));

typedef struct {
	Key size;
	Key count; 
	Key type;
	Key geneClass;
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
static Error _distributeGenes(XMain *xMainSysP, Key nSystemsMax) {

	Seed **seedPP = xMainSysP->biomeP->seedPA;
	Seed **seedEndPP = seedPP + xMainSysP->biomeP->nEntities;   // pointer to the end of the above array
	Genome *genomeP;
	GeneHeader **ghPP, **ghEndPP;   // pointers to an array of gene header pointers and its end
	GeneHeader gh;  // faster access to double pointer'd gene header
	XHeader xh = {0};
	Key componentType;
	System *sP = NULL;
	Error e = SUCCESS;
	
	// Loop through seeds
	for (Entity entityCounter = nSystemsMax; !e && seedPP < seedEndPP; seedPP++, entityCounter++) {
		genomeP = (*seedPP)->genomeP;
		ghPP = (*seedPP)->genomeP->genePA;
		ghEndPP = ghPP + genomeP->nGenes;
		// Loop through current genome's genes
		for (; !e && ghPP < ghEndPP; ghPP++) {  // ghPP is a pointer to a pointer to a global singleton of a component
			gh = **ghPP;
			switch (gh.geneClass) {
				case ECS_COMPONENT:
					componentType = (*ghPP)->type;
					sP = (System*) xGetComp(sP, componentType);  // We don't set the owner of the gene pool.
					if (sP) {
						xh.type = componentType;
						e = xAddComp(sP, entityCounter, &xh);
					}
					break;
				case ECS_SHARED:
					e = mapSet(xMainSysP->sharedMP, gh.type, (const void*) *ghPP);  //*ghPP expands out to the size of the gene.
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
Error xMainIniSys(System *sP, void *sParamsP) {
	Key nSharedMaps = 0;
	XHistoElem *xheP, *xheEndP;
	XMain *xMainSysP = (XMain*) sP;

	if (!sParamsP)
		return E_BAD_ARGS;

	XMainIniSysPrms *xMainIniSysPrmsP = (XMainIniSysPrms*) sParamsP;

	if (!xMainIniSysPrmsP->biomeP || !xMainIniSysPrmsP->sysPA)
		return E_BAD_ARGS;
	

	XHistoElem xHistoA[xMainIniSysPrmsP->nSystemsMax];
	memset((void*) xHistoA, 0, xMainIniSysPrmsP->nSystemsMax * sizeof(XHistoElem));
	xMainSysP->biomeP = xMainIniSysPrmsP->biomeP;
	Error e = _histoGeneTypes(&xHistoA[0], xMainSysP->biomeP);
	
	/****************************************
	************* ECS Components  ***********
	****************************************/
	// Add subsystems as components to Main system.
	if (xMainIniSysPrmsP)
		for (U32 i = 0; !e && i < xMainIniSysPrmsP->nSystems; i++) 
			if (xMainIniSysPrmsP->sysPA[i])
				e = xAddComp(sP, xMainIniSysPrmsP->sysPA[i]->id, &xMainIniSysPrmsP->sysPA[i]->xHeader);

	// Initialize subsystems before we throw genes in.
	if (!e) {
		xheP = &xHistoA[0];
		xheEndP = xHistoA + xMainIniSysPrmsP->nSystemsMax;
		for (Entity sID = 0; !e && xheP < xheEndP; xheP++, sID++) 
			if (xheP->geneClass == ECS_COMPONENT && xheP->count)  {  // i.e. if any entities exist having components for this system...
				System *_sP = (System*) xGetComp(sP, sID);
				if (_sP)
					e = xIniSys(_sP, xheP->count, _sP->sIniSParamsP);  // makes subsystem's component directory and activities
			}
	}

	/******************************************
	************* Shared Components ***********
	******************************************/
	// Count the number of TYPES of shared items. 
	if (!e) {
		xheP = &xHistoA[0];
		xheEndP = xHistoA + xMainIniSysPrmsP->nSystemsMax;
		for (; !e && xheP < xheEndP; xheP++)
			if (xheP->geneClass == ECS_SHARED && xheP->count)
				++nSharedMaps;
	}

	// Allocate map of maps.
	if (!e && nSharedMaps)
		e = mapNew(&xMainSysP->sharedMP, sizeof(Map), nSharedMaps);

	// Make a map of maps, one for each type (one map of positions, another of texts, etc.)
	// If you needed to make a shared map, then fill it. 
	if (!e && xMainSysP->sharedMP) {
		xheP = &xHistoA[0];
		xheEndP = xHistoA + xMainIniSysPrmsP->nSystemsMax;
		Map *mapP = NULL;
		for (; !e && xheP < xheEndP; xheP++)
			if (xheP->geneClass == ECS_SHARED && xheP->count) {
				e = mapNew(&mapP, xheP->size, xheP->count);
				if (!e)
					e = mapSet(xMainSysP->sharedMP, xheP->type, (void*) mapP);
			}
	}

	/***************************************
	************* Behavior Trees ***********
	***************************************/
#if 0
	if (!e) {
		Seed **seedPP = xMainIniSysPrmsP->biomeP->seedPA;
		Seed **seedEndPP = seedPP + xMainIniSysPrmsP->biomeP->nEntities;
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
				unused_(sP);
				unused_();
				continue;  // TODO
			}
		}
	}
#endif

	/************************************
	************* Blackboards ***********
	************************************/
	//TODO btNew() all tree singletons
	//TODO allocate array of blackboards with # elems = # entities
	

	// ===========================================
	// Distribute the genes to the proper systems.
	// ===========================================
	if (!e) 
		e = _distributeGenes(xMainSysP, xMainIniSysPrmsP->nSystemsMax);

	return e;
}

// ====================================================================================
// Placeholder for component-initialization; this has to be handled in xSystemIniS().
// ====================================================================================
Error xMainIniComp(System *sP, XHeader *xhP) {
	unused_(xhP);
	unused_(sP);
	return SUCCESS;
}

Error xMainProcessMessage(System *sP, Message *msgP) {
	unused_(sP);
	unused_(msgP);
	return SUCCESS;
}

// This runs System's subsystems.
static Error xMainRunSystems(Focus *fP) {
	assert(fP);
	XMainComp *cP, *cEndP;
	Error e = SUCCESS;
	arrayIniPtrs(fP->compA, (void**) &cP, (void**) &cEndP, fP->firstInactiveIdx);
	for (cP = fP->compA; !e && cP < cEndP; cP++) {
		e = xRun((System*) cP);
		// Put idle systems to sleep.
		if (!e && cP->firstInactiveActIdx == 0)
			e = xDeactivateComp(fP->ownerP, cP->id);
	}
	return e;
}

/* xIni() initializes the parent system as well as its children. */
Error xMainIni(XMain **xMainSysPP, System **sPA, U16 nSystems, Key nSystemsMax, Biome *biomeP) {
	if (!sPA || nSystems < 1 || !biomeP)
		return E_BAD_ARGS;

	XMainIniSysPrms xMainSysIniPrms = {
		.nSystemsMax = nSystemsMax,
		.nSystems = nSystems,
		.biomeP = biomeP,
		.sysPA = sPA
  };

	Error e = jbAlloc((void**) xMainSysPP, sizeof(XMain), 1);

	if (!e)
		e = xIniSys(&(*xMainSysPP)->system, nSystems, (void*) &xMainSysIniPrms);

	return e;
	// TODO: kick off the parent system activities here
}

