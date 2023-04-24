#ifndef X_GO
#define X_GO
#include "x.h"
#include "fray.h"

#define GO_ (2)
#define PERSONALITY (0x40)

void hivemindDel(Map **hivemindMPP);
void activityMPMPDel(Map **activityMPMPP);

typedef Error (*Action)(void *bbP);

typedef struct {
  Key    trigger;
  U8     priority;
  Action actionU;
} Quirk;

typedef struct {
  U32 nQuirks;
  Quirk **quirkPA;
} Personality;   // Gene-level data we expect to receive

typedef struct {
  Entity entity;
  Personality *personalityP;
} EntityPersonalityPair;

typedef struct {
} XActionMutation;

// Union of quirk, blackboard, and tree status.
typedef struct {
  Quirk       *quirkP;        // trigger-action pair
  void        *bbP;           // blackboard (stitched-together struct)
} Activity;  

typedef Activity XActionComp;

typedef struct {
	System                 system;
	Map                   *hivemindMP; // maps triggers to arrays of 
  Key                    nDistinctHivemindTriggers;
  U32                   *histoHivemindTriggerA;
  EntityPersonalityPair *entityPersonalityPairF;
} XAction;
XClrFuncDef_(Action);
XIniSubcompFuncDef_(Action);
XPostprocessCompsDef_(Action);

extern System *sActionP;
#endif
