#include "data/bitmap.h"
#include "data/array.h"

#define UNSET_ (0xFF)

// In the context of x, lists are usually going to be component frays.
// Since we don't know what the sizes of those elements are, we won't venture into
// generalizing list creation.
//
// So instead of handling array creation/deletion,
// we'll only facilitate linking and unlinking here.
// 
// We know we've completed node traversal when the next node is the list's head.

typedef struct ListNodeHeader {
  Key prev, next, listIdIdx;
} ListNodeHeader;

// Although it's a separate array, metalist impacts caching performance negligibly.
// We only interact with it when merging or splitting lists. (TODO: impl splitting only when you find a  need.)
/* The following rules apply for list IDs:
 * If we're initializing a new list, we grab the bit index of the first zero in the bitfield. That index is its ID.
 * A new list is initialized when we append or prepend a node to an empty list.
 * Here's the tough part: The index of the element's ID in the ID array is unknown right know. I don't know what to do about that. Don't tell me every list needs two bitfields!!
 *
 *  Initial state:
 *    Bitmap for available IDs:   00000000 (8 bits; pretend 7 is our maximum possible list ID for 8 elements in the array.)
 *    ID array: { UNSET_ UNSET_ UNSET_ UNSET_ UNSET_ UNSET_ UNSET_ UNSET_
 *
 *  Make 2 lists:
 *    Bitmap for available IDs:   00000011 
 *    ID array: { UNSET_ UNSET_ UNSET_ UNSET_ UNSET_ UNSET_ 1, 0 }
 *      - all elements in list 0 have ID index 0
 *      - all elements in list 1 have ID index 1
 *
 *  Merge those 2 lists:
 *    Bitmap for available IDs:   00000001 
 *    ID array: { UNSET_ UNSET_ UNSET_ UNSET_ UNSET_ UNSET_ 0, 0 }
 *      - all elements in list 0 have ID index 0
 *      - all elements *formerly* in list 1 have ID index 0
 *    
 *  Create a new list:
 *    Bitmap for available IDs:   00000011 
 *    ID array: { UNSET_ UNSET_ UNSET_ UNSET_ UNSET_ UNSET_ 0, 0 }
 *      - all elements in list 0 have ID index 0
 *      - all elements *formerly* in list 1 have ID index 0
 *
 */
typedef struct ListMetadata {
  Key maxId;
  Key *idA;
  VolatileBitmap *availableIdBitmapA;  // 0s are available IDs for new nodes, which can, by the way, move around-- should have nothing to do with its position in the array.
  VolatileBitmap *availableIdIndexBitmapA;  // 0s are available IDs for new nodes, which can, by the way, move around-- should have nothing to do with its position in the array.
} ListMetadata;

// Since lists are usually interwoven through a fray, we need to know where their heads and tails are.
typedef struct List {
  Key id;  // the only way to prevent removing a node from a list it's not in and re-adding it to a list it's in
  Key head, tail;  // yes, we use different types for head/tail versus prev/next since the latter pair is more common.
  ListMetadata* metaP;  // common metadata that all lists from the same array point to
  void* array;  // refers to a pre-existing array or fray (since both have the same elem sz and count locations)
} List;

void listIni( List* listP, void* array, const List* leaderListP, const PoolId poolId );

// NOTE: This assumes the address of the header is the same as the address of the array element.
void listRemove( List* listP, ListNodeHeader* nodeP );

void listInsertBefore( List* listP, ListNodeHeader* newNodeP, ListNodeHeader* nextNodeP );

void listInsertAfter( List* listP, ListNodeHeader* newNode, ListNodeHeader* prevNode );

void listPrepend( List* listP, ListNodeHeader* newNodeP );

void listAppend( List* listP, ListNodeHeader* newNode );

void listMerge( List* srcListP, List* dstListP );

void* listGetHead( List* listP );

void* listGetTail( List* listP );

void* listNodeGetNext( List* listP, ListNodeHeader* nodeP );

VolatileBitmap* vbmNew( U32 maxBitIdx, const PoolId poolId );

