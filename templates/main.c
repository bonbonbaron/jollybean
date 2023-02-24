#include "jb.h"

int main() {
  extern Biome <<INITIAL_BIOME_HERE>>;

	System *sPA[] = {
    <<SYSTEMS_HERE>>
	}; 

	Error e = xMasterIni(&xMaster, sPA, arrayNElems_(sPA), N_SYSTEM_TYPES, &<<INITIAL_BIOME_HERE>>);

	while (!e) {
		e = xRun(&xMaster.system);
  }

  return e;
}
