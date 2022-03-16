#include "xGo.h"

Error xGoIniSys(System *sP, void *sParamsP) {
	return SUCCESS;
}


Error xGoIniComp(System *sP, XGoComp *cP) {
	return SUCCESS;
}

typedef struct {
	BTreeSingleton *btA;
	Personality *pA;
} Package;

// Get arrays of everybody's personalities
// Zip through

// Receive a struct 
/*

		game/
		├── biome
		│   └── seed
		│       ├── genome
		│       │   ├── blackboard
		│       │   ├── ecs_component
		│       │   └── ecs_shared
		│       └── personality
		│           └── quirk
		│               └── tree
		│                   └── node
		└── include

 * */
