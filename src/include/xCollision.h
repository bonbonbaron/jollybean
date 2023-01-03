#ifndef SYS_Collision
#define SYS_Collision
#include "jb.h"

// Dummy struct to keep macro X_ happy.
typedef struct {
} XCollisionCompSrc; 

typedef Collision XCollisionComp;

typedef struct {
  System  system;
  Entity  cameraEntity;
  U8      cameraIsMoving;
  U16    *collisionA;          // Keeps track of all current collisions.
  U16     worldWidth;
  U16     worldHeight;
  Rect_  *rectA;               // This points to the master's rectangle map's mapA array.
  U8     *gridA;
} XCollision;

// Functions
Error xCollisionIniS(System *sP, void *sParamsP);
Error xCollisionProcessMessage(System *sP, Message *msgP);
extern System *sCollisionP;
#endif
