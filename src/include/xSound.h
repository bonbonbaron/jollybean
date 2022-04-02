#ifndef SYS_Anim
#define SYS_Anim
#include "botox.h"

typedef struct {
} XAnimCompSrc; 

typedef struct {
} XAnimComp;

// Images
Error xAnimIniS(System *sP, void *sParamsP);
Error xAnimProcessMessage(System *sP, Message *msgP);

typedef struct {
  System system;
} XAnim;

extern System *sAnimP;
#endif
