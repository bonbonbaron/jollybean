#ifndef X_H
#define X_H
#include "data/map.h"
#include "data/fray.h"
#include "data/mail.h"

typedef Key Entity;

// System type flags
#define FLG_NO_MUTATIONS_   (0x01)
#define FLG_NO_CF_SRC_A_    (0x02)

typedef enum {ACTIVATED, DEACTIVATED} BuiltinMsgArg;

// Subcomponents are things used to build components. 
// For example, Rendering components have colormaps and color palettes, and possibly tilemaps.
typedef enum { INITIALIZED = 1 } SubcomponentState; 
#define MASK_COMPONENT_TYPE    (0x3f)
#define MASK_COMPONENT_SUBTYPE (~MASK_COMPONENT_TYPE & 0xff)
// This is equivalent to bit-shifting 0xc0 by 6 bits to the right (if 0xc0 is subcomp mask).
// That way you can just change one value to affect all these definitions.
#define SUBCTYPE_TO_IDX_DIVISOR (MASK_COMPONENT_TYPE + 1)   /* compiler converts this to shift :) */
#define getSubcompIdx_(x_) ((x_ / SUBCTYPE_TO_IDX_DIVISOR) - 1)
#define N_POSSIBLE_SUBCOMPS    (MASK_COMPONENT_SUBTYPE / (SUBCTYPE_TO_IDX_DIVISOR))

#define X_(name_, id_, fieldToMutate_, flags_) \
  X##name_ x##name_ = { .system = System_(name_, id_, fieldToMutate_, flags_)};\
  System *s##name_##P = &x##name_.system

#define X_no_mutations_(name_, id_, flags_) \
  X##name_ x##name_ = { .system = System_no_mutations_(name_, id_, flags_)};\
  System *s##name_##P = &x##name_.system

/* name_:          name of system
 * id_             id of system
 * flags_:         bit-field to set special system options in (see FLG_XXXX macros above)
 * mutationField_: member of component struct to memcpy mutations to
 *                 NOTE: mutations may span multiple fields. 
 */

#define System_(name_, id_, fieldToMutate_, flags_) \
  {\
    .id                = id_,\
    .compSz            = sizeof(X##name_##Comp),\
    .mutationSz        = sizeof(X##name_##Mutation),\
    .mutationOffset    = structMemberOffset_(X##name_##Comp, fieldToMutate_),\
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
    .postMutate        = x##name_##PostMutate,\
    .postActivate      = x##name_##PostActivate,\
    .postDeactivate    = x##name_##PostDeactivate,\
    .getShare          = x##name_##GetShare,\
    .processMessage    = x##name_##ProcessMessage,\
    .run               = x##name_##Run, \
  }

#define System_no_mutations_(name_, id_, flags_) \
  {\
    .id                = id_,\
    .compSz            = sizeof(X##name_##Comp),\
    .mutationSz        = 0,\
    .mutationOffset    = 0,\
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
    .postMutate        = x##name_##PostMutate,\
    .postActivate      = x##name_##PostActivate,\
    .postDeactivate    = x##name_##PostDeactivate,\
    .getShare          = x##name_##GetShare,\
    .processMessage    = x##name_##ProcessMessage,\
    .run               = x##name_##Run, \
  }

struct _System;

// Function pointer types
typedef void (*XIniSU)(struct _System *sP, void* sParamsP);
typedef void (*XIniSubcompU)(struct _System *sP, const Entity entity, const Key subtype, void *dataP);
typedef void (*XRunU)(struct _System *sP);
typedef void (*XProcMsgU)(struct _System *sP, Message *messageP);
typedef void (*XGetShareU)(struct _System *sP, Map* shareMPMP);
typedef void (*XPostprocessCompsU)(struct _System *sP);
typedef void (*XPostMutateU)(struct _System *sP, void *cP);  // changes immutables alogn with mutables
typedef void (*XPostActivateU)(struct _System *sP, FrayChanges *changesP);  // changes immutables alogn with mutables
typedef void (*XPostDeactivateU)(struct _System *sP, FrayChanges *changesP);  // changes immutables alogn with mutables
// TODO if these functions are unused, then should they be null? 
// Or since they're only called at startup, does it really matter?
#define XPostprocessCompsDefUnused_(name_) XPostprocessCompsDef_(name_) {\
  unused_(sP);\
}
#define XPostprocessCompsDef_(name_) void x##name_##PostprocessComps(System *sP)

#define XIniSysFuncDefUnused_(name_) XIniSysFuncDef_(name_) {\
  unused_(sP);\
  unused_(sParamsP);\
}

#define XIniSysFuncDef_(name_) void x##name_##IniSys(System *sP, void *sParamsP)

#define XIniSubcompFuncDefUnused_(name_) XIniSubcompFuncDef_(name_) {\
  unused_(sP);\
  unused_(entity);\
  unused_(subtype);\
  unused_(dataP);\
}
#define XIniSubcompFuncDef_(name_)  void x##name_##IniSubcomp(System *sP, const Entity entity, const Key subtype, void *dataP)

#define XProcMsgFuncDefUnused_(name_)  XProcMsgFuncDef_(name_) {\
  unused_(sP);\
  unused_(msgP);\
}
#define XProcMsgFuncDef_(name_)  void x##name_##ProcessMessage(System *sP, Message *msgP)

