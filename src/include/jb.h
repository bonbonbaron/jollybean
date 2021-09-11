#ifndef JB_H
#define JB_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define N_FLAG_BYTES (1)  /* This times 8 is the number of items JB's hash map can hold. Increase as necessary. */
#define N_FLAG_BITS (8 * N_FLAG_BYTES)

typedef unsigned char U8;
typedef char S8;
typedef unsigned short U16;
typedef short S16;
typedef unsigned int U32;
typedef int S32;

#include "errors.h"

/* Basic utils */
Error jbAlloc(void **voidPP, U8 elemSz, U8 nElems);
void  jbFree(void **voidPP);
extern U8 bitCountLUT[];
extern U8 byteIdxLUT[];

/* Arrays */
Error arrayNew(void **arryPP, U32 elemSz, U32 nElems);
void arrayDel(void **arryPP);
U32 arrayGetNElems(const void *arryP);
U32 arrayGetElemSz(const void *arryP);

/* Maps */
typedef struct {
	U8 prevBitCount;
	U8 flags;
} FlagInfo;

typedef struct {
	FlagInfo flagA[N_FLAG_BYTES];  /* "A" means "Array" for JB's naming standards */
	void  *mapA;  
} Map;

Error mapNew(Map **mapPP, const U8 elemSz, const U16 nElems);
void  mapDel(Map **mapPP);
Error mapSet(Map *mapP, const U8 key, const void *valP);
void* mapGet(const Map *mapP, const U8 key);

/* System */
struct _System;
typedef Error (*SysFP)(struct _System *sysP, void *cmpA);
typedef Error (*SysIniFP)(struct _System *sysP, void *cmpA);

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
	U8 jBaseIdx;
	U8 jInnerIdx;
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
	SysFP *sysFP; /* function that runs on these components */
	void *cmpsA;  /* components the above function operates on */
} SysActivity;

/* Systems are allocated. */
typedef struct _System {
	U8           id;
	U8           active;            /* boolean for whether this syskem should even operate */
	U8           cmpSz;             /* components are the same size in all of this system's activities */
	Map         *cmpLocationMapP;   /* maps component IDs to an element in an array of CmpAddresses */
	SysActivity *sysActivityA;
	Message     *inboxA;
	Message     *outboxA;
} System;

/* Histograms */
Error histoU8New(U32 **histoPP, const U8 *srcA, const U32 maxVal);
void histoU8Del(U32 **histoPP);

/* Components are stored in memory. */
typedef struct {
	U8 sysKey;
	U8 cmpSz;
	void *cmpP;
	Map *cmpMapP;  /* For things like animation, this is crucial. We won't copy entire maps to a system; just the initial values. */
} Component;

typedef struct {
	Component *componentsA;
} EntitySeed;

typedef U8 Entity;  /* HAH! That's all an entity is!? A key to its components like we're a key to our atoms! */

#endif
