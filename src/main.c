#include "xRender.h"
#include "SDL.h"
#include <unistd.h>
#include "genes.h"
#include "ecs.h"

/* IMMEDIATE GOALS
 * ================
 X-split genes
 X-make genomes
 X-histogram them
 * use histogram results to allocate the right amount of memory into xRender's activities
 * try to load the right one into xRender
 * have xRender automatically initialize it for you
 * make sure xRender() doesn't draw anything, only blanks the screen
 * send xRender a message to activate E's component
 * call xRender() again and see if your image draws
 * wait 2 seconds
 * send xRender a message to deactivate E's component
 * call xRender() again to see if your image disappears
 * */


NEW_GENOME_(bigger, &biggerXRenderC.xHeader);
GENOME_GROUP_(grp1, &bigger, &bigger, &bigger );

Error histoGenomes(U32 *histoA, GenomeGrp *ggP) {
	Genome **gPP = &ggP->genomePA[0];
	Genome **gEndPP = gPP + ggP->nEntities;
	XHeader **xhPP, **xhEndPP;
	for (; gPP < gEndPP; gPP++) {
		xhPP = &(*gPP)->genePA[0];
		xhEndPP = xhPP + (*gPP)->nGenes;
		for (; xhPP < xhEndPP; xhPP++)
			histoA[(*xhPP)->type]++;
	}
	return SUCCESS;
}

Error sysIniAll(GenomeGrp *ggP) {
	U32 *histoP = NULL;
	Error e = histoNew(&histoP, N_SYS_TYPES);

	if (!e)
		e = histoGenomes(histoP, ggP);

	if (!e) {
		//TODO loop through systems and histo and allocate their maps and activities
		Entity entityCounter = 0;
		//TODO loop through genomes' genes, add them to the maps w/ the right E, and 
		// This is old code you can borrow:
		if (!e)
			e = sysIni(&sRender, 1);
	}
		

	histoDel(&histoP);
	
	return e;
}

void sysClrAll() {
	sysClr(&sRender);  // you're gonna have to loop through a mapped array of systems here. 
}

int main() {
	Error e = sysIniAll(&grp1);
	if (!e)
		sysClrAll();
	return e;
}
