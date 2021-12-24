#include "bt.h"

static void _nodePush(SrcNode *srcNodeP, Node *rootP, U8 *nextAvailIdxP) {
  Node *dstNodeP = &rootP[*nextAvailIdxP];
  dstNodeP->nodeCB = srcNodeP->nodeCB;
  dstNodeP->thisIdx = *nextAvailIdxP;
  ++*nextAvailIdxP;
  // If node has children, add all its descendants to the array recursively.
  if (srcNodeP->nChildren > 0) {
    dstNodeP->firstChildIdx = *nextAvailIdxP;  // already incremented above
    for (U8 childCount = 0; childCount < srcNodeP->nChildren; childCount++) {
      Node *dstChildP = &rootP[*nextAvailIdxP];
      _nodePush(srcNodeP->childrenPA[childCount], rootP, nextAvailIdxP);
      if (childCount < (srcNodeP->nChildren - 1))
        dstChildP->nextSiblingIdx = *nextAvailIdxP;
    }
  }
}

static U32 _countSrcNodes(SrcNode *nodeP) {
  U8 count = 1;  // counting this node
  if (nodeP->nChildren > 0) {
    for (U8 childCount = 0; childCount < nodeP->nChildren; childCount++) 
      count += _countSrcNodes(nodeP->childrenPA[childCount]);  // counting node's descendants
  }
  return count;
}

static U32 _countNodes(Node *rootP, Node *currNodeP) {
  U8 count = 1;
  if (currNodeP->firstChildIdx) 
    for (Node *childNodeP = &rootP[currNodeP->firstChildIdx];
         childNodeP != rootP;
         childNodeP = &rootP[childNodeP->nextSiblingIdx])
      count += _countNodes(rootP, childNodeP);  // counting node's descendants
  return count;
}

static U32 _countSpecificNodes(Node *rootP, Node *currNodeP, NodeCB nodeCB) {
  U8 count = (currNodeP->nodeCB == nodeCB);
  if (currNodeP->firstChildIdx) 
    for (Node *childNodeP = &rootP[currNodeP->firstChildIdx];
         childNodeP != rootP;
         childNodeP = &rootP[childNodeP->nextSiblingIdx])
      count += _countSpecificNodes(rootP, childNodeP, nodeCB);  // counting node's descendants
  return count;
}

static Error _iniCondKeys(Node *rootP, Node *currNodeP, NodeCB nodeCB, Map *mapP) {
  U8 tmp = 0;
  Error e = SUCCESS;
  if (currNodeP->nodeCB == nodeCB)
    mapSet(mapP, currNodeP->thisIdx, (void*) &tmp);
  if (currNodeP->firstChildIdx)
    for (Node *childNodeP = &rootP[currNodeP->firstChildIdx];
         !e && childNodeP != rootP;
         childNodeP = &rootP[childNodeP->nextSiblingIdx])
      e = _iniCondKeys(rootP, childNodeP, nodeCB, mapP);  // counting node's descendants
  return e;
}

// B-Tree
Error btNew(SrcNode *srcNodeP, U8 priority, BTree **treePP) {
  Error e = jbAlloc((void**) treePP, sizeof(BTree), 1);
  if (!e)
    e = arrayNew((void**) &(*treePP)->rootP, sizeof(Node), _countSrcNodes(srcNodeP));
  if (!e) {
    U8 nextAvailIdx = 0;
    _nodePush(srcNodeP, (*treePP)->rootP, &nextAvailIdx);
    (*treePP)->priority = priority;
  }
  return e;
}

void btDel(BTree **treePP) {
  arrayDel((void**) &(*treePP)->rootP);
  jbFree((void**) treePP);
}

//Blackboard; these can only be made after their related trees are made.
Error bbNew(Blackboard **bbPP, Node *rootP, BBSeed *bbSeedP) {
  // Blackboard
  Error e = jbAlloc((void**) bbPP, sizeof(Blackboard), 1);
  Blackboard *bbP;
  /// Node status array
  if (!e) {
    bbP = *bbPP;
    e = arrayNew((void**) &bbP->nodeStatA, sizeof(NodeStat), _countNodes(rootP, rootP));
  }
  // X-Condition nodes: These nodes run based on bit-flag conditions set by one or more ECS ("X") systems.
  if (!e) {
    U8 nXCondNodes = _countSpecificNodes(rootP, rootP, btXCondition);
    if (nXCondNodes)
      e = mapNew(&bbP->conditionMP, sizeof(U32), nXCondNodes);
    else
      bbP->conditionMP = NULL;
  }
  if (!e && bbP->conditionMP)
    _iniCondKeys(rootP, rootP, btXCondition, bbP->conditionMP);
  // Agent's blackboard
  if (bbSeedP) {
    if (!e)
      e = mapNew(&bbP->agentBbP, sizeof(void*), bbSeedP->nKeyValPairs);
    if (!e) {
      KeyValPair *kvpP = bbSeedP->keyValPairA;
      KeyValPair *kvpEndP = kvpP + bbSeedP->nKeyValPairs;
      for (; !e && kvpP < kvpEndP; kvpP++)
        e = mapSet(bbP->agentBbP, kvpP->key, kvpP->valueP);
    }
  }
  return e;
}

void bbDel(Blackboard **bbPP) {
  Blackboard *bbP = *bbPP;
  mapDel(&bbP->agentBbP);
  mapDel(&bbP->conditionMP);
  arrayDel((void**) &bbP->nodeStatA);
  jbFree((void**) bbPP);
}

static NodeCb_(_nodeRun) {
  NodeStat stat = bbP->nodeStatA[currNodeP->thisIdx];
  if (stat < COMPLETE)  // Less than COMPLETE is either READY or RUNNING.
    return currNodeP->nodeCB(rootP, currNodeP, bbP);
  return stat;
}

NodeStat btRun(BTree *treeP, Blackboard *bbP) {
  return _nodeRun(treeP->rootP, treeP->rootP, bbP);
}

/************ Specific node types ************/
NodeCb_(btSequence) {
  NodeStat stat = COMPLETE;
  for (Node *childNodeP = &rootP[currNodeP->firstChildIdx];
       stat == COMPLETE && childNodeP != rootP;
       childNodeP = &rootP[childNodeP->nextSiblingIdx]) 
    stat = _nodeRun(rootP, childNodeP, bbP);
  return stat;
}

NodeCb_(btSelector) {
  NodeStat stat = FAILED;
  for (Node *childNodeP = &rootP[currNodeP->firstChildIdx];
       stat != COMPLETE && childNodeP != rootP;
       childNodeP = &rootP[childNodeP->nextSiblingIdx])
    stat = _nodeRun(rootP, childNodeP, bbP);
  return stat;
}

NodeCb_(btXCondition) {
  NodeStat stat = FAILED;
  for (Node *childNodeP = &rootP[currNodeP->firstChildIdx];
       stat != COMPLETE && childNodeP != rootP;
       childNodeP = &rootP[childNodeP->nextSiblingIdx])
    stat = _nodeRun(rootP, childNodeP, bbP);
  return stat;
}
