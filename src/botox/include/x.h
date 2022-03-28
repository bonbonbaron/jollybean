#ifndef X_H
#define X_H
#include "data.h"

typedef Key Entity;

typedef struct {
  Entity owner;
  Key type;
} XHeader;

#define X_(name_, id_, ...) \
  X##name_##Comp x##name_##SwapPH;\
  Focus x##name_##FocusA[] = {__VA_ARGS__};\
  \
  X##name_ x##name_ = { .system=System_(name_, id_, __VA_ARGS__) };\
  \
  System *s##name_##P = &x##name_.system
  

#define System_(name_, id_, ...) \
  {\
    .id                = id_,\
    .sIniSysFP         = x##name_##IniSys,\
    .sIniCompFP        = x##name_##IniComp,\
    .sClrFP            = x##name_##Clr,\
    .sGetShareFP       = x##name_##GetShare,\
    .sProcessMessageFP = x##name_##ProcessMessage,\
    .compSz            = sizeof(X##name_##Comp),\
    .swapPlaceholderP  = &x##name_##SwapPH,\
    .compDirectoryP    = NULL,\
    .inboxP            = NULL,\
    .outboxP           = NULL,\
    .nFocuses          = nArgs_(Focus, __VA_ARGS__),\
    .focusA            = &x##name_##FocusA[0]\
  }

#define Focus_(id_, sFP_) {\
  .id               = id_, \
  .firstInactiveIdx = 0, \
  .firstEmptyIdx    = 0, \
  .focusFP          = sFP_, \
  .compA            = NULL, \
  .ownerP           = NULL \
}
struct _Focus;
struct _System;

typedef Error (*FocusFP)(struct _Focus *aP);
typedef Error (*XIniSFP)(struct _System *sP, void* sParamsP);
typedef Error (*XIniCompFP)(struct _System *sP, void *dataP);
typedef Error (*XClrFP)(struct _System *sP);
typedef Error (*XProcMsgFP)(struct _System *sP, Message *messageP);
typedef Error (*XGetShareFP)(struct _System *sP, Map *shareMMP);

#define XIniSysFuncDef_(name_)   Error x##name_##IniSys(System *sP, void *sParamsP)
#define XIniCompFuncDef_(name_)  Error x##name_##IniComp(System *sP, void *dataP)
#define XClrFuncDef_(name_)      Error x##name_##Clr(System *sP)
#define XProcMsgFuncDef_(name_)  Error x##name_##ProcessMessage(System *sP, Message *msgP)
#define XGetShareFuncDefUnused_(name_) Error x##name_##GetShare(System *sP, Map *shareMMP) {\
  unused_(sP); \
  unused_(shareMMP); \
  return SUCCESS; \
}
#define XGetShareFuncDef_(name_) Error x##name_##GetShare(System *sP, Map *shareMMP)

typedef struct _Focus {
  Key id;
  Key firstInactiveIdx; /* marks the first inactive element's index */
  Key firstEmptyIdx;    /* marks the first empty element's index */
  FocusFP focusFP;     /* function that runs on these components */
  void *compA;         /* components the above function operates on */
  struct _System *ownerP;
} Focus;

typedef struct {
  Key focusID;
  Key compIdx;
  Key checkIdx;  // index of check
  HardCodedMap *hcmP;  // Some types of components' values change under various circumstances.
  void *compP; /* Systems that use pointers to other systems' components may use double pointers to avoid requesting updated info. */
} CompLocation;

typedef struct {
  Key focusIdx;
  Focus *focusP; /* Systems that use pointers to other systems' components may use double pointers to avoid requesting updated info. */
} FocusLocation;

// When an external function wants to trigger a sprite walk animation and motion, the user only needs to call go(entity, WALK_LEFT). That function then starts two messages: one for the animation system (animMsg = {.contents.cmd = {ANIMATE, goKey}}). 
// Go can wrap animate() and move(). Those in turn can wrap writeMessage(), to which they tell the correct 

