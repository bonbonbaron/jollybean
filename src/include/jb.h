#ifndef JB_
#define JB_

// Core
#include "botox.h"
#include "xMaster.h"
#include "xGo.h"

// Child systems
#include "xRender.h"
#include "xControl.h"
#include "xAnim.h"
#include "xTimer.h"
#include "xMotionTrans.h"
#include "xCollision.h"

// Keys  (hopefully obsolete later)
// MASTER is 1, GO is 2
typedef enum {CONTROL = 3, POSITION, MOTION_TRAN, ANIMATION, COLLISION, RENDER, TIMER, JOLLYBEAN, N_SYSTEM_TYPES} SystemType; 

#endif
