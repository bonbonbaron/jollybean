#include "nectar.h"

NodeFuncDef_(cb5) {
  printf("node 5\n");
  return COMPLETE;
}

LeafNodeFuncDef_(e, cb5);
