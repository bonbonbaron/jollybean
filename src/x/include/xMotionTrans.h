#ifndef SYS_MotionTrans
#define SYS_MotionTrans
#include "interface.h"
#include "jb.h"

// Have a #ifdef here. You can do this version for vanilla JB. 
// Vectorized JB should keep this aligned to the original dest rect shared array.
typedef struct {
  S8 x;
  S8 y;
} Velocity;

typedef Velocity XMotionTransMutation;

typedef struct {
  Velocity velocity;
  Key rectIdx;
} XMotionTransComp;  

// Images
Error xMotionTransIniS(System *sP, void *sParamsP);
Error xMotionTransProcessMessage(System *sP, Message *msgP);

typedef struct {
  System system;
  Rect_ *rectA;
} XMotionTrans;

extern System *sMotionTransP;
#endif
