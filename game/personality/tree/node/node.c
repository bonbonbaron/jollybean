#include "personality.h"

NodeCb_(cb1) {
  printf("node 1\n");
  U32 *hp = (U32*) mapGet(bbP->agentBbP, 1);
  U32 *mp = (U32*) mapGet(bbP->agentBbP, 2);
  printf("hp is %d; mp is %d.\n", *hp, *mp);
  return COMPLETE;
}

NodeCb_(cb2) {
  printf("node 2\n");
  return COMPLETE;
}

NodeCb_(cb3) {
  printf("node 3\n");
  return COMPLETE;
}

NodeCb_(cb4) {
  printf("node 4\n");
  return COMPLETE;
}

NodeCb_(cb5) {
  printf("node 5\n");
  return COMPLETE;
}

LeafNode_(a, cb1);
LeafNode_(b, cb2);
LeafNode_(c, cb3);
LeafNode_(d, cb4);
SelectorNode_(sel, &a, &b, &c, &d);
LeafNode_(e, cb5);
SequenceNode_(root, &sel, &e);
