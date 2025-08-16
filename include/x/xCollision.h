#ifndef SYS_Collision
#define SYS_Collision
#include "x/x.h"
#include "interface/interface.h"
#include "jb.h"

/* Let's think through the mutation aspect here.
 * We need something similar to animation: a map of arrays.
 * But instead of timing, it relies on a message from animation system to tell it when.
 */
typedef struct {
  Entity entity;   // Collider
  Key    type;     // Colliding elements react according to each other's type.
  Key    layer;
  Key    rectIdx;  // Rect index of Collidee (i swear i don't talk like this)
} Collision;

typedef struct {
  U8 nCollisionTypes;
  U8 *colissionA;
} CollisionPalette;

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
void xCollisionIniS(System *sP, void *sParamsP);
void xCollisionProcessMessage(System *sP, Message *msgP);
extern System *sCollisionP;
#endif
