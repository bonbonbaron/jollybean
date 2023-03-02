#ifndef JB_
#define JB_

// Core
#include "data.h"
#include "x.h"
// Core jb objects
#include "interface.h"
// Child systems
#include "xMaster.h"
#include "xRender.h"
#include "xAnim.h"
#if 0  // add back in later after fixing mess
#include "xControl.h"
#include "xTimer.h"
#include "xMotionTrans.h"
#include "xCollision.h"
#endif

// Keys  (hopefully obsolete later)
// MASTER is 1, GO is 2
typedef enum {UPDATE_RECT} MessageCommand;
typedef enum {MASTER = 1, RENDER, ANIMATION, TIMER, COLLISION, N_SYSTEM_TYPES} SystemType; 
// Make sure you don't get rid of the 3! 2 belongs to Master.
typedef enum{WINDOW_GENE_TYPE = 1, RENDERER_GENE_TYPE, SRC_RECT, RECT_OFFSET, DST_RECT, N_MASTER_GENES} ShareType;

#endif
