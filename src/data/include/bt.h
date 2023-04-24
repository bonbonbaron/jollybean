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

typedef enum { LEFT_CHILD, RIGHT_CHILD } Child;

// Orphan-hood (having neither parents nor children) 
//   is determined when both children are identical non-zeros. 
// The rest is just because I'm a dork.
#define ORPHAN_BYTE_ (0x33)
#define ORPHAN_ (\
    /* ORPHAN_BYTE_ << 24 | */ \
    ORPHAN_BYTE_ << 16 | \
    ORPHAN_BYTE_ <<  8 | \
    ORPHAN_BYTE_)
#define btIsAnOrphan_(btP_, idx_) ((*((U32*) &btP_[(idx_)].header) & 0x00ffffff) == ORPHAN_)
#define btHasChild_(btElHeaderP_, child_) btElHeaderP_->childA[child_]  /* if nonzero, non-orphan has childn*/
#define btEraseHeader_(btElHeaderP_) memset(btElHeaderP_, 0, sizeof(BtElHeader))
#define btLinkNodes_(btP_, parentP_, childP_, child_) \
  (parentP_)->header.childA[child_] = (childP_) - btP_;\
  btEraseHeader_(&(childP_)->header);\
  (childP_)->header.parent = (parentP_) - btP_;

typedef struct {
  Key            /* these all need to be the same type */
    childA[2],   // children of this element
    parent;      // parent of this element
} BtElHeader;

Error btNew(void **btAP, U32 elemSz, U32 nElems);
#define btDel_ arrayDel

#endif  // #ifndef BT_H
