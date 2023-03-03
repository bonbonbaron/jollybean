#ifndef X_H
#define X_H
#include "data.h"

typedef Key Entity;

// System type flags
#define FLG_NO_SWITCHES_    (0x01)
#define FLG_NO_CF_SRC_A_    (0x02)
#define FLG_NO_CHECKS_      (0x04)
#define FLG_ADD_COMP_LATER_ (0x08)

typedef enum {ACTIVATED, DEACTIVATED} BuiltinMsgArg;

// Pieces are parts of components. For example, Rendering components have colormaps and color palettes.
typedef enum { INITIALIZED = 1 } SubcomponentState; 
#define MASK_COMPONENT_TYPE    (0x3f)
#define MASK_COMPONENT_SUBTYPE (~MASK_COMPONENT_TYPE & 0xff)
// This is equivalent to bit-shifting 0xc0 by 6 bits to the right (if 0xc0 is subcomp mask).
// That way you can just change one value to affect all these definitions.
#define SUBCTYPE_TO_IDX_DIVISOR (MASK_COMPONENT_TYPE + 1)   /* compiler converts this to shift :) */
#define getSubcompIdx_(x_) ((x_ / SUBCTYPE_TO_IDX_DIVISOR) - 1)
#define N_POSSIBLE_SUBCOMPS    (MASK_COMPONENT_SUBTYPE / (SUBCTYPE_TO_IDX_DIVISOR))

#define X_(name_, id_, flags_) \
  X##name_ x##name_ = { .system = System_(name_, id_, flags_)};\
  System *s##name_##P = &x##name_.system
  

#define System_(name_, id_, flags_) \
  {\
    .id                = id_,\
    .compSz            = sizeof(X##name_##Comp),\
    .flags             = flags_,\
    .cF                = NULL,\
    .cIdx2eA           = NULL,\
    .e2cIdxMP          = NULL,\
    .mutationMPMP      = NULL,\
    .mailboxF          = NULL,\
    .deactivateQueueF  = NULL,\
    .pauseQueueF       = NULL,\
    .subcompOwnerMP    = NULL,\
    .iniSys            = x##name_##IniSys,\
    .iniSubcomp        = x##name_##IniSubcomp,\
    .postprocessComps  = x##name_##PostprocessComps,\
    .clr               = x##name_##Clr,\
    .getShare          = x##name_##GetShare,\
    .processMessage    = x##name_##ProcessMessage,\
    .run               = x##name_##Run, \
  }

struct _System;

// Function pointer types
typedef Error (*XIniSU)(struct _System *sP, void* sParamsP);
typedef Error (*XIniSubcompU)(struct _System *sP, const Entity entity, const Key subtype, void *dataP);
typedef Error (*XRunU)(struct _System *sP);
typedef Error (*XClrU)(struct _System *sP);
typedef Error (*XProcMsgU)(struct _System *sP, Message *messageP);
typedef Error (*XGetShareU)(struct _System *sP, Map *shareMMP);
typedef Error (*XPostprocessCompsU)(struct _System *sP);
typedef void* (*XSwitchCompU)(Key key);  // used to switch between a multi-form component, like rect's position.

#define XPostprocessCompsDefUnused_(name_) XPostprocessCompsDef_(name_) {\
  unused_(sP);\
  return SUCCESS;\
}
#define XPostprocessCompsDef_(name_) Error x##name_##PostprocessComps(System *sP)

#define XIniSysFuncDefUnused_(name_) XIniSysFuncDef_(name_) {\
  unused_(sP);\
  unused_(sParamsP);\
  return SUCCESS;\
}
#define XIniSysFuncDef_(name_) Error x##name_##IniSys(System *sP, void *sParamsP)

#define XIniSubcompFuncDefUnused_(name_) XIniSubcompFuncDef_(name_) {\
  unused_(sP);\
  unused_(entity);\
  unused_(subtype);\
  unused_(dataP);\
  return SUCCESS;\
}
#define XIniSubcompFuncDef_(name_)  Error x##name_##IniSubcomp(System *sP, const Entity entity, const Key subtype, void *dataP)

