#ifndef JB_
#define JB_

/* Basic */
#include "SDL.h"
#include "ecs.h"
#include "bt.h"

// Child systems
#include "xRender.h"
#include "xControl.h"
#include "xParent.h"

// Entity DNA
#include "genes.h"
#include "personality.h"

/* Keys */
ENUM_KEYS_(CONTROL, POSITION, MOTION, ANIMATION, COLLISION, RENDER, JOLLYBEAN, N_SYS_TYPES) SystemKey; 

#endif
