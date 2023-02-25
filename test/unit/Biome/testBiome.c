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

Rect_ dummySrcRect = {0};  // this'll be updated later
Rect_ dummyDstRect = {0};  // this'll be updated later

UnitaryGene_(redColormap,     MEDIA_GENE,     RENDER | COLORMAP,      0, Colormap);
UnitaryGene_(redColorPalette, EXCLUSIVE_GENE, RENDER | COLOR_PALETTE, 0, Colormap);
UnitaryGene_(dummySrcRect,    SHARED_GENE,    SRC_RECT,               0, Rect_);
UnitaryGene_(dummyDstRect,    SHARED_GENE,    DST_RECT,               0, Rect_);
CompositeGene_(redGraybody, &geneName_(redColormap), &geneName_(dummySrcRect), &geneName_(dummyDstRect));
CompositeGene_(redPalette,  &geneName_(redColorPalette));
Genome_(redBody, &geneName_(redGraybody), &geneName_(redPalette));

Spawn biome1SpawnA[] = {
  {
    .geneMutationMPA = NULL,
    .keyP = NULL,
    .nEntitiesPossible = 1,
    .nEntitiesToSpawn = 1,
    .positionNodeA = spawn1PosNodeA,
    .genomeP = &redBodyGenome
  }
};

Biome testBiome = {
  .nEntitiesToSpawn = 0,   // gets updated in xMaster.c::_countEntitiesToSpawn()
  .nSpawns = arrayNElems_(biome1SpawnA),
  .spawnA = biome1SpawnA
};
