#ifndef X_H
#define X_H
#include "data.h"

typedef Key Entity;

typedef struct {
	Entity owner;
	U8 type;
} XHeader;

#define System_(name_, id_, ...) \
	\
	X##name_##Comp x##name_##SwapPH;\
	\
	Focus x##name_##FocusA[] = {__VA_ARGS__};\
	System s##name_ = {\
		.xHeader          = {.owner = 0, .type = 0},\
		.id               = id_,\
		.sIniSFP          = x##name_##IniSys,\
		.sIniCFP          = x##name_##IniComp,\
		.compSz           = sizeof(X##name_##Comp),\
		.swapPlaceholderP = &x##name_##SwapPH,\
		.compDirectoryP   = NULL,\
		.inboxP           = NULL,\
		.outboxP          = NULL,\
		.nFocuses         = nArgs_(Focus, __VA_ARGS__),\
		.focusA           = &x##name_##FocusA[0]\
	}

#define Focus_(id_, sFP_) {\
  .id               = id_, \
  .firstInactiveIdx = 0, \
  .firstEmptyIdx   = 0, \
  .focusFP              = sFP_, \
  .compA               = NULL, \
  .ownerP           = NULL \
}
struct _Focus;
struct _System;

typedef Error (*FocusFP)(struct _Focus *aP);
typedef Error (*XIniSFP)(struct _System *sP, void* sParamsP);
typedef Error (*XIniCompFP)(XHeader *xhP);

typedef struct _Focus {
	U8 id;
  U8 firstInactiveIdx; /* marks the first inactive element's index */
  U8 firstEmptyIdx;    /* marks the first empty element's index */
  FocusFP focusFP;     /* function that runs on these components */
  void *compA;         /* components the above function operates on */
	struct _System *ownerP;
} Focus;

typedef struct {
  U8 focusID;
  U8 compIdx;
  U8 checkIdx;  // index of check
	HardCodedMap *hcmP;  // Some types of components' values change under various circumstances.
  void *compP; /* Systems that use pointers to other systems' components may use double pointers to avoid requesting updated info. */
} CompLocation;

typedef struct {
  U8 focusIdx;
  Focus *focusP; /* Systems that use pointers to other systems' components may use double pointers to avoid requesting updated info. */
} FocusLocation;

// When an external function wants to trigger a sprite walk animation and motion, the user only needs to call go(entity, WALK_LEFT). That function then starts two messages: one for the animation system (animMsg = {.contents.cmd = {ANIMATE, goKey}}). 
// Go can wrap animate() and move(). Those in turn can wrap writeMessage(), to which they tell the correct 

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
  struct Complocation *compLocationP;      // keep tabs on component's location
} Check;
//
//TODO: ensure that when a latch-case (toggle = FALSE) check returns TRUE, the system deactivates the check.
typedef struct {
  Check *checkA;
  U8 firstInactiveIdx; /* marks the first inactive element's index */
  U8 firstEmptyIdx; /* marks the first empty element's index */
} Checkers;

typedef struct _System {
	XHeader      xHeader;
  U8           id;                  /* ID of focus */
  U8           compSz;							/* components are the same size in all of this system's activities */
  U8           nFocuses;            /* Number of activities in focusA[] */
  Key          firstInactiveActIdx; /* index of first inactive focus */
  void        *swapPlaceholderP;    /* Avoids allocating a new placeholder every EC-swap. */
	Focus       *focusA;              // array of individual tasks to focus on. Comps can only be active in one focus at a time.
	void        *sIniSParamsP;        /* whatever sIniSFP() needs to properly initialize this system */
  XIniSFP      sIniSFP;             /* System init function pointer */
  XIniCompFP   sIniCFP;             /* Some systems need to inflate components before using them. */
  Mailbox     *inboxP;               /* Where commands come in from the outside world */
  Mailbox     *outboxP;              /* Where this system talks to the outside world */
  Map         *compDirectoryP;      /* maps component IDs to an element in an array of CmpAddresses */
  Map         *focusDirectoryP;      /* maps component IDs to an element in an array of CmpAddresses */
  Checkers     checkers;            /* Array of checks; similar to Focus without exlusive C-access */
} System;

Error    xIniSys(System *sP, U32 nComps, void *miscP);
Error	 	 xIniComp(System *sP, const U8 focusIdx, const Entity entity, const void *cmpP);
Error	 	 xAddComp(System *sP, Entity entity, XHeader *xhP);
void*	 	 xGetComp(System *sP, Entity entity);
U32      xGetNComps(System *sP);
Error	 	 xActivateComp(System *sP, Entity entity);
Error    xActivateFocus(Focus *fP);
Error	 	 xDeactivateComp(System *sP, Entity entity);
Error    xDeactivateFocus(Focus *fP);
U8       xCompIsActive(System *sP, Entity entity);
Error    xStartFocus(System *sP, Entity entity, Key dstFocusID);
Error    xRun(System *sP);
#endif
