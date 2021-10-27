#include "sRender.h"
#include "SDL.h"
#include <unistd.h>

typedef struct {CmpHeader header; S8 x, y;} Velocity;

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


// Storing a component in memory:
#define MTN_C_(...) {CMP_HEADER_(MOTION), {{__VA_ARGS__}}}
#define POS_C_(...) {CMP_HEADER_(POSITION), __VA_ARGS__}
/* Examples */
Key key1 = 1;
Motion
	m1 = MTN_C_(1, 2, 3, 4, 5),
	m2 = MTN_C_(6, 7, 8, 9, 10),
	m3 = MTN_C_(11, 12, 13, 14, 15);

#define KV_(key_, val_) {key_, (const typeof(val_)) val_}
NONBINARY_GENE_(mario, Motion, key1, KV_(1, &m1), KV_(2, &m2), KV_(3, &m3));
BINARY_GENE_(mario, Motion, key1, 0xFF, 1, MTN_C_(1, 2, 3, 4, 5), MTN_C_(6, 7, 8, 9, 10));
//BINARY_GENE_(mario, Motion, key1, 0xFF, 1, m1, m2);
UNARY_GENE_(mario, Position, POS_C_(58, 47));
//TODO: make another binary and unary that can take a reference to a separate variable.

/* Macro for grabbing the type header handle of the gene. getGene() casts that header
 * to the appropriate gene type for further manipulation. */

/* You're going to have VERY FEW responses defined for each system! Wow!!! */
/**************************************************************/
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

EntitySeed marioES = {&p1, {GENE_(mario, Motion, B), GENE_(mario, Motion, N), GENE_(mario, Position, U)}};
// Position will be mapped by scene ID. However, that multiplies the number of bytes allocated to FlagInfo dramatically. Therefore, we need to figure out a more efficient map storage in the long run.  

U16 palette[16] = {0xF00F, 0xABCD, 0x1234, 0x4321, 0x0FF0, 0xA81F, 0x0F0F, 0x0000, 0xF00F, 0x0000, 0xF00F, 0x0000, 0xF00F, 0x0000, 0xF00F, 0x0000}; 


Image img;

int main(int argc, char **argv) {
	Error e = inflate(&inflatableBigger);

	// If inflation worked, give tileset its data and make color map full-image. 

	if (!e) {
		e = imageGen(&img);
		printf("imageGen error: %d\n", e);
	}

	/************* SDL2 example starts here! ***************/
	if (SDL_Init(SDL_INIT_VIDEO) != SUCCESS)
		return EXIT_FAILURE;

	SDL_Window *windowP = SDL_CreateWindow("Hello world!", 100, 100, 620, 387, SDL_WINDOW_SHOWN);
	if (!windowP)
		return EXIT_FAILURE;

	SDL_Renderer *rendererP = SDL_CreateRenderer(windowP, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!rendererP) {
		SDL_DestroyWindow(windowP);
		SDL_Quit();
		return EXIT_FAILURE;
	}

	SDL_Surface *surfaceP = SDL_CreateRGBSurfaceFrom((void*) img.pixelA,
			                                             img.w,
																									 img.h,
																									 img.bpp,
																									 img.pitch,
																									 0xF800,
																									 0x07E0,
																									 0x001F,
																									 0xFFFF);

	if (!surfaceP) 
		printf("failed to make w: %d, h: %d, bpp: %d, pitch:%d surface: %s\n", img.w, img.h, img.bpp, img.pitch, SDL_GetError());
	else 
		sleep(3);
		
	SDL_DestroyRenderer(rendererP);
	SDL_DestroyWindow(windowP);

	/*******************************************************/

	
	jbFree((void**) &inflatableBigger.inflatedDataP);
	imageClr(&img);

	return e;
}
