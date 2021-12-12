#ifndef BT_H
#define BT_H
#include "ecs.h"

#define NO_CHILDREN NO_SIBLINGS_LEFT 
#define NO_SIBLINGS_LEFT (0)  /* Root can never be a sibling, so this works. */

struct Node;
typedef enum {COMPLETE, RUNNING, FAILED} NodeStat;
typedef NodeStat(*NodeCB)(struct Node *rootP, struct Node *currNodeP, U8 event, Map *bbP);  

// SrcNode is read-only memory. It gets translated to Node stored in an array at load-time.
typedef struct SrcNode {
  U8 nChildren;
  NodeCB nodeCB;
  struct SrcNode **childrenPA;
} SrcNode;

#define CbNode_(name_, cb_, ...)\
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

// NodeFunc_ enforces conformity to node callback
#define NodeFunc_(name_) NodeStat name_(Node *rootP, Node *currNodeP, U8 event, Map *bbP) 

// TODO may be able to squeeze something useful into 2-byte padding. expConditions is pending.
typedef struct Node {
  U8 firstChildIdx;
  U8 nextSiblingIdx;  // allows easy "while(nodeP->nextSibling)" condition-check
  U8 expConditions;   // 8 conditions max throughout tree
  NodeCB nodeCB;      // because nobody points at root @ index 0 as sibling or child
} Node;

typedef struct {
  U8 priority;
  Node *rootP;
} Tree;

// Functions
Error btNew(SrcNode *srcNodeP, U8 nTreeNodes, Node **treePP);
void btDel(Node **treePP);
NodeStat btRun(Node *rootP, U16 event, Map *bbP);
NodeFunc_(btSequence);
NodeFunc_(btSelector);
#endif
