#include "nectar.h"


NodeFuncDef_(cb3) {
  printf("node 3\n");
  return COMPLETE;
}

LeafNodeFuncDef_(c, cb3);
