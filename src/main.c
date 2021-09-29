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
typedef enum {UNARY, BINARY, NONBINARY} _GeneType;

typedef struct {   /* 15 bytes */
	U8          type;
	U8          _elemSz;
	U8          _nElems;
	Key        *keyP;
	Map        *mapP;       /* defaults to NULL to prevent copies */
	KeyValPair *kvAP;        /* automagically generated via macro */
} NonbinaryGene;

#define PADDING (sizeof(NonbinaryGene) - (2 * sizeof(void*) + (3 * sizeof(U8))))
typedef struct {  /* 11 bytes + padding */
	U8 type;
	U8 mask;        
	U8 expVal;       /* the masked data must equal expVal to index 1. Otherwise 0. */
	U8 _padding[PADDING];
	U8 *tgtP;
	void *binaryA;   /* automatically generated by macro */
} BinaryGene;
#undef PADDING

#define PADDING (sizeof(NonbinaryGene) - (sizeof(void*) + (sizeof(U8))))
typedef struct {  /* 5 bytes + padding */
	U8 type;
	U8 _padding[PADDING];
	void *unaryP;
} UnaryGene;
#undef PADDING

 /*
 * ******* WHAT HAPPENS IN GENE LOOP *********
 * What happens in gene loop? Each map/binarryArray/voidPtr is created, and its proper element 
 * is selected with its key and sent to the system it belongs in. The allocated memory stays in
 * the seed. The seed will be pointed at later.
 *
 * ******* ARRAY OF POINTERS TO ENTITY SEEDS ********
 * How does the system's resident component change to something else? When such a SET command is
 * sent to the system handling the entities, that entity no longer has a pointer to its seed. So
 * an array is made to hold pointers to the entities' seeds. Worry about that later.
 * Then just give some structs padding for neat looping.
 */

/* GenePool.type determines which function in the below array accesses the data. */
/* Unary gene-selection */

#if 0
void* getUnaryGene(void *gP) {
	return gpP->data.unaryP;
}

/* Binary gene-selection */
//TODO make hard-coded arrays so you can call arrayGetVoidPointer on it.
void* getBinaryGene(void *gP) {
	return arrayGetVoidElemPtr(gpP->data.binaryA, ((*gpP->accessor.binary.tgtP & gpP->accessor.binary.mask) == gpP->accessor.binary.expVal));
}

/* Nonbinary gene-selection */
void* getNonbinaryGene(void *gP) {
	Error e = SUCCESS;
	void *resultP = NULL;

	if (gpP->nonbinaryHCM.mapP == NULL) 
		e = mapIni(NULL, &gpP->nonbinaryHCM);
	if (!e)
		resultP = mapGet(gpP->nonbinaryHCM.mapP, *gpP->accessor.nonbinaryKeyP);

	return resultP;
}

typedef void* (*GetGeneFP)(GenePool *gpP);
GetGeneFP getGeneFPA[] = {getUnaryGene, getBinaryGene, getNonbinaryGene};

void* getGene(GenePool *gpP) {
	/* No need for internal NULL-checks since wrapper does it here. */
	if (gpP != NULL)
		return (*getGeneFPA[gpP->type])(gpP);
	return NULL;
}
#endif

// Storing a component in memory:
#define MTN_C_(...) {CMP_HEADER_(MOTION), {{__VA_ARGS__}}}
#define POS_C_(...) {CMP_HEADER_(POSITION), __VA_ARGS__}
#define CMP_HEADER_(sysKey) {0, sysKey}

Motion
	m1 = MTN_C_(1, 2, 3, 4, 5),
	m2 = MTN_C_(6, 7, 8, 9, 10),
	m3 = MTN_C_(11, 12, 13, 14, 15);

Key key1 = 1;

/* Gene pool storage macros */
#if 0
#define BINARY_GENE_POOL(mask_, expval_, dataName, binaryA) {\
	.type = BINARY, \
	.data.binary = {\
		.keyP = &globalStateKey_,\
		.hcm = \
	} \
}
#endif

/* Unary Example */
/* Binary Example */
/* Nonbinary Example */
/* So genes will indeed exist in other folders just like I originally envisioned.
 * Everybody who wants one can grab it. */
#if 0
typedef struct {   /* 15 bytes */
	U8          type;
	U8          _elemSz;
	U8          _nElems;
	Key        *keyP;
	Map        *mapP;       /* defaults to NULL to prevent copies */
	KeyValPair *kvAP;        /* automagically generated via macro */
} NonbinaryGene;
#endif

#define UNARY_GENE_(name_, cmpType_, ...) \
	cmpType_ name_##UVal = __VA_ARGS__;\
	UnaryGene name_##cmpType_##UGene = {\
		.type = UNARY,\
		.unaryP = &name_##UVal};

#define BINARY_GENE_(name_, cmpType_, tgtVal_, mask_, expVal_, ...) \
	cmpType_ name_##BA[2] = {__VA_ARGS__};\
	BinaryGene name_##cmpType_##BGene = {\
		.type    = BINARY,\
		.mask    = mask_,\
		.expVal  = expVal_,\
		.tgtP    = &tgtVal_,\
		.binaryA = name_##BA};

#define NONBINARY_GENE_(name_, cmpType_, globalStateKey_, ...) \
	KeyValPair name_##NBKVA[] = {__VA_ARGS__};\
	NonbinaryGene name_##cmpType_##NBGene  = {\
		.type    = NONBINARY,\
		._elemSz = sizeof(cmpType_),\
		._nElems = NUM_ARGS_(KeyValPair, __VA_ARGS__),\
		.keyP    = &globalStateKey_,\
		.mapP    = NULL,\
		.kvAP    = name_##NBKVA};
NONBINARY_GENE_(mario, Motion, key1, {1, &m1}, {2, &m2}, {3, &m3});
BINARY_GENE_(mario, Motion, key1, 0xFF, 1, MTN_C_(1, 2, 3, 4, 5), MTN_C_(6, 7, 8, 9, 10));
UNARY_GENE_(mario, Position, POS_C_(58, 47));

//HardCodedMap hcm = HARD_CODED_MAP_(Motion, {1, &m1}, {2, &m2}, {3, &m3});
//GenePool g1 = NONBINARY_GENE_POOL_(Motion, key1, {1, &m1}, {2, &m2}, {3, &m3}); 


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

// Problem: Some genes vary from scene to scene, such as position. These will have to be blank in genes. Instead, they'll be dynamic genes, populated by the scene struct itself. 
int main(int argc, char **argv) {
	printf("size of nonbinary gene: %d\n", sizeof(NonbinaryGene));
	printf("size of binary gene: %d\n", sizeof(BinaryGene));
	printf("size of unary gene: %d\n", sizeof(UnaryGene));
	printf("mario non-binary gene is at 0x%08x\n", (U32) &marioMotionNBGene);
	printf("mario binary gene is at 0x%08x\n", (U32) &marioMotionBGene);
	printf("mario's position is (%d, %d).\n", ((Position*) marioPositionUGene.unaryP)->x, ((Position*) marioPositionUGene.unaryP)->y);


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