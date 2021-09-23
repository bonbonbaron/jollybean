#ifndef ECS_H
#define ECS_H
#include "data.h"

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

/* Enums */
typedef enum {
	DIRECTOR_SYS_KEY,
	NUM_SYS_KEYS  // only used for creating system histograms
} SysKeys;

typedef enum {
	ENTITY,         // a message to an entity never directly reaches a system; the entity likely changes sequences
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

typedef HardCodedMap Behavior;
typedef HardCodedMap Personality;

struct _Component;

/**********/
/* Entity */
/**********/

typedef struct {
	void *compSeedP;
} Gene;

typedef struct {
	KeyValPair *personalityA;
	Gene *geneA;
} EntitySeed;

typedef U8 Entity;  /* HAH! That's all an entity is!? A key to its components like we're a key to our atoms! */

/*************/
/* Component */
/*************/
typedef struct {
	U8 sysKey;
	U8 cmpSz;  /* this size does not include the Entity byte */
	void *cmpDataP;
	Map *cmpMapP;  /* For things like animation, this is crucial. We won't copy entire maps to a system; just the cmps' init vals. */
} ComponentSeed;

/**********/
/* System */
/**********/
struct _System;
struct _Activity;
typedef Error (*SysFP)(struct _System *sysP, struct _Activity *aP, void *cmpA);
typedef Error (*SysIniFP)(struct _System *sysP, void *cmpA);
typedef void (*SysBasicFP)(struct _System *sP);
typedef void (*SysOneOffFP)(struct _System *sP, Entity entity, U8 key);  /* optional key to a map or idx to array */

typedef struct {
	U8 activityIdx;
	U8 ecIdx;
  void *ecP;      /* Systems that use pointers to other systems' components may use double pointers to avoid requesting updated info. */
} ECLocation;

typedef U8 CmpType;

typedef struct {
	Entity owner;
	CmpType type;
} CmpHeader;

typedef enum {NO_OUTPUT, NUM_OUTPUTS} Output;
typedef struct {
	Key sysKey;
	U8  funcEnum;
	Key miscKey;
	Output  output;
} Response;

typedef struct {
	U8						 cmd;
	CmdType        cmdType;
	ECSType        toECSType;
	U32            toID;          /* id of recipient */
	U32            misc;          /* any miscellaneous content may be included here */
	MsgUrgency		 urgency;       /* speed at which message must reach its destination */
	Priority       priority;      /* priority which message command has over any existing component activity */
	void *paramsP;                /* Parameters for the target to pass to the cmd function... IF tgtECSType == SYSTEM. */
} Message;

typedef struct {
	Message *msgA;
	U32 nMsgs;
} Mailbox;

// TODO: revamp sys.c to support an array of SysActivity elements instead of several disparate elements talking about the same thing.
typedef struct _Activity {
	U8 active; /* boolean for whether this function should even operate */
	U8 firstInactiveIdx; /* marks the first inactive element's index */
	U8 firstEmptyIdx; /* marks the first empty element's index */
	SysFP sysFP; /* function that runs on these components */
	void *ecA;  /* components the above function operates on */
} Activity;

/* Systems are agnostic. They know nothing outside themselves. They just react to their inboxes. When they're done with that, they fill their outboxes. The outside world can do whatever with it that they wish. */
typedef struct _System {
  U8           id;                /* this is needed to ensure messages are sent to the correct system */
	U8           active;            /* boolean for whether this syskem should even operate */
	U8           ecSz;              /* components are the same size in all of this system's activities */
  void        *swapPlaceholderP;  /* Avoids allocating a new placeholder every EC-swap. */
	Map         *ecLocationMapP;   /* maps component IDs to an element in an array of CmpAddresses */
	Activity    *activityA;         /* Array of activities that loop through their components */
	SysOneOffFP *oneOffFPA;         /* Array of one-off functions for system to perform one action immediately on an EC */
	Mailbox      inbox;            /* Where commands come in from the outside world */
	Mailbox      outbox;           /* Where this system talks to the outside world */
} System;

typedef enum {
	SYS_TOGGLE_ACTIVE,
	_MAX_SYSTEMWIDE_CMD_ID
} SysCmdID;


void sysToggleActive(System *sP);

void sysIniPtrs(System *sP, Activity *aP, void **startPP, void **endPP);
Error sysNew(System **sPP, U8 sysID, const U8 ecSz, U8 nFuncs);
void sysIni(System *sP, U8 sysID, SysFP *funcPtrA, U8 ecSz);
void sysDel(System **sPP);
#endif
