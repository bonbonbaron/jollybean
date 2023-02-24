#ifndef X_GO
#define X_GO
#include "bt.h"

// TODO make this either bt-based or function pointer-based

#define GO_ (2)
#define PERSONALITY (0x40)

void hivemindDel(Map **hivemindMPP);
void activityMPMPDel(Map **activityMPMPP);

// Union of quirk, blackboard, and tree status.
typedef struct {
	U8           priority;       // priority of this activity to compare to any other requested one
	BTree       *btP;        // current tree from system's switch
  Map         *bbMP;           // blackboard
  BTreeStatus *btStatP;        // status of tree's nodes for this activity's owner (entity)
} Activity;  

typedef Activity XGoComp;

typedef struct {
  Key    trigger;
  U8     priority;
  BTree *btP;
} Quirk;

typedef struct {
  U32 nQuirks;
  Quirk **quirkPA;
} Personality;   // Gene

typedef struct {
  Entity entity;
  Personality *personalityP;
} EntityPersonalityPair;

typedef struct {
	System                 system;
	Map                   *hivemindMP; // maps triggers to arrays of 
  Key                    nDistinctHivemindTriggers;
  U32                   *histoHivemindTriggerA;
  EntityPersonalityPair *entityPersonalityPairF;
} XGo;
XClrFuncDef_(Go);
XIniSubcompFuncDef_(Go);
XPostprocessCompsDef_(Go);

extern System *sGoP;
#endif
