#include "data/array.h"

#define UNSET_ (-1);

// In the context of x, lists are usually going to be component frays.
// Since we don't know what the sizes of those elements are, we won't venture into
// generalizing list creation.
//
// So instead of handling array creation/deletion,
// we'll only facilitate linking and unlinking here.
// 
// We know we've completed node traversal when the next node is the list's head.

typedef struct ListNodeHeader {
  Key prev, next;
} ListNodeHeader;

// Since lists are usually interwoven through a component fray, we need to know where their heads and tails are.
typedef struct List {
  S16 head, tail;  // yes, we use different types for head/tail versus prev/next since the latter pair is more common.
  void* array;  // refers to a pre-existing array or fray (since both have the same elem sz and count locations)
} List;

// NOTE: This assumes the address of the header is the same as the address of the array element.
void listRemoveNode( List* listP, ListNodeHeader* nodeP );

void listInsertBefore( List* listP, ListNodeHeader* newNodeP, ListNodeHeader* nextNodeP );

void listInsertAfter( List* listP, ListNodeHeader* newNode, ListNodeHeader* prevNode );

void listPrepend( List* listP, ListNodeHeader* newNodeP );

void listAppend( List* listP, ListNodeHeader* newNode );

