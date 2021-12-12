#ifndef BT_H
#define BT_H
#include "ecs.h"

#define NO_CHILDREN NO_SIBLINGS_LEFT 
#define NO_SIBLINGS_LEFT (0)  /* Root can never be a sibling, so this works. */

struct Node;
typedef enum {COMPLETE, RUNNING, FAILED} NodeStat;
typedef NodeStat(*NodeCB)(const struct Node *rootP, struct Node *currNodeP, U8 event, Map *bbP);  

// SrcNode is read-only memory. It gets translated to Node stored in an array at load-time.
typedef struct SrcNode {
  U8 nChildren;
  U8 priority;
  NodeCB nodeCB;
  struct SrcNode *childrenP;
} SrcNode;

// TODO may be able to squeeze something useful into 2-byte padding. expConditions is pending.
typedef struct Node {
  U8 firstChildIdx;
  U8 nextSiblingIdx;  // allows easy "while(nodeP->nextSibling)" condition-check
  U16 expConditions;   // 8 conditions max throughout tree
  NodeCB nodeCB;      // because nobody points at root @ index 0 as sibling or child
} Node;

typedef struct {
  U8 priority;
  Node *rootP;
} Tree;
#endif