#define XClrFuncDefUnused_(name_) XClrFuncDef_(name_) {\
  unused_(sP);\
  return SUCCESS;\
}
#define XClrFuncDef_(name_)      Error x##name_##Clr(System *sP)

#define XProcMsgFuncDefUnused_(name_)  XProcMsgFuncDef_(name_) {\
  unused_(sP);\
  unused_(msgP);\
  return SUCCESS;\
}
#define XProcMsgFuncDef_(name_)  Error x##name_##ProcessMessage(System *sP, Message *msgP)

#define XGetShareFuncDefUnused_(name_) Error x##name_##GetShare(System *sP, Map *shareMMP) {\
  unused_(sP); \
  unused_(shareMMP); \
  return SUCCESS; \
}
#define XGetShareFuncDef_(name_) Error x##name_##GetShare(System *sP, Map *shareMMP)

#define xGetShareErrCheck \
  if (!sP || !shareMMP) \
    return E_BAD_ARGS;

typedef struct {
  Entity owner;
  void* subcompA[N_POSSIBLE_SUBCOMPS];
} SubcompOwner;

// Communcication
typedef enum {
  ACTIVATE,
  DEACTIVATE,
  PAUSE,
  UNPAUSE,
  SWITCH, 
  SWITCH_AND_ACTIVATE, 
  SWITCH_AND_DEACTIVATE, 
  N_XMAIL_BUILTIN_CMDS
} XMailCmd;

typedef struct _System {
  Key           id;                  // ID of system 
  Key           compSz;              // components are the same size in all of this system's activities 
  U8            flags;               // System flags. Use this however you want.
  void         *cF;                  // component fray 
  void         *pauseQueueF;         // queue for pausing
  void         *deactivateQueueF;    // queue for pausing
  Map          *subcompOwnerMP;       // keeps track of subcomponents' owners since cF isn't pop'd then
  Key          *cIdx2eA;             // insert component index to get entity 
  Map          *e2cIdxMP;            // insert entity to get component index 
  Map          *mutationMPMP;        // key = Entity, val = maps to void pointers (triple pointer EW)
  Message      *mailboxF;            // Where commands come in from and go out to outside world
  // Function pointers 
  XIniSU       iniSys;               // System init function pointer 
  XIniSubcompU iniSubcomp;              // Some systems need to inflate components before using them. 
  XPostprocessCompsU postprocessComps;  // If components are composites, piece them together here.
  XRunU        run;                  // runs the system 
  XClrU        clr;                  // for system cleanup (not deleting system itself) 
  XProcMsgU    processMessage;       // What to do in response to commands in inbox messages. 
  XGetShareU   getShare;             // Some systems' components share pointers to common data. This is how it retrieves them by a parent system's call. 
} System;

Error    xAddComp(System *sP, Entity entity, void *compDataP);
Error    xIniSys(System *sP, U32 nComps, void *miscP);
Error    xAddEntityData(System *sP, Entity entity, Key compType, void *entityDataP);
Error    xIniSubcomp(System *sP, const Entity entity, const void *cmpP);
Error    xAddEntity(System *sP, Entity entity, Key compType, void *compDataP, Map *mutationMP);
Error    xAddMutationMap(System *sP, Entity entity, Map *mutationMP);
//void*    xGetComp(System *sP, Entity entity);
U32      xGetNComps(System *sP);
void*    xGetCompValP(System *sP, Entity entity, Key key);
Entity   xGetEntityByCompIdx(System *sP, Key compIdx);
void*    xGetCompPByEntity(System *sP, Entity entity);
void     xActivateComponentByEntity(System *sP, Entity e1);
void     xDeactivateComponentByEntity(System *sP, Entity e1);
void     xPauseComponentByEntity(System *sP, Entity entity);
void     xUnpauseComponentByEntity(System *sP, Entity entity);
void     xSwitchComponent(System *sP, Entity entity, Key newCompKey);
void     xActivateComponentByIdx(System *sP, Key compOrigIdx);
void     xDeactivateComponentByIdx(System *sP, Key compOrigIdx);
void     xQueuePause(System *sP, void *componentP);
void     xQueueDeactivate(System *sP, void *componentP);
void     xClr(System *sP);
Error    xRun(System *sP);
#endif