#define XGetShareFuncDefUnused_(name_) void x##name_##GetShare(System *sP, Map* shareMPMP) {\
  unused_(sP); \
  unused_(shareMPMP); \
}
#define XGetShareFuncDef_(name_) void x##name_##GetShare(System *sP, Map* shareMPMP)

#define XRunFuncDef_(name_) void x##name_##Run(System *sP)

#define XPostMutateFuncDefUnused_(name_) void x##name_##PostMutate(System *sP, void *cP) {\
  unused_(sP);\
  unused_(cP);\
}

#define XPostMutateFuncDef_(name_) void x##name_##PostMutate(System *sP, void *cP)

#define XPostActivateFuncDefUnused_(name_) void x##name_##PostActivate(System *sP, FrayChanges* changesP) {\
  unused_(sP);\
  unused_(changesP);\
}

#define XPostActivateFuncDef_(name_) void x##name_##PostActivate(System *sP, FrayChanges* changesP)

#define XPostDeactivateFuncDefUnused_(name_) void x##name_##PostDeactivate(System *sP, FrayChanges* changesP) {\
  unused_(sP);\
  unused_(changesP);\
}

#define XPostDeactivateFuncDef_(name_) void x##name_##PostDeactivate(System *sP, FrayChanges* changesP)



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
  MUTATE, 
  MUTATE_AND_ACTIVATE, 
  MUTATE_AND_DEACTIVATE, 
  N_XMAIL_BUILTIN_CMDS
} XMailCmd;

typedef struct _System {
  Key           id;                  // ID of system 
  U8            compSz;              // components are the same size in all of this system's activities 
  U8            mutationSz;          // System flags. Use this however you want.
  U8            mutationOffset;      // position in destination component struct to copy mutation to
  U32           flags;               // System flags. Use this however you want.
  void         *cF;                  // component fray 
  Entity       *pauseQueueF;         // queue for pausing
  Entity       *deactivateQueueF;    // queue for pausing
  Map          *subcompOwnerMP;      // keeps track of subcomponents' owners since cF isn't pop'd then
  Key          *cIdx2eA;             // insert component index to get entity 
  Map          *e2cIdxMP;            // insert entity to get component index 
  Map          *mutationMPMP;        // key = Entity, val = maps to void pointers (triple pointer EW)
  Message      *mailboxF;            // Where commands come in from and go out to outside world
  // Function pointers 
  XRunU        run;                  // runs the system 
  XProcMsgU    processMessage;       // What to do in response to commands in inbox messages. 
  XPostMutateU postMutate;           // changes immutables after mutables are changed
  XPostActivateU postActivate;       // changes sys-specific members after component is activated
  XPostDeactivateU postDeactivate;   // changes sys-specific members after component is deactivated
  XIniSU       iniSys;               // System init function pointer 
  XIniSubcompU iniSubcomp;              // Some systems need to inflate components before using them. 
  XPostprocessCompsU postprocessComps;  // If components are composites, piece them together here.
  XGetShareU   getShare;             // Some systems' components share pointers to common data. This is how it retrieves them by a parent system's call. 
} System;

void    xAddComp(System *sP, Entity entity, void *compDataP);
void    xMutateComponent(System *sP, Entity entity, Key newCompKey);
void    xIniSys(System *sP, U32 nComps, void *miscP);
void    xAddEntityData(System *sP, Entity entity, Key compType, void *entityDataP);
void    xIniSubcomp(System *sP, const Entity entity, const void *cmpP);
void    xAddEntity(System *sP, Entity entity, Key compType, void *compDataP, Map *mutationMP);
void    xAddMutationMap(System *sP, Entity entity, Map *mutationMP);
//void*    xGetComp(System *sP, Entity entity);
Entity   xGetEntityByVoidComponentPtr(System *sP, void *componentP);
U32      xGetNComps(System *sP);
void*    xGetCompValP(System *sP, Entity entity, Key key);
Entity   xGetEntityByCompIdx(System *sP, Key compIdx);
void*    xGetCompPByEntity(System *sP, Entity entity);
void     xActivateComponentByEntity(System *sP, Entity e1);
void     xDeactivateComponentByEntity(System *sP, Entity e1);
void     xPauseComponentByEntity(System *sP, Entity entity);
void     xUnpauseComponentByEntity(System *sP, Entity entity);
void     xActivateComponentByIdx(System *sP, Key compOrigIdx);
void     xDeactivateComponentByIdx(System *sP, Key compOrigIdx);
void     xQueuePause(System *sP, void *componentP);
void     xQueueDeactivate(System *sP, void *componentP);
Bln      xIsEntityActive( System  *sP, Entity entity );
void     xRun(System *sP);
void     __xSwap(System *sP, S32 origIdx, S32 newIdx);

inline  Key* _getCompIdxPByEntity(System *sP, Entity entity) {
  return (Key*) mapGet(sP->e2cIdxMP, entity);
}

#endif
