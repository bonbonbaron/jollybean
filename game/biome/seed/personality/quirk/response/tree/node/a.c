#include "nectar.h"

NodeCb_(cb1) {
  printf("node 1\n");
  U32 *hp = (U32*) mapGet(bbP->agentBbP, 1);
  U32 *mp = (U32*) mapGet(bbP->agentBbP, 2);
  printf("hp is %d; mp is %d.\n", *hp, *mp);
  return COMPLETE;
}

LeafNode_(a, cb1);
