#ifndef ECS_H
#define ECS_H
#include "data.h"

#define FIRST_ACTIVITY (0)

#define Activity_(id_, sFP_) {\
  .id               = id_, \
  .ownerP           = NULL, \
  .firstInactiveIdx = 0, \
  .firstEmptyIdx   = 0, \
  .sFP              = sFP_, \
  .cA               = NULL \
}

#define System_(name_, id_, ...) \
	\
	X##name_##C x##name_##SwapPH;\
	\
	Activity x##name_##ActivityA[] = {__VA_ARGS__};\
	System s##name_ = {\
		.xHeader          = {.owner = 0, .type = 0},\
		.id               = id_,\
		.sIniSFP          = x##name_##IniS,\
		.sIniCFP          = x##name_##IniC,\
		.cSz              = sizeof(X##name_##C),\
		.swapPlaceholderP = &x##name_##SwapPH,\
		.cDirectoryP      = NULL,\
		.inbox            = {NULL, 0},\
		.outbox           = {NULL, 0},\
		.nActivities      = nArgs_(Activity, __VA_ARGS__),\
		.activityA        = &x##name_##ActivityA[0]\
	}

/**********/
/* Entity */
/**********/
typedef U8 Entity;  

struct _CDirEntry;

typedef struct {
  Entity owner;
  U8 type;
} XHeader;

/************/
/** Checks **/
/************/
//    Checks prevent a function from having to query all the components over and over
// again every frame. The original vision was for external functions to query these
// just to check conditions; now we're making that the responsibility of systems
// themselves, who have more immediate access to these components. 
//    So, instead, we're spreading those conditions out across multiple systems.
// And when preceding sequential steps are completed, we don't need to test their con-
// ditions again. We just move on to the next step. 
typedef Bln (*CheckCBP)(XHeader *xhP, void *operandP); 
typedef struct {
  Bln    cbIdx;                 // index to FP instead of FP itself to prevent external functions
  Bln    toggle;                // opposite of toggle is latch, in which case thee condition only needs to have been true once
  U8     outputIfTrue;          // condition flag to be OR'd into if true
  Key    root;                  // root of behavior tree to fire
  U8    *conditionP;            // condition to update through a simple pointer
  Entity entity;                // entity this check regards
  struct _CDirEntry *cdeP;      // keep tabs on component's location
} Check;
//TODO: ensure that when a latch-case (toggle = FALSE) check returns TRUE, the system deactivates the check.




// ********** THREE MOVING PIECES: **********
// 1) Unconditional checks that trigger roots   -- Brainstorming: What if you query a map and its required condition is zero? Then your passed-in condition is a selector. 
//                                                                Otherwise, your passed-in condition is a trigger to a sequence. The sequence doesn't trigge
// 2) Entities' states
// 3) Key to reaction map (AKA root)
//                     +--> response set (component actions and/or checks)
// 4) Current Condition
// 5) Expected Condition
// 6) 
//
//
// @@@@@@@@@@ WHAT REACTIONS DO @@@@@@@@@@
// 1) If the required condition is zero, your passed-in condition is a selector to an array of {callback, params, response set} elements. 

/*
 * If I were ignorant of everything I'd said above before cluttering my mind, this is what I would say a SEQUENCE would be:
 *  SEQUENCE is an Array of Elements made of {
 *    U8  condition in order to advance
 *    U32 pointer to current array of system commands to issue when this element is reached
 *    U32 pointer to current array of checks to distribute when this element is reached
 *    U32 callback to call when this element is reached
 *    U32 void pointer to parameters to pass to callback when this element is reached
 *  }  (that's all i can think of for now
 *
 *  SELECTOR is a struct made of {
 *    condition that must be met before selecting anything
 *    U32 pointer to array of other choices such as this one
 *    U32 pointer to current array of checks to distribute when this element is reached
 *    U32 callback to call when this element is reached
 *    U32 void pointer to parameters to pass to callback when this element is reached
 *
 *
 *
 * A behavior tree is a labyrinth of sequences and selectors. A selector could be right in the middle of a sequence; one of the selector's choices could be another sequence, branching off. Some sequence items are conditional. If there's a point you want to return to when something goes wrong somewhere in the labyrinth, there ought to be a checkpoint. 
 *
 *
 *
 * A for-loop can iterate through a sequence. 
 *
 * Tree traversal:
 *  
 *    start at root (probably a selector)
 *
 * */

