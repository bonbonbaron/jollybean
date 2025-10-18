#include <assert.h>
#include "gene.h"

// forward-declare Gene since x and gene are circular dependencies.
struct Gene;

extern struct Gene rootGene;

// In the real engine's runtime, each step below is done for all systems at once.
void testStartX( System* sP ) {
  assert( sP );
  xIniSys( sP );
  distributeGenes( &rootGene );
  sP->postprocessComps( sP );
}
