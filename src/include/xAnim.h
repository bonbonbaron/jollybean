#ifndef SYS_Anim
#define SYS_Anim
#include "jb.h"
#include "body.h"
typedef struct {
} XAnimCompSrc; 

typedef Animation XAnimComp;

// Images
Error xAnimIniS(System *sP, void *sParamsP);
Error xAnimProcessMessage(System *sP, Message *msgP);

typedef struct {
  System system;
  Rect_ *rectOffsetF;
} XAnim;

extern System *sAnimP;
#endif
