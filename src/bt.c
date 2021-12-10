typedef unsigned char U8;
typedef signed char S8;
typedef unsigned short U16;
typedef signed short S16;
typedef unsigned int U32;
typedef signed int S32;

typedef enum {COMPLETE, RUNNING, FAILED} NodeStat;
typedef NodeStat(*NodeCB)();

// _Node is read-only memory. It gets translated to Node at load-time.
typedef struct SrcNode {
  NodeCB nodeCB;
  struct SrcNode *childrenP;
  U8 nChildren;
} SrcNode;

typedef struct Node {
  U8 firstChildIdx;
  U8 nextSiblingIdx;  // allows easy "while(nodeP->nextSibling)" condition-check
  NodeCB nodeCB;
} Node;

#define NO_CHILDREN NO_SIBLINGS_LEFT 
#define NO_SIBLINGS_LEFT (0)  /* Root can never be a sibling, so this works. */
void putNodeInArray(SrcNode *srcNodeP, Node *rootP, U8 *nextAvailIdxP) {
  Node *dstNodeP = &rootP[*nextAvailIdxP];
  dstNodeP->nodeCB = srcNodeP->nodeCB;
  // If node has children, add all its descendants to the array recursively.
  if (srcNodeP->nChildren > 0) {
    // Increment for destination node's first child.
    dstNodeP->firstChildIdx = ++*nextAvailIdxP;  // only first child is contiguous to its parent
    for (U8 childCount = 0; childCount < srcNodeP->nChildren; childCount++) {
      // Store current child for access after recursing through its descendants, if any.
      Node *dstChildP = &rootP[*nextAvailIdxP];
      // Add this child to the array at index *nextAvailP.
      putNodeInArray(&srcNodeP->childrenP[childCount], rootP, nextAvailIdxP);
      // Next sibling (may not be contiguous)
      if (childCount < (srcNodeP->nChildren - 1))
        dstChildP->nextSiblingIdx = *nextAvailIdxP;
      else
        dstChildP->nextSiblingIdx = NO_SIBLINGS_LEFT;
    }
  }
  // This node has no children; tell it so.
  else
    dstNodeP->firstChildIdx = NO_CHILDREN;
  // Increment for destination node's next sibling.
  ++*nextAvailIdxP;
}

// TODO: make BTree source storage algorithm.
