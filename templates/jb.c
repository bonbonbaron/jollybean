#include "jb.h"

int main() {
  extern Biome <<INITIAL_BIOME_HERE>>;

	System *sPA[] = {
    <<SYSTEMS_HERE>>
	}; 

	Error e = xMasterIni(&xMaster, sGoP, sPA, sizeof(sPA) / sizeof(sPA[0]), N_SYSTEM_TYPES, &<<INITIAL_BIOME_HERE>>);

	while (!e) 
		e = xRun(&xMaster.system);

  return e;
}

// TODO:
// =====
//  \1) test BB by making a tree that uses it.
//  \2) move behavior tree stuff to their own directory
//  3) load entity's behavior trees at scene start
//  4) send message to system; at least just start an activity
//  5) receive message from a system
//  6) send check to a system
//  7) be triggered by a positive check in a system
