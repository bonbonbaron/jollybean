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

typedef struct _Reaction {
	BTreeS *treeSP;
  BTreeStatus *btStatP;
  Blackboard *bbP;
	U8 priority;
} Reaction;

typedef struct {
	Key trigger;
  Reaction reaction;
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

// (A) BBs' keys ought to be the same as their corresponding BTs: trigger. Therefore, they belong in the same map together; we don't want to compute the same key twice!

// XGo components are the functional equivalent of tasks.
typedef struct {
	U8 activePriority;
	Blackboard *activeBbP;
  BTreeStatus *activeBtStatusP;
	BTree *activeBtP;  
} XGoCompData;

typedef struct {
	XHeader xHeader;
  XGoCompData data;
} XGoComp;

typedef struct {
	Entity entity;
	Personality *personalityP;
	BBSeed *bbSeedP;  // Why array of pointers and not BBs themselves? Because btRun() takes Blackboard*.
} XGoIniSeed;

typedef struct {
	Key nSeeds;
	Key nDistinctIndividualQuirks;
	Key nDistinctHivemindQuirks;
	XGoIniSeed *seedA;
} XGoIniSeedPkg;

// (A) 
typedef struct {
	System       system;
	Key          nBBsSet;
	Map        **reactionMPA; // array of entities' reaction maps; comes packaged with requisite blackboard pointer
	Map         *hiveMindMP; // maps triggers to arrays of 
} XGo;

void xGoClr(System *sP);

#endif
