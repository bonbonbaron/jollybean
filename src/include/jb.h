#ifndef JB_
#define JB_

// Core
#include "botox.h"
// Core jb objects
#include "body.h"
// Child systems
#include "xRender.h"
#if 0  // add back in later after fixing mess
#include "xControl.h"
#include "xAnim.h"
#include "xTimer.h"
#include "xMotionTrans.h"
#include "xCollision.h"
#endif

// Keys  (hopefully obsolete later)
// MASTER is 1, GO is 2
typedef enum{WINDOW_GENE_TYPE = 1, RENDERER_GENE_TYPE, DST_RECT, N_MASTER_GENES} ShareType;
// Make sure you don't get rid of the 3! 1 and 2 belong to Master and Go.
typedef enum {RENDER = 3, TIMER, COLLISION, N_SYSTEM_TYPES} SystemType; 

#endif
