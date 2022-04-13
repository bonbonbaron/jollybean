#include "xCollision.h"

// Based on the "Performance Comparisons" link, we use brute force until
// the number of moving elements exceeds 200. When it does, we switch to the box-
// intersect algorithm found in link to the paper. 
// ================================================
// Performance Comparisons: https://0fps.net/2015/01/23/collision-detection-part-3-benchmarks/
// Github:                  https://github.com/mikolalysenko/box-intersect
// Paper:                   https://pub.ist.ac.at/~edels/Papers/2002-J-01-FastBoxIntersection.pdf
// ================================================


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

static Bln _collided(const register Rect_ *r1P, const register Rect_ *r2P) {
  return r1P->x          < r2P->x + r2P->w &&
         r1P->x + r1P->w > r2P->x          &&
         r1P->y          < r2P->y + r2P->h &&
         r1P->y + r1P->h > r2P->y;
}

// returns index to SUBarray (not necessarily wrt array's beginning)
static U8 _getNodeSubIdx(XCollision *xCollSysP, Rect_ *rectP, U8 granularity) {
  return (((rectP->x << DECIMAL_RES) * granularity) + 
          ((rectP->y << DECIMAL_RES) * granularity)   ) >> DECIMAL_RES;
}
// TODO make granularity access a pre-computed ratio as per your notebook
// TODO use the above with a pointer to a subarray:
//      subA = &array[50];
//      subA[_getNodeSubIdx] = firstChild;

static void _removeNodeElement(Qt) {
  elemP->nextElemIdx
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
  //
  // TODO figure out how to check for onscreen in a fast way when cam or entity is moving; 
  //      [de]activate accordingly

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


/*
 * Collision *components* aren't actually in either set of collision grids; they're used to update it.
 * If I can get the collision component to 4 bytes, *perfect*.
 * I want a single-array speed boost.
 * Events COLLISION_BEGAN and COLLISION_ENDED should respond to motion events. In order to do so, it must keep a list of ongoing collisions. Don't iterate through that list; you must be able to key into it directly with the entity who moved. Should be a bitset since every entity has a unique number. This happens in the message processor (MP).
 * That would be nice if I only checked on moving sprites rather than querying every node. But the problem arises when *everbody's* moving; I'd then have a horribly inefficient random-access pattern, and on three whole levels at that! So I have to pick and choose. However, I could have my cake and eat it too: I can switch the system's sRun() to something different depending on the situation! 
 * 
 * LET THE LOGICKE BEGINNNNN
 * =========================
 * 
 * To only check onscreen moving entities for collisions, the camera must check for collisions.
 * For the camera to check for collisions, the quadtree must already be initialized.
 * For the quadtree to already be initiailzed, everybody in the world must be taken into account.
 * 
 * I think the overall beauty/justification of the onscreen method is that, for lower-end systems
 * like the Pi Zero, onscreen-based collision lightens other systems' loads as well; that means
 * animation, motion, AI, sound, etc. only have to operate on a fraction of the world's entities. 
 * How well this'll actually play out remains to be seen. So we'll let experimentation decide. I'll
 * still try to make this as fast and efficient as possible of course.
 *
 *
 * So here's how it's going to work:
 *    1) Compute active components' node ranges.
 *    2) compare to old node ranges (in yet another array). 
 *      a) if different, update quad tree nodes and their element lists.
 *    3) 
 *
 */
