#include "bt.h"

static void _nodePush(SrcNode *srcNodeP, Node *rootP, U8 *nextAvailIdxP) {
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
      _nodePush(srcNodeP->childrenPA[childCount], rootP, nextAvailIdxP);
      // Next sibling (may not be contiguous)
      if (childCount < (srcNodeP->nChildren - 1))
        dstChildP->nextSiblingIdx = *nextAvailIdxP;
      else
        dstChildP->nextSiblingIdx = NO_SIBLINGS_LEFT;
    }
  }
  // This node has no children.
  else
    dstNodeP->firstChildIdx = NO_CHILDREN;
  // Increment for destination node's next sibling.
  ++*nextAvailIdxP;
}

Error btNew(SrcNode *srcNodeP, U8 nTreeNodes, Node **treePP) {
  Error e = arrayNew((void**) treePP, sizeof(Node), nTreeNodes);
  if (!e) {
    U8 nextAvailIdx = 0;
    _nodePush(srcNodeP, *treePP, &nextAvailIdx);
  }
  return e;
}

void btDel(Node **treePP) {
  arrayDel((void**) treePP);
}

static NodeFunc_(_nodeRun) {
  if ((event & rootP->expConditions) == rootP->expConditions)
    return currNodeP->nodeCB(rootP, currNodeP, event, bbP);
  return FAILED;
}

NodeStat btRun(Node *rootP, U16 event, Map *bbP) {
  return _nodeRun(rootP, rootP, event, bbP);
}

/************ Specific node types ************/
NodeFunc_(btSequence) {
  NodeStat stat = COMPLETE;
  for (Node *childNodeP = &rootP[currNodeP->firstChildIdx];
       stat == COMPLETE && childNodeP != rootP;
       childNodeP = &rootP[childNodeP->nextSiblingIdx])
    stat = _nodeRun(rootP, childNodeP, event, bbP);
  return stat;
}

NodeFunc_(btSelector) {
  NodeStat stat = FAILED;
  for (Node *childNodeP = &rootP[currNodeP->firstChildIdx];
       stat != COMPLETE && childNodeP != rootP;
       childNodeP = &rootP[childNodeP->nextSiblingIdx])
    stat = _nodeRun(rootP, childNodeP, event, bbP);
  return stat;
}
