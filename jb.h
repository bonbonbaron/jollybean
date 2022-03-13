#ifndef JB_
#define JB_

/* Basic */
#include "SDL.h"
#include "ecs.h"
#include "bt.h"

// Entity DNA
#include "quirk.h"
#include "personality.h"
#include "genome.h"
#include "seed.h"
#include "biome.h"

// Child systems
#include "xRender.h"
#include "xControl.h"
#include "xSystem.h"

/* Keys */
enumKeys_(CONTROL, POSITION, MOTION, ANIMATION, COLLISION, RENDER, JOLLYBEAN, N_COMPONENT_TYPES) ComponentType; 

#endif
