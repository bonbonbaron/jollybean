#ifndef X_H
#define X_H
#include "data.h"

typedef Key Entity;

#define FLG_NO_SWITCHES_   (1 << 0)
#define FLG_NO_CF_SRC_A_   (1 << 1)
#define FLG_NO_CHECKS_     (1 << 2)

typedef enum {ACTIVATED, DEACTIVATED} BuiltinMsgArg;

// Pieces are parts of components. For example, Rendering components have colormaps and color palettes.
typedef enum { INITIALIZED = 1 } PieceState; 
#define MASK_COMPONENT_TYPE    (0x3f)
#define MASK_COMPONENT_SUBTYPE (~MASK_COMPONENT_TYPE)

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
    .switchMPMP        = NULL,\
    .inboxF            = NULL,\
    .outboxF           = NULL,\
    .deactivateQueueF  = NULL,\
    .pauseQueueF       = NULL,\
    .iniSys            = x##name_##IniSys,\
    .iniSubcomp       = x##name_##IniSubcomp,\
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

#define XIniSysFuncDefUnused_(name_) xIniSysFuncDef_(name_) {\
  unused_(sP);\
  unused_(sParamsP);\
  return SUCCESS;\
}
#define xIniSysFuncDef_(name_) Error x##name_##IniSys(System *sP, void *sParamsP)

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

// TODO add postprocess function
typedef struct _System {
  Key           id;                  // ID of system 
  Key           compSz;              // components are the same size in all of this system's activities 
  U8            flags;               // System flags; one example use is preventing unnecessary allocations of unused system parts
  void         *cF;                  // component fray 
  void         *pauseQueueF;         // queue for pausing
  void         *deactivateQueueF;    // queue for pausing
  Key          *cIdx2eA;             // insert component index to get entity 
  Map          *e2cIdxMP;            // insert entity to get component index 
  Map          *switchMPMP;          // key = Entity, val = maps to void pointers (triple pointer EW)
  Message      *inboxF;              // Where commands come in from the outside world 
  Message      *outboxF;             // Where this system talks to the outside world; can actually point to another system's inbox if you want 
  // Function pointers 
  XIniSU       iniSys;               // System init function pointer 
  XIniSubcompU iniSubcomp;              // Some systems need to inflate components before using them. 
  XPostprocessCompsU postprocessComps;  // If components are composites, piece them together here.
  XRunU        run;                  // runs the system 
  XClrU        clr;                  // for system cleanup (not deleting system itself) 
  XProcMsgU    processMessage;       // What to do in response to commands in inbox messages. 
  XGetShareU   getShare;             // Some systems' components share pointers to common data. This is how it retrieves them by a parent system's call. 
} System;

Error    xIniSys(System *sP, U32 nComps, void *miscP);
Error    xIniSubcomp(System *sP, const Entity entity, const void *cmpP);
Error    xAddEntity(System *sP, Entity entity, Key compType, void *compDataP, Map *switchMP);
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
