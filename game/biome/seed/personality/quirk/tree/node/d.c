#include "nectar.h"

NodeFuncDef_(cb4) {
  printf("node 4\n");
  return COMPLETE;
}

LeafNodeFuncDef_(d, cb4);
