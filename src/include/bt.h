#ifndef BT_H
#define BT_H
#include "data.h"

#define NO_CHILDREN NO_SIBLINGS_LEFT 
#define NO_SIBLINGS_LEFT (0)  /* Root can never be a sibling, so this works. */

struct Node;

// RUNNING indicates that ECS is running checks on active components. When a check reports something, the
// tree reiterates from the root. (Multiple nodes could be waiting on different checks at the same time.) 
// That "RUNNING" node will eventually see the updated state (from a map whose keys are BT array indices; 
// its values are states) and performs its next block of code if it meets its conditions. 

typedef enum {READY, RUNNING, COMPLETE, FAILED, ERROR} NodeStat;

typedef struct {
  U8 nKeyValPairs;
  KeyValPair *keyValPairA;
} BBSeed;

typedef struct {
  NodeStat *nodeStatA;
  Map      *conditionMP;  // maps node indices to U32 condition flags specifically enumerated for the condition node
  Map      *agentBbP;     // maps a state name to a void pointer to it. Not world; access that directly.
} Blackboard;

typedef NodeStat (*NodeCB)(struct Node *rootP, struct Node *currNodeP, Blackboard *bbP);  

// SrcNode is read-only memory. It gets translated to Node stored in an array at load-time.
typedef struct SrcNode {
  U8 nChildren;
  U8 condition;
  NodeCB nodeCB;
  struct SrcNode **childrenPA;
} SrcNode;

#define LeafNode_(name_, cb_, ...)\
  SrcNode *name_##ChildrenA[] = {__VA_ARGS__};\
  SrcNode name_ = {\
    .nodeCB = cb_,\
    .nChildren = NUM_ARGS_(SrcNode*, __VA_ARGS__),\
    .childrenPA = name_##ChildrenA\
  };

#define SelectorNode_(name_, ...)\
  SrcNode *name_##ChildrenA[] = {__VA_ARGS__};\
  SrcNode name_ = {\
    .nodeCB = btSelector,\
    .nChildren = NUM_ARGS_(SrcNode*, __VA_ARGS__),\
    .childrenPA = name_##ChildrenA\
  };

#define SequenceNode_(name_, ...)\
  SrcNode *name_##ChildrenA[] = {__VA_ARGS__};\
  SrcNode name_ = {\
    .nodeCB = btSequence,\
    .nChildren = NUM_ARGS_(SrcNode*, __VA_ARGS__),\
    .childrenPA = name_##ChildrenA\
  };

// NodeCb_ enforces conformity to node callback
#define NodeCb_(name_) NodeStat name_(Node *rootP, Node *currNodeP, Blackboard *bbP) 

// TODO may be able to squeeze something useful into 1-byte padding. 
typedef struct Node {
  U8 firstChildIdx;
  U8 nextSiblingIdx;  // allows easy "while(nodeP->nextSibling)" condition-check
  U8 thisIdx;         // allows updating of its own NodeStat and for keying condition map
  U8 condition;       // bit-flag conditions which must be met to run this node
  NodeCB nodeCB;      // because nobody points at root @ index 0 as sibling or child
} Node;

typedef struct NodeA {
  U8 *firstChildIdxA;
  U8 *nextSiblingIdxA;  // allows easy "while(nodeP->nextSibling)" condition-check
  U8 *thisIdxA;         // allows updating of its own NodeStat and for keying condition map
  U8 *conditionA;
  NodeCB *nodeCbA;      // because nobody points at root @ index 0 as sibling or child
} NodeA;

typedef struct {
  U8 priority;
  Node *rootP;
} BTree;

// Functions
Error btNew(SrcNode *srcNodeP, U8 priority, BTree **treePP);
void btDel(BTree **treePP);
Error bbNew(Blackboard **bbPP, Node *rootP, BBSeed *bbSeedP);
void  bbDel(Blackboard **bbPP);
NodeStat btRun(BTree *treeP, Blackboard *bbP);
NodeCb_(btSequence);
NodeCb_(btSelector);
NodeCb_(btCondition);   // easy-to-check condition (e.g. world state)
NodeCb_(btXCondition);  // ECS-based condition
#endif
