#include "data/bt.h"

// Binary trees
void* btNew( U32 elemSz, U32 nElems, const PoolId poolId) {
  assert (elemSz && nElems);
  void* btA = arrayNew(elemSz, nElems, poolId );
  // This pre-configures the headers in each element to indicate orphan-hood till tree node is linked.
  // We trust the user to re-assign the non-header values in the struct.
  memset(btA, ORPHAN_BYTE, elemSz * nElems);
  _btHeaderClr(btA);
  return btA;
}
