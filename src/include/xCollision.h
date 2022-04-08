#ifndef SYS_Collision
#define SYS_Collision
#include "botox.h"

typedef struct {
} XCollisionCompSrc; 

typedef struct {
} XCollisionComp;

// Images
Error xCollisionIniS(System *sP, void *sParamsP);
Error xCollisionProcessMessage(System *sP, Message *msgP);

typedef struct {
  System system;
} XCollision;

extern System *sCollisionP;
#endif
