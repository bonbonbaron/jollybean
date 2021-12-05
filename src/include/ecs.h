#ifndef ECS_H
#define ECS_H
#include "data.h"

/* Keys */
ENUM_KEYS_(CONTROL, POSITION, MOTION, ANIMATION, COLLISION, RENDER, JOLLYBEAN, N_SYS_TYPES) SystemKey; 
/* Macros for Response, Response Sets, and Response Set Sequences */
#define RS_(...) {__VA_ARGS__, NULL}   /* Response Set (must be null-terminated) */
#define RS_SEQ_(...) {__VA_ARGS__, NULL}   /* Response Set Sequence (must be null-terminated) */
/* Macros for Behavior */
#define BEHAVIOR_(...) HARD_CODED_MAP_(void**, __VA_ARGS__)
#define RESPOND_TO_(key) {key, 
#define BY_(val) (void*) &val}
/* Macros for Personality */
#define PERSONALITY_(...) HARD_CODED_MAP_(void**, __VA_ARGS__)
#define IF_SCENE_TYPE_IS_(key) {key, 
#define BEHAVE_AS_(val) (void*) &val}
#define OTHERWISE_BEHAVE_AS_(val) {DEFAULT, (void*) &val}

#define NEW_GENOME_(name, ...) Genome name = {\
  .nGenes = NUM_ARGS_(XHeader*, __VA_ARGS__),\
  .genePA = {__VA_ARGS__}\
}

#define GENOME_GROUP_(name, ...) GenomeGrp name = {\
  .nEntities = NUM_ARGS_(Genome*, __VA_ARGS__),\
  .genomePA = {__VA_ARGS__}\
}

#define FIRST_ACTIVITY (0)

#define ACTIVITY_(id_, sFP_) {\
  .id               = id_, \
  .ownerP           = NULL, \
  .firstInactiveIdx = 0, \
  .firstEmptyIdx   = 0, \
  .sFP              = sFP_, \
  .cA               = NULL \
}

#define NEW_SYS_(name_, id_, ...) \
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
  .oneOffFPA        = NULL,\
  .inbox            = {NULL, 0},\
  .outbox            = {NULL, 0},\
  .nActivities      = NUM_ARGS_(Activity, __VA_ARGS__),\
  .activityA        = &x##name_##ActivityA[0]\
}

/**********/
/* Entity */
/**********/
typedef U8 Entity;  

typedef struct {
  Entity owner;
  U8 type;
} XHeader;

typedef struct {
  U32 nGenes;
  XHeader *genePA[];
} Genome;

typedef struct {
  U32 nEntities;
  Genome *genomePA[];
} GenomeGrp;

typedef struct {
	Genome *genomeP;
	Map *reactionMP;  // This would be a map of... 
} Seed;

/* Enums */
typedef enum {
  DIRECTOR_SYS_KEY,
  NUM_SYS_KEYS  // only used for creating stem histograms
} SysKeys;

typedef enum {
  ENTITY,         // a message to an entity never directly reaches a stem; changes E's behavior sequences
  COMPONENT,
  SYSTEM,
  NUM_ECS_TYPES   // probably never used
} ECSType;

typedef enum {
  IMMEDIATELY,
  AFTER_CURR_SYS
} MsgUrgency;

typedef enum {
  TOP_PRIORITY
} Priority;

typedef enum {
  SYSTEMWIDE_CMD,
  REPEATING_CMP_CMD,
  ONE_OFF_CMP_CMD
} CmdType;

typedef HardCodedMap Personality;


/**********/
/* System */
/**********/
struct _System;
struct _Activity;
typedef Error (*SysFP)(struct _Activity *aP);
typedef Error (*SysIniFP)(void* sParamsP);
typedef Error (*SysIniCFP)(XHeader *xhP);
typedef void (*SysBasicFP)(struct _System *sP);
typedef void (*SysOneOffFP)(struct _System *sP, Entity entity, U8 key);  /* optional key to a map or idx to array */

