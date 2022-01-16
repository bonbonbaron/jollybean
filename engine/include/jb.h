#ifndef JB_
#define JB_

/* Basic */
#include "SDL.h"
#include "ecs.h"
#include "bt.h"

// Entity DNA
#include "genome.h"
#include "biome.h"
#include "personality.h"
#include "seed.h"
#include "personality.h"

// Child systems
#include "xRender.h"
#include "xControl.h"
#include "xParent.h"

/* Keys */
enumKeys_(CONTROL, POSITION, MOTION, ANIMATION, COLLISION, RENDER, JOLLYBEAN, N_SYS_TYPES) SystemKey; 

#endif
