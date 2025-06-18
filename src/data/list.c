#include "data/list.h"

static U32 listGetNodeIdx( List* listP, ListNodeHeader* nodeP ) {
  assert( listP && listP->array && nodeP );
  U32 elSz = arrayGetElemSz( listP->array );
  assert( (size_t) nodeP > (size_t) listP->array );
  assert( (size_t) nodeP <= (size_t) listP->array + ( ( arrayGetNElems( listP->array ) - 1 ) * elSz );
  return ( (size_t) nodeP - (size_t) listP->array ) / elSz;
}

// NOTE: This assumes the address of the header is the same as the address of the array element.
void listRemoveNode( List* listP, ListNodeHeader* nodeP ) {
  U32 nodeIdx = listGetNodeIdx( listP, nodeP );
  if ( listP->head == nodeIdx ) {
    listP->head = UNSET_;
  }
  if ( listP->tail == nodeIdx ) {
    listP->tail = UNSET_;
  }
  ListNodeHeader* prevP = (ListNodeHeader*) arrayGetVoidElemPtr( nodeP->prev );
  ListNodeHeader* nextP = (ListNodeHeader*) arrayGetVoidElemPtr( nodeP->next );
  prevP->next = nodeP->next;
  nextP->prev = nodeP->prev;
  // Is it even worth changing the removed node's data if nobody connects to it?
  // Maybe not if we make sure we set its relevant data when adding it back in.
}

void listInsertBefore( List* listP, ListNodeHeader* newNodeP, ListNodeHeader* nextNodeP ) {
  newNodeP->prev = nextNodeP->prev;
  newNodeP->next = oldIdx;
  nextNodeP->prev = newIdx;
  if ( listP->head == oldIdx ) {
    listP->head = newIdx;
  }
}

void listInsertAfter( List* listP, ListNodeHeader* newNode, ListNodeHeader* prevNode ) {
  newNode->next = prevNode->next;
  newNode->prev = oldIdx;
  prevNode->next = newIdx;
  if ( listP->tail == oldIdx ) {
    listP->tail = newIdx;
  }
}

void listPrepend( List* listP, ListNodeHeader* newNodeP ) {
  assert ( listP && listP->array && newNodeP );
  U32 newNodeIdx = listGetNodeIdx( listP, newNodeP );
  if ( listP->head >= 0 ) {
    ListNode* oldHeadNodeP = arrayGetVoidElemPtr( listP->array, listP->head );
    oldHeadNodeP->prev = newNodeIdx;
    newNodeP->next = listP->head;
  }
  else {
    listP->head = listP->tail = newNodeIdx;
  }
  list->head = newNodeIdx;
}

void listAppend( List* listP, ListNodeHeader* newNode ) {
  newNode->next = prevNode->next;
  newNode->prev = oldIdx;
  prevNode->next = newIdx;
  if ( listP->tail == oldIdx ) {
    listP->tail = newIdx;
  }
}

