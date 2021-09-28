#include "ecs.h"

typedef struct {CmpHeader header; S8 x, y;} Velocity;

#define ENUM_KEYS_(first, ...) typedef enum {first = 1, __VA_ARGS__} 
#define ENUM_INDICES_(...) typedef enum {__VA_ARGS__} 
/* Keys */
ENUM_KEYS_(DEFAULT, SCENE_TYPE_1, SCENE_TYPE_2, N_SCENE_TYPES) SceneType;
ENUM_KEYS_(POSITION, MOTION, ANIMATION, COLLISION, IMAGE, RENDERING, N_SYSTEMS) SystemKey; 
ENUM_KEYS_(T1, T2, T3, T4, T5, N_TRIGGERS) Trigger;
ENUM_KEYS_(DONT_SWITCH, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT, MOVE_STOP, NUM_MOVES) MotionKey;
/* Indices */
ENUM_INDICES_(INCREASE_VELOCITY, DECREASE_VELOCITY, I_MTN_STOP_VEL, NUM_MTN_FUNCS) MotionFuncIdx;   

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


// TODO try the hard way first. Then think of what could be easier and less error-prone.
// Do this for the following:
// [X] Personalities
// [X] Genes      <--- Remember, each C will be maps of Cs; 
// [X] Entities        Also, some Cs will be inflatable.
// [ ] Scenes     <--- this determines systems from entities' behaviors
// [ ] Systems

/* Genes */
// individual component will look like...
typedef struct {
	CmpHeader header;
	union {
		/* Translation */
		struct {
			S8 vx;
			S8 vy;
			S8 vMax;
			U8 ax;
			U8 ay;
		} translation;
		/* Rotation  */
		struct {
			U8 radius;
			S8 radVel;
			S8 radVelMax;
			S8 radAcc;
		} rotation;
	} data;
} Motion;

typedef struct {
	CmpHeader header;
	U16 x;
	U16 y;
} Position;

/* genePool.type is binary. It determines whether a gene is selected from the pool by 
 * querying one or more flags with a mask, which outputs 0 or 1 to index a binary array;
 * or simply by using an entire [Key] element from the game state to access a gene map. */
typedef enum {UNARY, BINARY, NONBINARY} GenePoolType;
typedef struct {
	U8 type;  /* Forcing one-byte storage instead of letting GenePoolType expand ot 4 */
	union {
		struct {
			void *dataP;
		} unary;
		struct {
			U8 mask;         
			U8 expVal;       /* the masked data must equal expVal to index 1. Otherwise 0. */
			U8 *dataP;
			void *binaryPA[];
		} binary;
		struct {
			Key *keyP;
			HardCodedMap *hcmP;
		} nonbinary;
	} data;
} GenePool;

/* GenePool.type determines which function in the below array accesses the data. */
/* Unary gene-selection */
void* getUnaryGene(GenePool *gpP) {
	return gpP->data.unary.dataP;
}

/* Binary gene-selection */
void* getBinaryGene(GenePool *gpP) {
	return &gpP->data.binary.binaryPA[((*gpP->data.binary.dataP & gpP->data.binary.mask) == gpP->data.binary.mask)];
}

/* Nonbinary gene-selection */
void* getNonbinaryGene(GenePool *gpP) {
	Error e = SUCCESS;
	void *resultP = NULL;

	if (gpP->data.nonbinary.hcmP->mapP == NULL) 
		e = mapIni(NULL, gpP->data.nonbinary.hcmP);
	if (!e)
		resultP = mapGet(gpP->data.nonbinary.hcmP->mapP, *gpP->data.nonbinary.keyP);

	return resultP;
}

typedef void* (*GetGeneFP)(GenePool *gpP);
GetGeneFP getGeneFPA[] = {getUnaryGene, getBinaryGene, getNonbinaryGene};

void* getGene(GenePool *gpP) {
	/* No need for internal NULL-checks when wrapper does it here. */
	if (gpP != NULL)
		return (*getGeneFPA[gpP->type])(gpP);
	return NULL;
}



// So if I wanted to store a component like that in memory...
#define MOTION_COMPONENT_(...) {CMP_HEADER_(MOTION), {{__VA_ARGS__}}}
#define POSITION_COMPONENT_(...) {CMP_HEADER_(POSITION), __VA_ARGS__}
#define CMP_HEADER_(sysKey) {0, sysKey}

Motion 
	gremlinMotion = MOTION_COMPONENT_(1, 2, 3, 4, 5),
	marioMotion = MOTION_COMPONENT_(59, 3, 15, 0, 0);

