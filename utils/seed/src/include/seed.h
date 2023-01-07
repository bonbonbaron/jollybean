#ifndef SEED_H
#define SEED_H
#include "biome.h"
#include "fileUtils.h"
#include "genie.h"

#include "seed.h"

// Gene
Error geneNew(char *geneName, char *geneClass, char *type, U32 size, Key mutationKey, char *srcDataName);
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
 * Okay, before I drive myself crazy by going a long way down the wrong road, let's walk through how this'll work from start to finish. We'll use the context of Bodies as an example.
 *
 * Every body has a graybody and palette. Those in turn have their own components. Since it would be painful to hunt down every one of those components individually, I'm wrapping them up in the Graybody and Palette objects. Then I'm wrapping them up again in the Body object. So in the final product, the Body is the only Gene used. Beautiful and simple.
 *
 * So once body.c finishes making the body, it should ask you to name its gene. It should also ask you to name its lower-level graybody and palette components, but those won't be genes; they'll just be reusable to other bodies.
 *
 * So you name the gene, and geneNew() writes to a new gene header and source file. These files look like this:
 *
 * HEADER:   <this gene's name>.h
 * ==============================
 * #include "gene.h"
 * extern Gene <this gene's name>;
 *
 * 
 * SOURCE:   <this gene's name>.c
 * ==============================
 * #include "<this gene's name>.h"
 *
 * Gene <this gene's name> = {
 *  .geneClass = %s (gene class),
 *  .type = %s (type),
 *  .size = %d (size),
 *  .key = %s (mutation key),
 *  .dstDataP = NULL,
 *  .srcDataP = %s (srcDataName)
 * };
 *
 * Hang on: You don't need to extern the source object if you put the gene in the same file as it.
 * However, that approach will be a problem once you start sharing stuff. So you'll have to start
 * externing things at some point below the gene level.
 * 
 * All the above are automatable-- brilliant!
 * So all I have to do is name it!? Great!
 *
 * But what if I want to make a new body out of an existing graybody and palette?
 * Just extern the palette and graybody. Make the new body point to both of them. 
 *  jb gene new body
 *  
 * And what if I want to make a new palette, which doesn't techincally count as a gene?
 * */
#endif
