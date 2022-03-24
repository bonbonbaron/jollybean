#ifndef X_GO
#define X_GO
#include "botox.h"

// Quirk is just a mapping from trigger to reaction (behavior tree).
#define Quirk_(name_, cmd_, priority_, treeSP_) \
	Quirk name_ = {\
		.cmd = cmd_,\
		.priority = priority_,\
		.treeSP = treeSP_\
	};

typedef struct {
	Key trigger;
	U8 priority;
	// U8 hiveMinded; <--- If you decide to implement this, add check for this in _histoHiveMindTriggers().
	BTreeSingleton *treeSP;
} Quirk;

// Personality is a hard-coded mapping of Key events to tree pointers. 
#define Personality_(name_, ...) \
	Personality name_ = {\
		.quirkPA = {__VA_ARGS__}\
	};

typedef struct {
	Key nQuirks;
	Quirk **quirkPA;
} Personality;

// XGo components are the functional equivalent of tasks.
typedef struct {
	XHeader xHeader;
	U8 priority;
	Key bbIdx;
	BTree *btP;
} XGoComp;

typedef struct {
	BTree *btP;
	U8 priority;
} Reaction;

typedef struct {
	Entity entity;
	Personality *personalityP;
	Blackboard *bbP;  // Why array of pointers and not BBs themselves? Because btRun() takes Blackboard*.
} XGoIniSeed;

typedef struct {
	Key nSeeds;
	Key nDistinctIndividualQuirks;
	Key nDistinctHivemindQuirks;
	XGoIniSeed *seedA;
} XGoIniSeedPkg;

typedef struct {
	System system;
	Key          nBBsSet;
	Map        **bTreeMPA;
	Blackboard **bbPA;
	Map         *hiveMindMP; // maps triggers to arrays of 
} XGo;

void xGoClr(System *sP);

#endif