Position 
	gremPos = POSITION_COMPONENT_(72, 70),
	marioPos = POSITION_COMPONENT_(89, 5),
	bowserPos = POSITION_COMPONENT_(2, 3);

#define ADD_(name) (CmpHeader*) &name
#define GENES_(name) *name[]
CmpHeader 
	GENES_(marioGenes) = {
		ADD_(marioPos),
		ADD_(marioMotion)},
	GENES_(bowserGenes) = {
		ADD_(bowserPos),
		ADD_(gremlinMotion)};

/* Game state maps to genes. So this is similar to personality. */
/* Key is enum of game state. Internally, JB checks if the key'th bit in the game state bytes is 1.
 * Some genes will be conditional; not all. Therefore, every gene needs a condition U8 or U16; better typedef GameStateCondition.
 * The conditional genes need a fall-back option; maybe even multiple. Hell, that's literally a map. I'll make a U8 map to spare the RAM.
 * Multiple conditions may be OR'ed together. 
 * */
#define IF_GAME_STATE(key) {key, 
#define MANIFEST_AS(val) (void*) &val}
#define OTHERWISE_MANIFEST_AS_(val) {DEFAULT, (void*) &val}


/* You're going to have VERY FEW responses defined for each system! Wow!!! */
/**************************************************************/
#define RESPONSE_SET_(name) *name[]
#define RESPONSE_SEQUENCE_(name) **name[]
Response 
	r1 = {MOTION, INCREASE_VELOCITY, DONT_SWITCH, NO_OUTPUT},
	r2 = {MOTION, INCREASE_VELOCITY, MOVE_UP, NO_OUTPUT},
	r3 = {MOTION, INCREASE_VELOCITY, MOVE_UP, NO_OUTPUT},
	r4 = {MOTION, INCREASE_VELOCITY, MOVE_DOWN, NO_OUTPUT},
	r5 = {MOTION, INCREASE_VELOCITY, DONT_SWITCH, NO_OUTPUT},

	RESPONSE_SET_(rs1) = RS_(&r1, &r2), 
	RESPONSE_SET_(rs2) = RS_(&r2, &r4, &r5), 

	RESPONSE_SEQUENCE_(takingOutTheTrash) = RS_SEQ_(rs1, rs2),
	RESPONSE_SEQUENCE_(sayingHello) = RS_SEQ_(rs2);

Behavior
	b1 = BEHAVIOR_(
		RESPOND_TO_(T1) BY_(takingOutTheTrash),
		RESPOND_TO_(T2) BY_(sayingHello)),
	b2 = BEHAVIOR_(
		RESPOND_TO_(T1) BY_(takingOutTheTrash),
		RESPOND_TO_(T5) BY_(sayingHello),
		RESPOND_TO_(T3) BY_(sayingHello),
		RESPOND_TO_(T2) BY_(takingOutTheTrash),
		RESPOND_TO_(T4) BY_(sayingHello)),
	b3 = BEHAVIOR_(
		RESPOND_TO_(T3) BY_(takingOutTheTrash));

Personality
	p1 = PERSONALITY_(
		IF_SCENE_TYPE_IS_(SCENE_TYPE_1) BEHAVE_AS_(b1),
		IF_SCENE_TYPE_IS_(SCENE_TYPE_2) BEHAVE_AS_(b2),
		OTHERWISE_BEHAVE_AS_(b3)),
	p2 = PERSONALITY_(
		IF_SCENE_TYPE_IS_(SCENE_TYPE_2) BEHAVE_AS_(b2),
		OTHERWISE_BEHAVE_AS_(b1));

EntitySeed
	marioSeed = {&p1, marioGenes},
	bowserSeed = {&p2, bowserGenes};

// Problem: Some genes vary from scene to scene, such as position. These will have to be blank in genes. Instead, they'll be dynamic genes, populated by the scene struct itself. 
int main(int argc, char **argv) {
	printf("size of GenePool: %d\n", sizeof(GenePool));

	return 0;
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
 \* 8) make macros for personalities and their constituents to make game devs' lives easier
 * 11) make macros for making all kinds of data
 * 12) lay foundation for game states' interplay with components/genes
 * 12) histogram components; at the same time, validate personality target systems against available systems in histo
 * 13) validate personality response function indices against target systems' activity array lengths
 * 14) validate that responses' universal keys return non-NULLs from entities' components
 * 15) make personality inflatable; inflate only once
 * 16) allocate systems using the above histo
 * 17) go through all personalities and put the system attribute indicated by trigger SCENE_START into the corresponding activity
 * 18) make system seeds for position (which'll just print the position) and motion (which'll message position)
 *
 * */