/************/
/** Checks **/
/************/
//    Checks prevent a function from having to test a condition on all the components 
// again every frame. The original vision was for behavior tree nodes to test these
// conditions; now we're making that the responsibility of systems
// themselves. Because systems have more immediate access to these components.
//    Therefore, we're spreading delegating these tests to the systems concerned.
typedef Bln (*CheckCbP)(XHeader *xhP, void *operandP);
typedef struct {
  Bln    doesToggle;            // if true, switch back and forth between two callbacks on true; if false, retire this check after first time checkCb returns true.
  Entity entity;                // entity this check regards... so you get entity's tree map, and you unlock the correct tree with the root key.
  Key    root;                  // root of behavior tree to fire 
  U8     outputIfTrueA[2];      // condition flag to be OR'd into if true
  U8     currCbIdx;             // index to cbA
  CheckCbP cbA[2];              // Up to two different callbacks based on whether it toggles.
  U8    *resultFlagsP;          // condition to update through a simple pointer
  void  *operandP;              // operand to pass into CheckCbP; gets assigned by tree node
  CompLocation *compLocationP;      // keep tabs on component's location (has to be location struct to avoid updating multiple things)
} Check;

//TODO: ensure that when a latch-case (toggle = FALSE) check returns TRUE, the system deactivates the check.
typedef struct {
  Key firstInactiveIdx; /* marks the first inactive element's index */
  Key firstEmptyIdx; /* marks the first empty element's index */
  Check *checkA;
} Checkers;

typedef struct _System {
  Key          id;                  /* ID of focus */
  Key          compSz;              /* components are the same size in all of this system's activities */
  Key          nFocuses;            /* Number of activities in focusA[] */
  Key          firstInactiveActIdx; /* index of first inactive focus */
  void        *swapPlaceholderP;    /* Avoids allocating a new placeholder every EC-swap. */
  Focus       *focusA;              // array of individual tasks to focus on. Comps can only be active in one focus at a time.
  void        *sIniSParamsP;        /* whatever sIniSysFP() needs to properly initialize this system */
  XIniSFP      sIniSysFP;             /* System init function pointer */
  XIniCompFP   sIniCompFP;             /* Some systems need to inflate components before using them. */
  XClrFP       sClrFP;
  XProcMsgFP   sProcessMessageFP;   /* What to do in response to commands in inbox messages. */
  XGetShareFP  sGetShareFP;           /* Some systems' components share pointers to common data. This is how it retrieves them by a parent system's call. */
  Mailbox     *inboxP;               /* Where commands come in from the outside world */
  Mailbox     *outboxP;              /* Where this system talks to the outside world; can actually point to another system's inbox if you want */
  Map         *compDirectoryP;      /* maps component IDs to an element in an array of CmpAddresses */
  Map         *focusDirectoryP;      /* maps component IDs to an element in an array of CmpAddresses */
  Checkers     checkers;            /* Array of checks; similar to Focus without exlusive C-access */
} System;

Error    xIniSys(System *sP, U32 nComps, void *miscP);
Error    xIniComp(System *sP, const U8 focusIdx, const Entity entity, const void *cmpP);
Error    xAddComp(System *sP, Entity entity, Key compType, Bln isMap, void *compRawDataP);
void*    xGetComp(System *sP, Entity entity);
Map*     xGetCompMapP(System *sP, Entity entity);
U32      xGetNComps(System *sP);
Error    xActivateComp(System *sP, Entity entity);
Error    xActivateFocus(Focus *fP);
Error    xDeactivateComp(System *sP, Entity entity);
Error    xDeactivateFocus(Focus *fP);
U8       xCompIsActive(System *sP, Entity entity);
Error    xStartFocus(System *sP, Entity entity, Key dstFocusID);
void     xClr(System *sP);
Error    xRun(System *sP);
#endif
