#ifndef ECS_H
#define ECS_H
#include "data.h"

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

/* This is how every system increments the component pointer. */
#define INCR_CMP_P(ptr, stepSz) (((U8*) ptr) += stepSz)
#define INI_CMP_P(ptr) (((U8*) ptr) + sizeof(Entity))

struct _Component;

/**********/
/* Entity */
/**********/
typedef struct {
	struct _Component *componentsA;
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
typedef Error (*SysFP)(struct _System *sysP, void *cmpA);
typedef Error (*SysIniFP)(struct _System *sysP, void *cmpA);
typedef void (*SysBasicFP)(struct _System *sP);
typedef void (*SysOneOffFP)(struct _System *sP, Entity entity, U8 key);  /* optional key to a map or idx to array */

typedef struct {
	U8 activityIdx;
	U8 ecIdx;
  void *ecP;      /* Systems that use pointers to other systems' components may use double pointers to avoid requesting updated info. */
} ECLocation;

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
typedef struct {
	U8 active; /* boolean for whether this function should even operate */
	U8 firstInactiveIdx; /* marks the first inactive element's index */
	U8 firstEmptyIdx; /* marks the first empty element's index */
	SysFP *sysFP; /* function that runs on these components */
	void *ecA;  /* components the above function operates on */
} Activity;

/* Systems are agnostic. They know nothing outside themselves. They merely react to their inbox contents. When they're done, they populate their outboxes however they're programmed to; the outside world can do whatever with it that they wish. */
typedef struct _System {
  U8           id;                /* this is needed to ensure messages are sent to the correct system */
	U8           active;            /* boolean for whether this syskem should even operate */
	U8           ecSz;              /* components are the same size in all of this system's activities */
  void        *swapPlaceholderP;  /* Avoids allocating a new placeholder every EC-swap. */
	Map         *cmpLocationMapP;   /* maps component IDs to an element in an array of CmpAddresses */
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

SysBasicFP sysBasicFuncs[] = {
	sysToggleActive
};

#endif
