#ifndef SYS_Collision
#define SYS_Collision
#include "botox.h"

typedef struct {
} XCollisionCompSrc; 

// This is just a scratch-sheet area-- not part of the code above. I'll delete this later.
typedef struct {
  Entity entity;
  Key    type;         // When someone collides with this element, they need to know what type of reaction to have.
  Key    rectIdx;
  Key    currIdx;      // If you glue all elements together, you can easily do this.
} XCollisionComp;

// With two bytes per node (32 nodes per cache line), traversal is so fast that I may simplify my algorithm a bit.
typedef struct {
  Key count;       // If count > 1, *then* bother to drill into this node.
  Key firstChild;  // If count < splitTrhesh (defined in XCollision struct), this points to 
} QtNode;

typedef struct {
  Entity componentIdx;
  Key prevElemIdx;   // This allows simple tree updates when an element leaves a QT node.
  Key nextElemIdx;
} QtNodeElem;

/* How it works:
 *   If anybody moves, update the whole try by iterating through all of the moving entities
 *   To simplify processing, each entity can only process one dynamic collision at a time.
 *   Always process all fixed collisions.
 *   So which dynamic collision is most important? 
 *
 */
typedef struct {
  System  system;
  Key     splitThresh;         // When a node's count reaches this value, split it (*unless* unless it's already at the fine level).
  Entity  cameraEntity;
  U16    *collisionA;          // Keeps track of all current collisions.
  Bln    *qtElemTraversedA;          // Avoid processing any node twice.
  QtNode *coarseQtA;           // Split tree into three levels of granularity to mitigate unnecessary fine-level processing.
  QtNode *mediumQtA;
  QtNode *fineQtA;
  U8     *fixedGridA;          // Fixed grid can be queried directly without traversal.
} XCollision;

// Images
Error xCollisionIniS(System *sP, void *sParamsP);
Error xCollisionProcessMessage(System *sP, Message *msgP);
extern System *sCollisionP;
#endif

/* 
 * Before anything happens, it's necessary to update the world's quadtree. For each moving entity, qtPlace will put them in their appropriate node(s).
 * qtPlace computes the top-left and bottom-right nodes; the others are inferred. But this is different for each node based on its granularity. 
 *
 * qtNodePushElem(node, elemIdx, elems)
 *    elems[elemIdx].prev = -1;
 *    elems[elemIdx].next = nodeP->firstChild;
 *    elems[nodeP->firstChild].prev = elemIdx;
 *    nodeP->firstChild = elemIdx;
 *
 * qtNodeRemoveElem(elem)
 *    elems[elem.prev].next = elem.next;
 *    elems[elem.next].prev = elem.prev;
 *    elem.next = elem.prev = -1;
 *
 * TODO make this return top-left and bottom-right nodes
 * getQtNode(x, y, granularity)
 *    return (e.x / (level.w >> granularity)) + (e.y / (level.h >> granularity)) * (1 << granularity); 
 *
 * I want there to be just one quadtree. The camera will test it for collisions with itself ONLY if it moves. The way we'll know if it moves is
 * if the collision system receives a MOVED message and its attn field matches its own cameraEntity field. It addes every collided entity to
 * its queue for further processing. It has enough space to store all mobile entities in its arrays. We fill the outbox with messages about the
 * camera colliding with each one of these entities. One effect this has is for the BT to tell the animation and, if necessary, motion systems
 * to activate these entities' components. We don't have to worry about telling the animation system to animate something without an animation
 * component, because the reaction to this trigger will be the correct one.
 *
 *
 * 
 *
 *
 */