/**********/
/* System */
/**********/
struct _System;
struct _Activity;
typedef Error (*SysFP)(struct _Activity *aP);
typedef Error (*SysIniFP)(void* sParamsP);
typedef Error (*SysIniCFP)(XHeader *xhP);

typedef struct _CDirEntry {
  U8 activityID;
  U8 cIdx;
  U8 checkIdx;  // index of check
	HardCodedMap *hcmP;  // Some types of components' values change under various circumstances.
  void *cP; /* Systems that use pointers to other systems' components may use double pointers to avoid requesting updated info. */
} CDirEntry;

typedef struct {
  U8 activityIdx;
  struct _Activity *activityP; /* Systems that use pointers to other systems' components may use double pointers to avoid requesting updated info. */
} ActDirEntry;

typedef enum {NO_OUTPUT, NUM_OUTPUTS} Output;

// When an external function wants to trigger a sprite walk animation and motion, the user only needs to call go(entity, WALK_LEFT). That function then starts two messages: one for the animation system (animMsg = {.contents.cmd = {ANIMATE, goKey}}). 
// Go can wrap animate() and move(). Those in turn can wrap writeMessage(), to which they tell the correct 

typedef struct _Activity {
  U8 id;  /* ID of activity */
  U8 firstInactiveIdx; /* marks the first inactive element's index */
  U8 firstEmptyIdx; /* marks the first empty element's index */
  SysFP sFP; /* function that runs on these components */
  void *cA;  /* components the above function operates on */
  struct _System *ownerP;
} Activity;

typedef struct {
  Check *checkA;
  U8 firstInactiveIdx; /* marks the first inactive element's index */
  U8 firstEmptyIdx; /* marks the first empty element's index */
} Checkers;

/* Systems are agnostic to the outside world. They just react to their inboxes and fill their outboxes. This makes them completely modular. */
typedef struct _System {
  XHeader      xHeader;             /* This allows (sub)systems to be components of other systems! */
  U8           id;                  /* this is needed to ensure messages are sent to the correct system */
  U8           cSz;                 /* components are the same size in all of this system's activities */
  U8           nActivities;         /* Number of activities in activityA[] */
  Key          firstInactiveActIdx; /* index of first inactive activity */
  void        *swapPlaceholderP;    /* Avoids allocating a new placeholder every EC-swap. */
  Map         *cDirectoryP;         /* maps component IDs to an element in an array of CmpAddresses */
  Map         *actDirectoryP;       /* maps component IDs to an element in an array of CmpAddresses */
	void        *sIniSParamsP;        /* whatever sIniSFP() needs to properly initialize this system */
  SysIniFP     sIniSFP;             /* System init function pointer */
  SysIniCFP    sIniCFP;             /* Some systems need to inflate components before using them. */
  Mailbox      inbox;               /* Where commands come in from the outside world */
  Mailbox      outbox;              /* Where this system talks to the outside world */
  Checkers     checkers;            /* Array of checks; similar to Activity without exlusive C-access */
  Activity    *activityA;           /* Array of activities that loop through their components */
} System;

// *************************
// Functions 
// *************************
void  sIniPtrs(System *sP, Activity *aP, void **startPP, void **endPP);
Error sNew(System **sPP, const U8 cSz, U8 nFuncs);
Error sIni(System *sP, U32 nComps, void *miscP);
Error sRun(System *sP);
void  sClr(System *sP);
Error sIniActivity(System *sP, Activity *aP, U32 nComps);
Error sIniActivityC(System *sP, const U8 activityIdx, const Entity entity, const void *cmpP);
Error sAddC(System *sP, Entity entity, XHeader *xhP);
void* sGetC(System *sP, Entity entity);
Error sDeactivateC(System *sP, Entity entity);
Error sActivateC(System *sP, Entity entity);
Error sStartCActivity(System *sP, Entity entity, Key dstActivityID);
Activity* sGetActivityFromE(System *sP, Entity entity);
U8 sComponentIsActive(System *sP, Entity entity);
Activity* sGetActivity(System *sP, Key actID);
Error sActivateActivity(System *sP, Key activityID);
Error sDeactivateActivity(System *sP, Key activityID);
void sAct(System *sP);
U32 sGetNComponents(System *sP);
#endif
