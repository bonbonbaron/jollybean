#ifndef SEED_H
#define SEED_H
#include "biome.h"
#include "fileUtils.h"

#include "seed.h"

// Gene
Error geneNew(char *geneName, char *geneClass, char *type, U32 size, Key mutationKey, char *srcDataName);
// Genome
Error genomeNew(Genome **genomePP);
Error genomeAddGene(Genome *genomeP, Gene *geneP);
Error genomeRemGene(Genome *genomeP, U32 geneIdx);
Error genomeEditGene(Genome *genomeP, Gene *newGeneP, U32 oldGeneIdx);
// Quirk
Error quirkNew(Quirk **quirkPP, Key trigger, void *actionP);
// Personality
Error personalityNew(Personality **personatityPP);
Error personalityAddQuirk(Personality *personalityP, Quirk *quirkP);
Error personalityRemQuirk(Personality *personalityP, U32 quirkIdx);
Error personalityEditQuirk(Personality *personalityP, Quirk *newQuirkP, U32 oldQuirkIdx);
// Seed
Error seedNew(Seed **seedPP);
Error seedAddGenome(Seed *seedP, Genome *genomeP);
Error seedAddPersonality(Seed *seedP, Personality *personalityP);
Error seedRemGenome(Seed *seedP);  // don't need index as there's only one genome per seed
Error seedRemPersonality(Seed *seedP);  // don't need index as there's only one personality per seed
Error seedEditGenome(Seed *seedP, Genome *newGenomeP);  // don't need index as there's only one genome per seed
Error seedEditPersonality(Seed *seedP, Personality *newPersonalityP);  // don't need index as there's only one personality per seed

#endif
