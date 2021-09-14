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
	MsgUrgency		 urgency;       /* speed at which message must reach its destination */
	Priority       priority;      /* priority which message command has over any existing component activity */
	void *paramsP;                /* Parameters for the target to pass to the cmd function... IF tgtECSType == SYSTEM. */
} Message;

// TODO: revamp sys.c to support an array of SysActivity elements instead of several disparate elements talking about the same thing.
typedef struct {
	U8 active; /* boolean for whether this function should even operate */
	U8 firstInactiveIdx; /* marks the first inactive element's index */
	U8 firstEmptyIdx; /* marks the first empty element's index */
	SysFP *sysFP; /* function that runs on these components */
	void *ecA;  /* components the above function operates on */
} Activity;

/* Systems are allocated. */
typedef struct _System {
  U8           id;                /* this is needed to ensure messages are sent to the correct system */
	U8           active;            /* boolean for whether this syskem should even operate */
	U8           ecSz;              /* components are the same size in all of this system's activities */
  void        *swapPlaceholderP;  /* An array to hold the place of swaps; avoids allocating a new placeholder every time. */
	Map         *cmpLocationMapP;   /* maps component IDs to an element in an array of CmpAddresses */
	Activity    *activityA;
	Message     *inboxA;
	Message     *outboxA;
} System;

typedef enum {
	SYS_TOGGLE_ACTIVE,
	_MAX_SYSTEMWIDE_CMD_ID
} SysCmdID;

SysBasicFP sysBasicFuncs[];

#endif
