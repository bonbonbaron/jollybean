#include "bt.h"

// Design-wise, it bothers me how I have multiple functions traversing BTs the same way just to perform a different action on them.
static void _nodePush(SrcNode *srcNodeP, Node *rootP, U8 *nextAvailIdxP) {
  Node *dstNodeP = &rootP[*nextAvailIdxP];
  dstNodeP->nodeCb = srcNodeP->nodeCb;
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

static U16 _countSrcNodes(SrcNode *nodeP) {
  U16 count = 1;  // counting this node
  if (nodeP->nChildren > 0) 
    for (U8 childCount = 0; childCount < nodeP->nChildren; childCount++) 
      count += _countSrcNodes(nodeP->childrenPA[childCount]);  // counting node's descendants
  return count;
}

static U16 _countNodes(Node *rootP, Node *startFromNodeP) {
  U16 count = 1;  // counting this node
  if (startFromNodeP->firstChildIdx) 
    for (Node *childNodeP = &rootP[startFromNodeP->firstChildIdx];
         childNodeP != rootP;
         childNodeP = &rootP[childNodeP->nextSiblingIdx])
      count += _countNodes(rootP, childNodeP);  // counting node's descendants
  return count;
}

static U16 _countSpecificNodes(Node *rootP, Node *startFromNodeP, NodeCb nodeCb) {
  U16 count = (startFromNodeP->nodeCb == nodeCb);
  if (startFromNodeP->firstChildIdx) 
    for (Node *childNodeP = &rootP[startFromNodeP->firstChildIdx];
         childNodeP != rootP;
         childNodeP = &rootP[childNodeP->nextSiblingIdx])
      count += _countSpecificNodes(rootP, childNodeP, nodeCb);  // counting node's descendants
  return count;
}

// Recursively map conditional nodes' indices to condition bytes.
static Error _iniCondKeys(Node *rootP, Node *startFromNodeP, NodeCb nodeCb, Map *mapP) {
  U8 tmp = 0;
  Error e = SUCCESS;
	// This node's index maps to a byte's worth of condition flags.
  if (startFromNodeP->nodeCb == nodeCb)
    mapSet(mapP, startFromNodeP->thisIdx, (void*) &tmp);
	// If this node has any children, recursively call this function on those matching the specified node (nodeCb).
  if (startFromNodeP->firstChildIdx)
    for (Node *childNodeP = &rootP[startFromNodeP->firstChildIdx];
         !e && childNodeP != rootP;
         childNodeP = &rootP[childNodeP->nextSiblingIdx])
      e = _iniCondKeys(rootP, childNodeP, nodeCb, mapP);  // counting node's descendants
  return e;
}

// B-Tree
Error btNew(SrcNode *srcNodeP, BTree **treePP) {
  Error e = jbAlloc((void**) treePP, sizeof(BTree), 1);
  if (!e)
    e = arrayNew((void**) &(*treePP)->rootP, sizeof(Node), _countSrcNodes(srcNodeP));
  if (!e) {
    U8 nextAvailIdx = 0;
    _nodePush(srcNodeP, (*treePP)->rootP, &nextAvailIdx);
  }
  return e;
}

void btDel(BTree **treePP) {
  arrayDel((void**) &(*treePP)->rootP);
  jbFree((void**) treePP);
}

void btSDel(BTreeS **btSPP) {
  if ((*btSPP)->treeP)
    btDel(&(*btSPP)->treeP);
}

// Blackboard
// These can only be made after their related trees are made.
// The reason is because it has to count the number of nodes the tree has
// as well as the number of condition nodes.
Error bbNew(Blackboard **bbPP, Key ownerId, BBSeed *bbSeedP) {
  if (!bbPP || bbSeedP)
    return E_BAD_ARGS;
  // Blackboard
  Error e = jbAlloc((void**) bbPP, sizeof(Blackboard), 1);
  Blackboard *bbP;
  // Map agent's blackboard element enums to the actual elements.
  if (!e) {
    bbP = *bbPP;
    e = mapNew(&bbP->agentBbMP, sizeof(void*), bbSeedP->nKeyValPairs);
  }
  if (!e) {
    KeyValPair *kvpP = bbSeedP->keyValPairA;
    KeyValPair *kvpEndP = kvpP + bbSeedP->nKeyValPairs;
    for (; !e && kvpP < kvpEndP; kvpP++)
      e = mapSet(bbP->agentBbMP, kvpP->key, kvpP->valueP);
  }
  return e;
}

// Ini instead of new; I want an array or map of these later on.
Error btStatNew(BTreeStatus **btStatPP, Node *rootP) {
  if (!btStatPP || !rootP)
    return E_BAD_ARGS;
  Error e = jbAlloc((void**) btStatPP, sizeof(BTreeStatus), 1);
  /// Node status array
	U16 nNodes = _countNodes(rootP, rootP);
  BTreeStatus *btStatP = NULL;
  if (!e) {
    btStatP = *btStatPP;
    e = arrayNew((void**) &btStatP->nodeStatA, sizeof(NodeStat), nNodes);
  }
  // X-Condition nodes: These nodes run based on bit-flag conditions set externally.
  if (!e) {
    U8 nXCondNodes = _countSpecificNodes(rootP, rootP, btXCondition);
    if (nXCondNodes)
      e = mapNew(&btStatP->conditionMP, sizeof(U32), nXCondNodes);
    else
      btStatP->conditionMP = NULL;
  }
	// Recursively map conditional nodes' indices to condition bytes.
  if (!e && btStatP->conditionMP)
    _iniCondKeys(rootP, rootP, btXCondition, btStatP->conditionMP);
  return e;
}

void btStatDel(BTreeStatus **btStatPP) {
  if (btStatPP) {
    BTreeStatus *btStatP = *btStatPP;
    arrayDel((void*) &btStatP->nodeStatA);
    mapDel(&btStatP->conditionMP);
    jbFree((void**) btStatPP);
  }
}

void bbDel(Blackboard **bbPP) {
  Blackboard *btStatP = *bbPP;
  mapDel(&btStatP->agentBbMP);
  jbFree((void**) bbPP);
}

static NodeFuncDef_(_nodeRun) {
  NodeStat stat = btStatP->nodeStatA[currNodeP->thisIdx];
  if (stat < COMPLETE)  // Less than COMPLETE is either READY or RUNNING.
    return currNodeP->nodeCb(rootP, currNodeP, btStatP, bbP, outboxF);
  return stat;
}

NodeStat btRun(BTree *treeP, BTreeStatus *btStatP, Blackboard *bbP, Message *outboxF) {
  return _nodeRun(treeP->rootP, treeP->rootP, btStatP, bbP, outboxF);
}

/************ Specific node types ************/
NodeFuncDef_(btSequence) {
  NodeStat stat = COMPLETE;
  for (Node *childNodeP = &rootP[currNodeP->firstChildIdx];
       stat == COMPLETE && childNodeP != rootP;
       childNodeP = &rootP[childNodeP->nextSiblingIdx]) 
    stat = _nodeRun(rootP, childNodeP, btStatP, bbP, outboxF);
  return stat;
}

NodeFuncDef_(btSelector) {
  NodeStat stat = FAILED;
  for (Node *childNodeP = &rootP[currNodeP->firstChildIdx];
       stat != COMPLETE && childNodeP != rootP;
       childNodeP = &rootP[childNodeP->nextSiblingIdx])
    stat = nodeRun_;
  return stat;
}

NodeFuncDef_(btXCondition) {
  NodeStat stat = FAILED;
  for (Node *childNodeP = &rootP[currNodeP->firstChildIdx];
       stat != COMPLETE && childNodeP != rootP;
       childNodeP = &rootP[childNodeP->nextSiblingIdx])
    stat = nodeRun_;
  return stat;
}


