#ifndef X_H
#define X_H
#include "data.h"

typedef Key Entity;

#define X_(name_, id_, ...) \
  X##name_ x##name_ = { .system=System_(name_, id_, __VA_ARGS__) };\
  System *s##name_##P = &x##name_.system
  

#define System_(name_, id_, ...) \
  {\
    .id                = id_,\
    .iniSys         = x##name_##IniSys,\
    .iniComp        = x##name_##IniComp,\
    .clr            = x##name_##Clr,\
    .getShare       = x##name_##GetShare,\
    .processMessage = x##name_##ProcessMessage,\
    .compSz            = sizeof(X##name_##Comp),\
    .run               = x##name_##Run, \
    .compDirectoryP    = NULL,\
    .inboxP            = NULL,\
    .outboxP           = NULL,\
  }

struct _System;

typedef Error (*XIniSFP)(struct _System *sP, void* sParamsP);
typedef Error (*XIniCompFP)(struct _System *sP, void *dataP);
typedef Error (*XRunFP)(struct _System *sP);
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

/************/
/** Checks **/
/************/
//    Checks prevent a function from having to test a condition on all the components 
// again every frame. The original vision was for behavior tree nodes to test these
// conditions; now we're making that the responsibility of systems
// themselves. Because systems have more immediate access to these components.
//    Therefore, we're spreading delegating these tests to the systems concerned.
typedef Bln (*CheckCbP)(Entity entity, void *operandP);
typedef struct {
  Bln    doesToggle;            // if true, switch back and forth between two callbacks on true; if false, retire this check after first time checkCb returns true.
  Key   *cIdxP;                // entity this check regards... so you get entity's tree map, and you unlock the correct tree with the root key.
  Key    root;                  // root of behavior tree to fire 
  U8     outputIfTrueA[2];      // condition flag to be OR'd into if true
  U8     currCbIdx;             // index to cbA
  CheckCbP cbA[2];              // Up to two different callbacks based on whether it toggles.
  U8    *resultFlagsP;          // condition to update through a simple pointer
  void  *operandP;              // operand to pass into CheckCbP; gets assigned by tree node
} Check;

typedef struct _System {
  Key           id;                  /* ID of system */
  Key           compSz;              /* components are the same size in all of this system's activities */
  Key          *cIdx2eA;             /* insert component index to get entity */
  Map          *e2cIdxMP;            /* insert entity to get component index */
  void         *cF;               /* component fray */
  void         *sIniSParamsP;        /* whatever sIniSysFP() needs to properly initialize this system */
  XIniSFP       iniSys;           /* System init function pointer */
  XIniCompFP    iniComp;          /* Some systems need to inflate components before using them. */
  XRunFP        run;              /* runs the system */
  XClrFP        clr;              /* for system cleanup (not deleting system itself) */
  XProcMsgFP    processMessage;   /* What to do in response to commands in inbox messages. */
  XGetShareFP   getShare;         /* Some systems' components share pointers to common data. This is how it retrieves them by a parent system's call. */
  HardCodedMap *hcmPA;               /* array of hard-coded maps used for changing component values with simple Key commands from inbox */
  Mailbox      *inboxP;              /* Where commands come in from the outside world */
  Mailbox      *outboxP;             /* Where this system talks to the outside world; can actually point to another system's inbox if you want */
  Check        *checkF;           /* Fray of checks; these check for conditions about component's and alert the world about them when true */
} System;

Error    xIniSys(System *sP, U32 nComps, void *miscP);
Error    xIniComp(System *sP, const Entity entity, const void *cmpP);
Error    xAddComp(System *sP, Entity entity, Key compType, Bln isMap, void *compRawDataP);
void*    xGetComp(System *sP, Entity entity);
Map*     xGetCompMapP(System *sP, Entity entity);
U32      xGetNComps(System *sP);
void     xActivateComponentByEntity(System *sP, Entity e1);
void     xDeactivateComponentByEntity(System *sP, Entity e1);
void     xActivateComponentByIdx(System *sP, Key compOrigIdx);
void     xDeactivateComponentByIdx(System *sP, Key compOrigIdx);
void     xClr(System *sP);
Error    xRun(System *sP);
#endif
