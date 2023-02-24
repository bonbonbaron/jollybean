#include "jb.h"
#include "blehColormap.h"
#include "blehColorPalette.h"
#include "heckColormap.h"
#include "heckColorPalette.h"
#include "redColormap.h"
#include "redColorPalette.h"

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

UnitaryGene_(redColormap,     EXCLUSIVE_GENE, RENDER | COLORMAP,      0, Colormap);
UnitaryGene_(redColorPalette, EXCLUSIVE_GENE, RENDER | COLOR_PALETTE, 0, Colormap);
CompositeGene_(redGraybody, &geneName_(redColormap));
CompositeGene_(redPalette,  &geneName_(redColorPalette));

Genome_(redBody, &geneName_(redGraybody), &geneName_(redPalette));

Spawn spawn = {
  .geneMutationMPA = NULL,
  .keyP = NULL,
  .nEntitiesPossible = 1,
  .nEntitiesToSpawn = 1,
  .positionNodeA = spawn1PosNodeA,
  .genomeP = &redBodyGenome
};

Biome testBiome = {
  .nEntitiesToSpawn = 0,
  .nSpawns = 0,
  .spawnA = NULL
};
