#ifndef SYS_MotionTrans
#define SYS_MotionTrans
#include "botox.h"
#include "yoyoInterface.h"

typedef struct {
} XMotionTransCompSrc; 

typedef struct {
  S8 x;
  S8 y;
  Rect_ *shareRectP;
} XMotionTransComp;

// Images
Error xMotionTransIniS(System *sP, void *sParamsP);
Error xMotionTransProcessMessage(System *sP, Message *msgP);

typedef struct {
  System system;
} XMotionTrans;

extern System *sMotionTransP;
#endif
