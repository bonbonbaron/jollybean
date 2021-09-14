#ifndef ECS_H
#define ECS_H
#include "data.h"
#include "errors.h"

/* This is how every system increments the component pointer. */
#define INCR_CMP_P(ptr, stepSz) (((U8*) ptr) += stepSz)
#define INI_CMP_P(ptr) (((U8*) ptr) + sizeof(Entity))

/* Entity */
typedef struct {
	Component *componentsA;
} EntitySeed;

typedef U8 Entity;  /* HAH! That's all an entity is!? A key to its components like we're a key to our atoms! */

/* Component */
typedef struct {
	U8 sysKey;
	U8 cmpDataSz;  /* this size does not include the Entity byte */
	void *cmpDataP;
	Map *cmpMapP;  /* For things like animation, this is crucial. We won't copy entire maps to a system; just the cmps' init vals. */
} ComponentSeed;

typedef struct {
	Entity owner;
	void *data;  /* actual data component entails, like Velocity */
} Component;

/* System */
struct _System;
typedef Error (*SysFP)(struct _System *sysP, void *cmpA);
typedef Error (*SysIniFP)(struct _System *sysP, void *cmpA);
typedef void (*SysBasicFP)(System *sP);

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
	SYSTEMWIDE_CMD,
	REPEATING_CMP_CMD,
	ONE_OFF_CMP_CMD
} CmdType;

typedef struct {
	U8 activityIdx;
	U8 cmpIdx;
} CmpLocation;

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
	void *cmpsA;  /* components the above function operates on */
} Activity;

/* Systems are allocated. */
typedef struct _System {
	U8           active;            /* boolean for whether this syskem should even operate */
	U8           cmpStepSz;             /* components are the same size in all of this system's activities */
	Map         *cmpLocationMapP;   /* maps component IDs to an element in an array of CmpAddresses */
	SysActivity *activityA;
	Message     *inboxA;
	Message     *outboxA;
} System;

typedef enum {
	SYS_TOGGLE_ACTIVE,
	_MAX_SYSTEMWIDE_CMD_ID
} SysCmdID;

SysBasicFP sysBasicFuncs[] = {
	sysToggleActive
};

#endif
