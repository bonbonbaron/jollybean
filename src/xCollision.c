#include "xCollision.h"

//======================================================
// Initialize Collision's system.
//======================================================
Error xCollisionIniSys(System *sP, void *sParamsP) {
  return SUCCESS;
	unused_(sParamsP);
  unused_(sP);
}

//======================================================
// Initialize xCollision's components, which are Images.
//======================================================
Error xCollisionIniComp(System *sP, void *compDataP, void *compDataSrcP) {
	if (!sP || !compDataP || !compDataSrcP)
		return E_BAD_ARGS;

  Error e = SUCCESS;
	XCollision *xCollisionSysP = (XCollision*) sP;
	XCollisionComp *cP = (XCollisionComp*) compDataP;
  XCollisionCompSrc *imgP = (XCollisionCompSrc*) compDataSrcP;

	return e;
}

Error xCollisionProcessMessage(System *sP, Message *msgP) {
	unused_(sP);
	unused_(msgP);
	return SUCCESS;
}

XClrFuncDef_(Collision) {
  unused_(sP);
  return SUCCESS;
}

XGetShareFuncDef_(Collision) {
  XCollision *xCollisionSysP = (XCollision*) sP;
  Error e = SUCCESS;
  return e;
}

//======================================================
// Collision activity
//======================================================
Error xCollisionRun(System *sP) {
	Error e = SUCCESS;

	XCollisionComp *cP = (XCollisionComp*) sP->cF;
	XCollisionComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);

  for (; cP < cEndP; cP++) {
    // do something on each element here
  }

  // TODO search coarse array
  // TODO search medium array
  // TODO search fine array
  // TODO search static grid

	return e;
}

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(Collision, 1, FLAGS_HERE);

typedef struct {
} QuadTree;

U16 *collTypeA;  // This keeps track of the kinds of collisions everyone currently has. For messaging, cmd = COLLISION_[STARTED|ENDED]; arg = TYPE.


/* Here we are, the collision system. I've been looking forward to this.
 *
 * An article I read the other day inspired me to take a 3-array approach: coarse, medium, and fine.  
 * One optimization it has is, for each row, check whether the row is NULL. But this won't be the case with
 * a straight-shot array I have in mind. I'll need something else, and I also don't want it to check every iteration.
 *
 * Each QtNode has index to first child in the next-level array. But what's more, I don't want to store big guys in 
 * every fine node they span; they should only inhabit nodes they're smaller than (to an extent). 
 *
 * (A) Does source author reference rectangles externally? If so, I'm in good shape, especially if by indices.
 * (B) Collision *components* aren't actually in either set of collision grids; they're used to update it.
 * (C) If I can get the collision component to 4 bytes, *perfect*.
 * (D) I want a single-array speed boost.
 * (E) Events COLLISION_BEGAN and COLLISION_ENDED should respond to motion events. In order to do so, it must keep a 
 *     list of ongoing collisions. Don't iterate through that list; you must be able to key into it directly with the
 *     entity who moved. Should be a bitset since every entity has a unique number. This happens in the message processor (MP).
 *     How do I decouple the MP from the motion system? I need to avoid saying "This object moved." Instead, I need to have it
 *     listen for a simple [DE]ACTIVATED message. This tells it a component is now worth checking. 
 * (F) That would be nice if I only checked on moving sprites rather than querying every node. But the problem arises when *everbody's*
 *     moving; I'd then have a horribly inefficient random-access pattern. So I have to pick and choose. However, I could have my cake and
 *     eat it too: I can switch the system's sRun() to something different depending on the situation! 
 *
 */
