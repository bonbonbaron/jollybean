#include "ecs.h"

typedef struct {CmpHeader header; S8 x, y;} Velocity;
typedef struct {CmpHeader header; S16 x, y;} Position;

Error incVel(System *sP, Activity *aP, void *velA) {
	Velocity *vP, *vEndP;
	arrayIniPtrs(aP->ecA, (void**) &vP, (void**) &vEndP, -1);
	for (; vP < vEndP; vP++) {
		vP->x++;
		vP->y++;
	}
	return SUCCESS;
}

Error decVel(System *sP, Activity *aP, void *velA) {
	Velocity *vP, *vEndP;
	arrayIniPtrs(aP->ecA, (void**) &vP, (void**) &vEndP, -1);
	for (; vP < vEndP; vP++) {
		vP->x--;
		vP->y--;
	}
	return SUCCESS;
}

SysFP velFPA[] = { incVel, decVel, NULL };


typedef enum {T1, T2, T3, T4, T5, T_NONE} Trigger;
typedef enum {SCENE_1, SCENE_2} SceneType;
typedef enum _SystemKey {K_SYS_MOTION, NUM_SYSTEMS} SystemKey;
typedef enum { INCREASE_VELOCITY, DECREASE_VELOCITY, I_MTN_STOP_VEL, NUM_MTN_FUNCS} MotionFuncIdx;   
typedef enum {K_MOVE_UP, K_MOVE_DOWN, K_MOVE_LEFT, K_MOVE_RIGHT, K_MOVE_STOP, K_NUM_MOVES} MotionKey;
/* You're going to have VERY FEW responses defined for each system! Wow!!! */
/* Responses */
Response 
r1 = {K_SYS_MOTION, INCREASE_VELOCITY, K_MOVE_UP, NO_OUTPUT},
r2 = {K_SYS_MOTION, INCREASE_VELOCITY, K_MOVE_UP, NO_OUTPUT},
r3 = {K_SYS_MOTION, INCREASE_VELOCITY, K_MOVE_UP, NO_OUTPUT},
r4 = {K_SYS_MOTION, INCREASE_VELOCITY, K_MOVE_UP, NO_OUTPUT},
r5 = {K_SYS_MOTION, INCREASE_VELOCITY, K_MOVE_UP, NO_OUTPUT},
/* Response Sets */
*rs1[] = RS_(&r1, &r2), 
*rs2[] = RS_(&r2, &r3, &r4, &r5), 
/* Response Set Sequences */
**rsSeq1AA[] = RS_SEQ_(rs1, rs2),
**rsSeq2AA[] = RS_SEQ_(rs2);

/* Behaviors */
typedef HardCodedMap Behavior;
Behavior
b1 = BEHAVIOR_(
  SET_(T1, rsSeq1AA),
  SET_(T2, rsSeq2AA)
),
b2 = BEHAVIOR_(
  SET_(T5, rsSeq1AA),
  SET_(T3, rsSeq2AA),
  SET_(T4, rsSeq2AA),
  SET_(T2, rsSeq1AA),
  SET_(T1, rsSeq2AA)
),
b3 = BEHAVIOR_(
	SET_(T3, rsSeq2AA)
);


U32 getNullTermALen(void *array, U32 elemSz) {
	U32 i = 0;
	U8 **elemPP = (U8**) array;
	for (; *elemPP != NULL; elemPP += elemSz) 
		++i;
	return i;
}
	
/***************************************************/

Error mapIni(Map **mapPP, KeyValPair *keyValA) {
	Error e = SUCCESS;

	U32 arrayLen = getNullTermALen((void*) keyValA, sizeof(KeyValPair));

	if (arrayLen) {
		e = mapNew(mapPP, keyValA[0]._valSz, arrayLen);
		if (!e)
			for (U32 i = 0; !e && i < arrayLen; i++) 
				e = mapSet(*mapPP, keyValA[i].key, keyValA[i].valueP);
	} else
		*mapPP = NULL;

	return e;
}
	


void main() {
	//System *sP;
	//Error e = sysNew(&sP, 1, sizeof(Velocity), NUM_MTN_FUNCS); 
	//if (!e) {
	//	sysIni(sP, 1, velFPA, sizeof(Velocity));
	//}
	//if (!e) 
	//	printf("sys id = %d\nactive = %d\necSz %d\n", sP->id, sP->active, sP->ecSz);
	Map *mapP;
}

/* director ini algo 
 \* 0) make enum of velocity function indices
 \* 1) make set of responses
 \* 2) make sequence of response sets
 \* 3) make behavior: array of KeyValPairs of trigger-to-sequence mapping
 \* 4) make personality: KeyValPair of scene type to behaviors
 \* 5) make two personalities
 \* 6) make three entities; two with the same personality
 \* 7) make mapIni() to generate maps from key-value arrays
 \* 8) make macros for behaviors to make developers' lives easier
 * 8) histogram components
 * 9) validate personality target systems against available systems in histo
 * 10) validate personality response function indices against target systems' activity array lengths
 * 11) make personality inflatable; inflate only once
 * 12) allocate systems using (8)
 * 13) go through all personalities and put the system attribute indicated by trigger SCENE_START into the corresponding activity
 * 14) 
 *
 * */
