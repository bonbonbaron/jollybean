#ifndef BT_H
#define BT_H

#include "array.h"

/* Binary trees
 * ============
 * Binary trees in JB are implemented by tacking headers onto the elements of otherwise normal arrays.
 *   Using headers prevents single-child-per-level trees from growing to > 2^256 bytes.
 * Every node's 4-byte header is initialized to 0x33333333.
 *   This takes advantage of the impossibility of identical, initialized children.
 *   It uses that to say "This node is an orphan!" AKA "This node has no parents or children yet!"
 *
 */

typedef struct {
  Key            /* these all need to be the same type */
    childA[2],   // children of this element
    parent;      // parent of this element
} BtElHeader;

typedef enum { LEFT_CHILD, RIGHT_CHILD } Child;

// Orphan-hood (having neither parents nor children) 
//   is determined when both children are identical non-zeros. 
// The rest is just because I'm a dork.
#define ORPHAN_BYTE (0x33)
// TODO adjust for endianness at some point
#define ORPHAN ((ORPHAN_BYTE) << 16 | (ORPHAN_BYTE) << 8 | (ORPHAN_BYTE))

inline U8 _btIsAnOrphan(BtElHeader *btHeaderP) {
  return ((*((U32*) btHeaderP) & 0x00ffffff) == ORPHAN);
}

inline U8 _btHasChild(BtElHeader *btElHeaderP, U32 child_) {
  return btElHeaderP->childA[child_];  /* if nonzero, non-orphan has childn*/
}

inline void _btHeaderClr(BtElHeader *btElHeaderP) {
  memset(btElHeaderP, 0, sizeof(BtElHeader));
}

// For both indices, just pass in the difference between the full element and the array address.
inline void _btLinkNodes(BtElHeader *parentHdrP, BtElHeader *childHdrP, Key parentIdx, Key childIdx, Key child) {
  parentHdrP->childA[child] = childIdx;
  _btHeaderClr(childHdrP);
  childHdrP->parent = parentIdx;
}

Error btNew(void **btAP, U32 elemSz, U32 nElems);

inline void _btDel(BtElHeader **btPP) {
  arrayDel((void**) btPP);
}

#endif  // #ifndef BT_H
