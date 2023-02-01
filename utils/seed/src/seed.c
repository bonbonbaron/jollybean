#include "seed.h"

// Gene
Error geneNew(char *geneName, char *geneClass, char *type, U32 size, Key mutationKey, char *srcDataName) {
  // validate args
  if (!geneName || !geneClass || !type || !size) {
    printf("Gene name, class, type, or size is zero.\n");
    return E_BAD_ARGS;
  }

  if (strcmp(geneClass, "EXCLUSIVE_GENE") && 
      strcmp(geneClass, "SHARED_GENE") && 
      strcmp(geneClass, "BB_GENE")) {
    printf("Gene type has to be one of the 3 accepted types (exclusive, shared, or blackboard).\n");
    return E_BAD_ARGS;
  }

  FILE *fP = getBuildFile("Seed/Genome/Gene/src", geneName, ".c");
  if (!fP) {
    return E_FILE_IO;
  }
  // TODO write src file
  // TODO write include file with extern
  

}
Error geneDel(char *geneName);
// Genome
Error genomeNew(Genome **genomePP);
Error genomeAddGene(Genome *genomeP, Gene *geneP);
Error genomeRemGene(Genome *genomeP, U32 geneIdx);
Error genomeEditGene(Genome *genomeP, Gene *newGeneP, U32 oldGeneIdx);
Error genomeDel(Genome **genomePP);
// Quirk
Error quirkNew(Quirk **quirkPP, Key trigger, void *actionP);
Error quirkDel(Quirk **quirkPP);
// Personality
Error personalityNew(Personality **personatityPP);
Error personalityAddQuirk(Personality *personalityP, Quirk *quirkP);
Error personalityRemQuirk(Personality *personalityP, U32 quirkIdx);
Error personalityEditQuirk(Personality *personalityP, Quirk *newQuirkP, U32 oldQuirkIdx);
Error personalityDel(Personality **personalityPP);
// Seed
Error seedNew(Seed **seedPP);
Error seedAddGenome(Seed *seedP, Genome *genomeP);
Error seedAddPersonality(Seed *seedP, Personality *personalityP);
Error seedRemGenome(Seed *seedP);  // don't need index as there's only one genome per seed
Error seedRemPersonality(Seed *seedP);  // don't need index as there's only one personality per seed
Error seedEditGenome(Seed *seedP, Genome *newGenomeP);  // don't need index as there's only one genome per seed
Error seedEditPersonality(Seed *seedP, Personality *newPersonalityP);  // don't need index as there's only one personality per seed
Error seedDel(Seed **seedPP);

/*
 * I'm not 100% clear on the use case for this yet.\
 * EXAMPLE: Body.
 *  Body is a convenient wrapper gene for several others like animation, image, and collisio box.
 *  Body's wanna straight-up create a destination seed for its contents. Too much trouble to scrabble them all together  manually, and why pretend to not know they belong together?
 *  USE CASE 1: automatic seed generation
 *  =====================================
 *    make image into a set of genes
 *      turn colormap into a gene
 *      turn animation into a gene
 *      turn collision box array into a gene
 *      bundle these up into a graybody gene and name it
 *      turn color palette into a gene
 *      turn collision palette into a gene
 *      bundle these up into a palette gene and name it
 *      bundle palette and graybody into a body gene and name it
 *    Make a genome containing body gene and name it
 *    Make seed containing genome and name it
 *    
 *  USE CASE 2: manual seed generation from scratch
 *  ===============================================
 *
 *  Make an SDL-based sandbox that dynamically creates genes and saves them if you like them.
 *
 */
