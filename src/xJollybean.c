#include "xJollybean.h"

/* IMMEDIATE GOALS
 * ================
 X-split genes
 X-make genomes
 X-histogram them
 X-make macro to ease system creation
 X-use histogram results to allocate the right amount of memory into xRender's activities
 * load the right one into xRender
 * have xRender automatically initialize it for you
 * call sysRun() to make sure xRender() doesn't draw anything, only blanks the screen
 * send xRender a message to activate E's component
 * call xRender() again and see if your image draws
 * wait 2 seconds
 * send xRender a message to deactivate E's component
 * call xRender() again to see if your image disappears
 * */

NEW_GENOME_(bigger, &biggerXRenderC.xHeader);
GENOME_GROUP_(grp1, &bigger, &bigger, &bigger);

NEW_SYS_(Jollybean, JOLLYBEAN, ACTIVITY_(xJollybeanRun));

// Register subsystems here.
static System *sysPA[N_SYS_TYPES] = {
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
			sP = (System*) sysGetComponent(&sJollybean, (*xhPP)->type);  // We don't set the owner of the gene pool.
			if (sP)
				e = sysAddComponent(sP, entityCounter, *xhPP);
		}
	}
	return SUCCESS;
}

// =====================================================================
// Initialize the Jollybean system.
// =====================================================================
Error xJollybeanIniS() {
	// Histogram gene types
	U32 *histoP = NULL;
	Error e = histoNew(&histoP, N_SYS_TYPES);
	if (!e)
		e = histoGeneTypes(histoP, &grp1);
	
	// Add subsystems as components to Jollybean system.
	for (U32 i = 0; !e && i < sizeof(sysPA) / sizeof(sysPA[0]); i++) 
		if (sysPA[i])
			e = sysAddComponent(&sJollybean, sysPA[i]->id, &sysPA[i]->xHeader);

	// Initialize Jollybean's component subsystems before we throw genes in.
	if (!e) {
		U32 *hP, *hEndP;
		arrayIniPtrs(histoP, (void**) &hP, (void**) &hEndP, -1);
		for (Entity sysID = 0; !e && hP < hEndP; hP++, sysID++) {
			if (*hP)  {
				System *sP = (System*) sysGetComponent(&sJollybean, sysID);
				if (sP)
					e = sysIni(sP, *hP);  // makes subsystem's EC map and activities
			}
		}
	}

	// Distribute the genes to the proper systems.
	if (!e)
		e = _distributeGenes(&sJollybean, &grp1);

	// TODO: replace below if-block with cohesive function
	// How do we determine what's active? 
	if (!e) {
		sJollybean.activityA[0].active = 1;
		sJollybean.activityA[0].firstInactiveIdx++;
	}
	
	// Clean up.
	histoDel(&histoP);
	
	return e;
}

// =====================================================================
// Clear all subsystems and Jollybean system.
// =====================================================================
void sysClrAll() {
	sysClr(&sRender);  // EXAMPLE CODE! You're gonna have to loop through a mapped array of systems here. 
}

// ====================================================================================
// Placeholder for component-initialization; this has to be handled in xJollbeanIniS().
// ====================================================================================
//typedef Error (*SysIniCFP)(XHeader *xhP);
Error xJollybeanIniC(XHeader *xhP) {
	UNUSED_(xhP);
	return SUCCESS;
}

// ====================================================================================
// This is THE game engine loop.
// ====================================================================================
Error xJollybeanRun(Activity *aP) {
	System *sP, *sEndP;
	arrayIniPtrs(aP->ecA, (void**) &sP, (void**) &sEndP, aP->firstInactiveIdx);
	for (; sP < sEndP; sP++)
		sysRun(sP);

	return SUCCESS;
}

// ====================================================================================
// Main 
// ====================================================================================
void displayComponents(System *jollybeanSysP, Entity entity) {
	System *sP =  (System*) sysGetComponent(jollybeanSysP, entity);
	if (sP) {
		printf("system %d's active components in its first activity:\n", sP->xHeader.owner);
		for (U32 i = 0, e = SUCCESS; !e && i < sP->activityA[0].firstInactiveIdx; i++) {
			XRenderC *cP = (XRenderC*) arrayGetVoidElemPtr(sP->activityA[0].ecA, i);
			printf("\tEntity %d: 0x%08x\n", cP->xHeader.owner, (U32) cP);
		}
	}
}
#include <unistd.h>
int main() {
	Error e = sysIni(&sJollybean, N_SYS_TYPES);
	if (!e)
		sysRun(&sJollybean);
	displayComponents(&sJollybean, RENDER);
	sleep(1);

	if (!e) {
		sysDeactivateCmp((System*) sJollybean.activityA[0].ecA, 9);
		displayComponents(&sJollybean, RENDER);
	}

	if (!e)
		sysRun(&sJollybean);
	sleep(1);
	
	sysClrAll();
	return e;
}