typedef struct {
  U8 activityID;
  U8 cIdx;
	HardCodedMap *hcmP;  // Some types of components' values change under various circumstances.
  void *cP; /* Systems that use pointers to other stems' components may use double pointers to avoid requesting updated info. */
} CDirEntry;

typedef struct {
  U8 activityIdx;
  struct _Activity *activityP; /* Systems that use pointers to other stems' components may use double pointers to avoid requesting updated info. */
} ActDirEntry;

typedef enum {NO_OUTPUT, NUM_OUTPUTS} Output;


// What's the most succinct yet powerful way to write messages?
// To, from, priority, urgency, contents
// If the hero moves, the camera must be told of it. How does the camera listen? To avoid the dilemma of every single entity's movement flooding the motion stem's outbox, we could have the one specific entity programmed to send that message on its movements. However, the camera make want to switch targets to follow. In such case, we must be flexible in who sends that message. So outputs must be configurable on the fly. 
typedef struct {
	Entity to;
	union {
		struct {
			Key id;  // event ID
			U8 type;  // needed by systems like collision, battle, etc.
			Entity otherEntity;  // needed by systems like motion if tracking someone
		} event;
		struct {
			Key sysID;
			Key activityID;
			Key key;  // tells activity what specifically to do with component every loop 
		} cmd;
	} contents;
} Message;  
// When an external functi9on wants to trigger a sprite walk animation and motion, the user only needs to call go(entity, WALK_LEFT). That function then starts two messages: one for the animation system (animMsg = {.contents.cmd = {ANIMATE, goKey}}). 
// Go can wrap animate() and move(). Those in turn can wrap writeMessage(), to which they tell the correct 

typedef struct {
  Message *msgA;
  U32 nMsgs;
} Mailbox;

typedef struct _Activity {
  U8 id;  /* ID of activity */
  U8 firstInactiveIdx; /* marks the first inactive element's index */
  U8 firstEmptyIdx; /* marks the first empty element's index */
  SysFP sFP; /* function that runs on these components */
  void *cA;  /* components the above function operates on */
  struct _System *ownerP;
} Activity;

/* Systems are agnostic to the outside world. They just react to their inboxes and fill their outboxes. This makes them completely modular. */
typedef struct _System {
  XHeader      xHeader;             /* This allows (sub)stems to be components of other stems! */
  U8           id;                  /* this is needed to ensure messages are sent to the correct stem */
  U8           cSz;                 /* components are the same size in all of this stem's activities */
  U8           nActivities;         /* Number of activities in activityA[] */
  void        *swapPlaceholderP;    /* Avoids allocating a new placeholder every EC-swap. */
  Map         *cDirectoryP;         /* maps component IDs to an element in an array of CmpAddresses */
  Map         *actDirectoryP;       /* maps component IDs to an element in an array of CmpAddresses */
  Key          firstInactiveActIdx; /* index of first inactive activity */
	void        *sIniSParamsP;         /* whatever sIniSFP() needs to properly initialize this system */
  SysIniFP     sIniSFP;              /* System init function pointer */
  SysIniCFP    sIniCFP;              /* Some stems need to inflate components before using them. */
  SysOneOffFP *oneOffFPA;           /* Array of one-off functions for stem to perform one action immediately on an EC */
  Mailbox      inbox;               /* Where commands come in from the outside world */
  Mailbox      outbox;              /* Where this stem talks to the outside world */
  Activity    *activityA;            /* Array of activities that loop through their components */
} System;

typedef enum {
  SYS_TOGGLE_ACTIVE,
  _MAX_SYSTEMWIDE_CMD_ID
} SysCmdID;


// *************************
// Functions
// *************************
void  sIniPtrs(System *sP, Activity *aP, void **startPP, void **endPP);
Error sNew(System **sPP, const U8 cSz, U8 nFuncs);
Error sIni(System *sP, U32 nComps, void *miscP);
void  sRun(System *sP);
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
void sSendMessage(System *sP, Message *msgP);
Error sDeliverMessage(Key sysID, Message *msgP);
void sAct(System *sP);
U32 sGetNComponents(System *sP);
#endif
