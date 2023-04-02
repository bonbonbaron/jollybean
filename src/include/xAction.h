#ifndef X_GO
#define X_GO
#include "x.h"

#define GO_ (2)
#define PERSONALITY (0x40)

void hivemindDel(Map **hivemindMPP);
void activityMPMPDel(Map **activityMPMPP);

typedef Error (*Action)(Entity entity, Map *bbMP);

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

// Union of quirk, blackboard, and tree status.
typedef struct {
  Entity       entity;
  Quirk       *quirkP;         // action 
  Map         *bbMP;           // blackboard
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
