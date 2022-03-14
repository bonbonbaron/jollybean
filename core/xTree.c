#include "xTree.h"

// Design-wise it bothers me how I have multiple functions traversing BTs the same way just to perform a different action on them.
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

// Blackboard
// These can only be made after their related trees are made.
// The reason is because it has to count the number of nodes the tree has
// as well as the number of condition nodes.
Error bbNew(Blackboard **bbPP, Node *rootP, Key ownerId, BBSeed *bbSeedP) {
  // Blackboard
  Error e = jbAlloc((void**) bbPP, sizeof(Blackboard), 1);
  Blackboard *bbP;
  /// Node status array
	U16 nNodes = _countNodes(rootP, rootP);
  if (!e) {
    bbP = *bbPP;
    e = arrayNew((void**) &bbP->nodeStatA, sizeof(NodeStat), nNodes);
  }
	
	if (!e)
		e = mailboxNew(&bbP->outboxP, ownerId, nNodes);
  // X-Condition nodes: These nodes run based on bit-flag conditions set externally.
  if (!e) {
    U8 nXCondNodes = _countSpecificNodes(rootP, rootP, btXCondition);
    if (nXCondNodes)
      e = mapNew(&bbP->conditionMP, sizeof(U32), nXCondNodes);
    else
      bbP->conditionMP = NULL;
  }
	// Recursively map conditional nodes' indices to condition bytes.
  if (!e && bbP->conditionMP)
    _iniCondKeys(rootP, rootP, btXCondition, bbP->conditionMP);
  // Map agent's blackboard element enums to the actual elements.
  if (bbSeedP) {
    if (!e)
      e = mapNew(&bbP->agentBbMP, sizeof(void*), bbSeedP->nKeyValPairs);
    if (!e) {
      KeyValPair *kvpP = bbSeedP->keyValPairA;
      KeyValPair *kvpEndP = kvpP + bbSeedP->nKeyValPairs;
      for (; !e && kvpP < kvpEndP; kvpP++)
        e = mapSet(bbP->agentBbMP, kvpP->key, kvpP->valueP);
    }
  }
  return e;
}

void bbDel(Blackboard **bbPP) {
  Blackboard *bbP = *bbPP;
  mapDel(&bbP->agentBbMP);
  mapDel(&bbP->conditionMP);
  arrayDel((void**) &bbP->nodeStatA);
  jbFree((void**) bbPP);
}

static Node_(_nodeRun) {
  NodeStat stat = bbP->nodeStatA[currNodeP->thisIdx];
  if (stat < COMPLETE)  // Less than COMPLETE is either READY or RUNNING.
    return currNodeP->nodeCb(rootP, currNodeP, bbP, outboxP);
  return stat;
}

NodeStat btRun(BTree *treeP, Blackboard *bbP, Mailbox *outboxP) {
  return _nodeRun(treeP->rootP, treeP->rootP, bbP, outboxP);
}

/************ Specific node types ************/
Node_(btSequence) {
  NodeStat stat = COMPLETE;
  for (Node *childNodeP = &rootP[currNodeP->firstChildIdx];
       stat == COMPLETE && childNodeP != rootP;
       childNodeP = &rootP[childNodeP->nextSiblingIdx]) 
    stat = _nodeRun(rootP, childNodeP, bbP, outboxP);
  return stat;
}

Node_(btSelector) {
  NodeStat stat = FAILED;
  for (Node *childNodeP = &rootP[currNodeP->firstChildIdx];
       stat != COMPLETE && childNodeP != rootP;
       childNodeP = &rootP[childNodeP->nextSiblingIdx])
    stat = _nodeRun(rootP, childNodeP, bbP, outboxP);
  return stat;
}

Node_(btXCondition) {
  NodeStat stat = FAILED;
  for (Node *childNodeP = &rootP[currNodeP->firstChildIdx];
       stat != COMPLETE && childNodeP != rootP;
       childNodeP = &rootP[childNodeP->nextSiblingIdx])
    stat = _nodeRun(rootP, childNodeP, bbP, outboxP);
  return stat;
}

// msgP may contain information the triggered callback finds pertinent; e.g. "Who collided w/ me?"
//TODO: make a way to stop all active components of an entity, and no more than that, in order to efficiently switch between an entity's reactions
#if 0
static Error _trigger(System *sP, Message *msgP) {
	return xStartFocus(sP, msgP->to, 1);
}

static Error _triggerGroup(System *sP, Message *msgP) {
	assert(msgP);
	Error e = SUCCESS;
	Entity *eA = (Entity*) mapGet(_subscriberAMP, msgP->topic);
	assert(eA);
	Entity *eP, *eEndP;
	arrayIniPtrs((void*) eA, (void**) &eP, (void**) &eEndP, -1);
	for (; !e && eP < eEndP; eP++) {
    msgP->to = *eP;
		e = _trigger(sP, msgP);
  }
	return e;
}
#endif

inline static U8 _isHigherPriority(U8 newPriority, U8 existingPriority) {
	return newPriority > existingPriority;
}

#if 0
static Error _readOutboxes(Focus *fP) {
	assert(fP);
	System *cP, *cEndP;
	Error e = SUCCESS;
	// Check active subsystems' outboxes. Their callbacks populate systems and JB's reaction activity.
	arrayIniPtrs(fP->compA, (void**) &cP, (void**) &cEndP, fP->firstInactiveIdx);
	for (; !e && cP < cEndP; cP++) 
		// Read outboxes
		for (Message *msgP = cP->outboxP->msgA, *msgEndP = cP->outboxP->msgA + cP->outboxP->nMsgs;
				 !e && msgP < msgEndP;
				 msgP++) 
			continue;
			//if (msgP->to) 

	return e;
}

static Error xTreeRunTasks(Focus *fP) {
	assert(fP);
	System *cP, *cEndP;
	Error e = SUCCESS;
	// Check active subsystems' outboxes. Their callbacks populate systems and JB's reaction activity.
	arrayIniPtrs(fP->compA, (void**) &cP, (void**) &cEndP, fP->firstInactiveIdx);
	for (; !e && cP < cEndP; cP++) 
		// Read outboxes
		for (Message *msgP = cP->outboxP->msgA, *msgEndP = cP->outboxP->msgA + cP->outboxP->nMsgs;
				 !e && msgP < msgEndP;
				 msgP++) 
			if (msgP->to) 
				e = _trigger(fP->ownerP, msgP);
			else
				e = _triggerGroup(fP->ownerP, msgP);
	
	return SUCCESS;
}
#endif
