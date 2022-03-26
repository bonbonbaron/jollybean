#include "nectar.h"


NodeFuncDef_(cb2) {
  printf("node 2\n");
  return COMPLETE;
}
LeafNodeFuncDef_(b, cb2);
