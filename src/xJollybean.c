#include "xJollybean.h"

NEW_GENOME_(bigger, &biggerXRenderC.xHeader);
GENOME_GROUP_(grp1, &bigger, &bigger, &bigger);

NEW_SYS_(Jollybean, JOLLYBEAN, 
		ACTIVITY_(JB_RUN, xJollybeanRun),
		ACTIVITY_(JB_BEHAVE, xJollybeanBehave)
);

Map *behaviorMA;
Map *SubscriberAM;

static System *sPA[N_SYS_TYPES] = {
	&sRender, 
	NULL, 
	NULL, 
	NULL, 
	NULL, 
	NULL, 
	NULL
}; 

// =====================================================================
// Loop through each genome's genes and histogram their component types.
// =====================================================================
Error histoGeneTypes(U32 *histoA, GenomeGrp *ggP) {
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
		e = histoGeneTypes(histoP, &grp1);
	
	// Add subsystems as components to Jollybean stem.
	sAddC(&sJollybean, sControl.id, &sControl.xHeader); // Add control sys no matter what
	for (U32 i = 0; !e && i < sizeof(sPA) / sizeof(sPA[0]); i++) 
		if (sPA[i])
			e = sAddC(&sJollybean, sPA[i]->id, &sPA[i]->xHeader);

	// Initialize Jollybean's component subsystems before we throw genes in.
	if (!e) {
		U32 *hP, *hEndP;
		arrayIniPtrs(histoP, (void**) &hP, (void**) &hEndP, -1);
		for (Entity sID = 1; !e && hP < hEndP; hP++, sID++) {
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
	if (!e) 
		sJollybean.activityA[0].firstInactiveIdx++;  // TODO: replace below if-block with cohesive function
	
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
//typedef Error (*SysIniCFP)(XHeader *xhP);
Error xJollybeanIniC(XHeader *xhP) {
	UNUSED_(xhP);
	return SUCCESS;
}

Error xJollybeanBehave(Activity *aP) {
	Behavior *bP, *bEndP;
	arrayIniPtrs(aP->cA, (void**) &bP, (void**) &bEndP, aP->firstInactiveIdx);
	Error e = SUCCESS;
	for (; !e && bP < bEndP; bP++)
		e = (*bP->callback)(bP->entity);

	return SUCCESS;
}

// ====================================================================================
// This is THE game engine loop.
// ====================================================================================
Error xJollybeanRun(Activity *aP) {
	System *sP, *sEndP;
	arrayIniPtrs(aP->cA, (void**) &sP, (void**) &sEndP, aP->firstInactiveIdx);
	for (; sP < sEndP; sP++)
		sRun(sP);

	return SUCCESS;
}

void displayComponents(System *jollybeanSysP, Entity entity) {
	System *sP =  (System*) sGetC(jollybeanSysP, entity);
	if (sP) {
		printf("stem %d's active components in its first activity:\n", sP->xHeader.owner);
		XRenderC *cP = sP->activityA[0].cA;
		for (U32 i = 0, e = SUCCESS; !e && i < sP->activityA[0].firstInactiveIdx; i++, cP++) {
			printf("\tEntity %d: 0x%08x\n", cP->xHeader.owner, (U32) cP);
		}
	}
}

// ====================================================================================
// Main 
// ====================================================================================
#include <unistd.h>
int main() {
	Error e = sIni(&sJollybean, N_SYS_TYPES);
	if (!e)
		sRun(&sJollybean);
	sleep(1);

	System *sRenderP = NULL;
	if (!e)
		sRenderP = sGetC(&sJollybean, RENDER);
	// Test deactivating and activating on all components 
	for (U8 i = 8; !e && i <= 10; i++) {
		//displayComponents(&sJollybean, RENDER);
		sDeactivateC(sRenderP, i);
		//displayComponents(&sJollybean, RENDER);
		sRun(&sJollybean);
		sleep(1);
		e = sActivateC(sRenderP, i);
		//displayComponents(&sJollybean, RENDER);
		if (!e) {
			sRun(&sJollybean);
			sleep(1);
		}
	}
	
	sClrAll();
	return e;
}
