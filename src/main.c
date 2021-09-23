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
typedef enum {DEFAULT = 1, SCENE_1, SCENE_2} SceneType;
typedef enum _SystemKey {MOTION, ANIMATION, IMAGE, RENDERING, NUM_SYSTEMS} SystemKey;
typedef enum {INCREASE_VELOCITY, DECREASE_VELOCITY, I_MTN_STOP_VEL, NUM_MTN_FUNCS} MotionFuncIdx;   
typedef enum {DONT_SWITCH, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT, MOVE_STOP, NUM_MOVES} MotionKey;
/* You're going to have VERY FEW responses defined for each system! Wow!!! */
/**************************************************************/
Response 
/* Responses */
r1 = {MOTION, INCREASE_VELOCITY, DONT_SWITCH, NO_OUTPUT},
r2 = {MOTION, INCREASE_VELOCITY, MOVE_UP, NO_OUTPUT},
r3 = {MOTION, INCREASE_VELOCITY, MOVE_UP, NO_OUTPUT},
r4 = {MOTION, INCREASE_VELOCITY, MOVE_DOWN, NO_OUTPUT},
r5 = {MOTION, INCREASE_VELOCITY, DONT_SWITCH, NO_OUTPUT},
/* Response Sets */
*rs1[] = RS_(&r1, &r2), 
*rs2[] = RS_(&r2, &r4, &r5), 
/* Response Set Sequences */
**takingOutTheTrash[] = RS_SEQ_(rs1, rs2),
**sayingHello[] = RS_SEQ_(rs2);
/* Behaviors */
Behavior
b1 = BEHAVIOR_(
	RESPOND_TO_(T1) BY_(takingOutTheTrash),
	RESPOND_TO_(T2) BY_(sayingHello)
),
b2 = BEHAVIOR_(
	RESPOND_TO_(T1) BY_(takingOutTheTrash),
	RESPOND_TO_(T5) BY_(sayingHello),
	RESPOND_TO_(T3) BY_(sayingHello),
	RESPOND_TO_(T2) BY_(takingOutTheTrash),
	RESPOND_TO_(T4) BY_(sayingHello)
),
b3 = BEHAVIOR_(
	RESPOND_TO_(T3) BY_(takingOutTheTrash)
);
/* Personalities */
Personality
p1 = PERSONALITY_(
  IF_SCENE_TYPE_IS_(SCENE_1) BEHAVE_AS_(b1),
	IF_SCENE_TYPE_IS_(SCENE_2) BEHAVE_AS_(b2),
	OTHERWISE_BEHAVE_AS_(b3)
),
p2 = PERSONALITY_(
	IF_SCENE_TYPE_IS_(SCENE_2) BEHAVE_AS_(b2),
	OTHERWISE_BEHAVE_AS_(b1)
);

#if 0
U32 getNullTermALen(void *array, U32 elemSz) {
	U32 i = 0;
	U8 **elemPP = (U8**) array;
	for (; *elemPP != NULL; elemPP += elemSz) 
		++i;
	return i;
}
#endif	

	


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
