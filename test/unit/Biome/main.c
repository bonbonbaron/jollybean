#include "jb.h"

int main(int argc, char **argv) {
  extern Biome testBiome;

	System *sPA[] = {
    sRenderP,
    sAnimP
	}; 

	Error e = xMasterIni(&xMaster, sPA, arrayNElems_(sPA), N_SYSTEM_TYPES, &testBiome);

	while (!e) {
		e = xRun(&xMaster.system);
  }

  return e;
}
