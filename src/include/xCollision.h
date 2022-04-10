#ifndef SYS_Collision
#define SYS_Collision
#include "botox.h"
#include "yoyoInterface.h"

// This is how many bits represent decimal places. When we compute quadtree node(s) a rectangle's in, we avoid integer division (e.x * nCellsW / worldW) by storing the second term in a bitshifted format:
// COARSE_CELL_SCALE = (nCellsW << 16) / worldW.
// So computing cell's X coordinate goes: ((e.x << 16) * COARSE_CELL_SCALE) >> 16.
// For cell index, we have to do it a bit differently for efficiency:
// (((e.x << DECIMAL_RES) * SCALE_X) + ((e.y << DECIMAL_RES) * SCALE_Y) >> DECIMAL_RES
#define DECIMAL_RES (16) 

typedef struct {
} XCollisionCompSrc; 

// This is just a scratch-sheet area-- not part of the code above. I'll delete this later.
typedef struct {
  Entity entity;
  Key    type;         // When someone collides with this element, they need to know what type of reaction to have.
  Key    rectIdx;
  Key    currIdx;      // I don't know how feasible this is yet.
} XCollisionComp;

// With two bytes per node (32 nodes per cache line), traversal is so fast that I may simplify my algorithm a bit.
typedef struct {
  Key count;       // If count > 1, *then* bother to drill into this node.
  Key firstChild;  // If count < splitTrhesh (defined in XCollision struct), this points to 
} QtNode;

typedef struct {
  Entity entity;   // entity is more stable than component index, preventing excess updates.
  Key prevElemIdx;   // This allows simple tree updates when an element leaves a QT node.
  Key nextElemIdx;
} QtNodeElem;

typedef struct {
  U8 granularity;  // By this the  requestor can infer which array(s) to populate.
  U32 node1Idx;
  U32 node2Idx;
} NodeRegion;

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
  U32     worldWidth;
  U32     worldHeight;
  U32     COARSE_SCALE;
  U32     MEDIUM_SCALE;   // TODO may remove this and FINE_SCALE later 
  U32     FINE_SCALE;
  U32     nFineCellsX;
  U32     nFineCellsY;
  Rect_  *rectA;               // This points to the master's rectangle map's mapA array.
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
