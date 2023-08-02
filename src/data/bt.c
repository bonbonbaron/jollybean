#include "bt.h"

// Binary trees
Error btNew(void **btAP, U32 elemSz, U32 nElems) {
  if (!btAP || !elemSz || !nElems) {
    return E_BAD_ARGS;
  }

  Error e = arrayNew(btAP, elemSz, nElems);

  // This pre-configures the headers in each element to indicate orphan-hood till tree node is linked.
  // We trust the user to re-assign the non-header values in the struct.
  if (!e) {
    memset(*btAP, ORPHAN_BYTE, elemSz * nElems);
    BtElHeader *firstHeaderP = *btAP;
    _btHeaderClr(firstHeaderP);
  }

  return e;
}
