#include "jb.h"
#include "blehColormap.h"
#include "blehColorPalette.h"
#include "heckColormap.h"
#include "heckColorPalette.h"
#include "redColormap.h"
#include "redColorPalette.h"
#include "redAnim.h"

PositionNode spawn1PosNodeA[] = {
  {
    .keyhole = 0,
    .position = {
      .x = 50,
      .y = 50
    }
  },
  {
    .keyhole = 0,
    .position = {
      .x = 75,
      .y = 25
    }
  }
};

UnitaryGene_(redColormap,     MEDIA_GENE,     RENDER | COLORMAP,              0, Colormap);
UnitaryGene_(redColorPalette, EXCLUSIVE_GENE, RENDER | COLOR_PALETTE,         0, ColorPalette);
UnitaryGene_(redAnimation,    EXCLUSIVE_GENE, ANIMATION | ANIMATION_SUBTYPE,  0, AnimStrip);
CompositeGene_(redGraybody, 
    &geneName_(redColormap), 
    &geneName_(redAnimation)
   );
CompositeGene_(redPalette,  &geneName_(redColorPalette));
Genome_(redBody, &geneName_(redGraybody), &geneName_(redPalette));

Spawn biome1SpawnA[] = {
  {
    .geneMutationMPA = NULL,
    .keyP = NULL,
    .nEntitiesPossible = 2,
    .nEntitiesToSpawn = 0,
    .positionNodeA = spawn1PosNodeA,
    .genomeP = &genomeName_(redBody)
  }
};

Biome testBiome = {
  .nEntitiesToSpawn = 0,   // gets updated in xMaster.c::_countEntitiesToSpawn()
  .nSpawns = arrayNElems_(biome1SpawnA),
  .spawnA = biome1SpawnA
};
