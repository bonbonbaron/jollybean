#include "ecs.h"
#include "genes.h"
#include "SDL.h"

// Child systems
#include "xRender.h"
#include "xControl.h"
#include "xParent.h"

/* Keys */
ENUM_KEYS_(CONTROL, POSITION, MOTION, ANIMATION, COLLISION, RENDER, JOLLYBEAN, N_SYS_TYPES) SystemKey; 
