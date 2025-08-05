#include "data/list.h"

static U32 listGetNodeIdx( List* listP, ListNodeHeader* nodeP ) {
  assert( listP && listP->array && nodeP );
  U32 elSz = arrayGetElemSz( listP->array );
  assert( (size_t) nodeP >= (size_t) listP->array );
  assert( (size_t) nodeP <= ( (size_t) listP->array + ( ( arrayGetNElems( listP->array ) - 1 ) * elSz ) ) );
  return ( (size_t) nodeP - (size_t) listP->array ) / elSz;
}

void listIni( List* listP, void* array ) {
  assert( listP && array );
  listP->head = listP->tail = 0;
  listP->flags = 0;
  listP->array = array;
}

// NOTE: This assumes the address of the header is the same as the address of the array element.
void listRemove( List* listP, ListNodeHeader* nodeP ) {
  U32 nodeIdx = listGetNodeIdx( listP, nodeP );  // internal assertions guard against bad access
#if 0  // TODO is this needed anymore?
  if ( listP->head == nodeIdx ) {
    listP->flags |= LIST_HAS_ELEMS;
  }
  if ( listP->tail == nodeIdx ) {
    listP->flags |= LIST_HAS_ELEMS;
  }
#endif
  // If you're removing the last remaining element of a list, set its flag to NOT having any members.
  if (listP->head == listP->tail) {
    assert( listP->head == nodeIdx );  // make sure the last node is actually the one you're removing 
    listP->flags &= (~LIST_HAS_ELEMS);
  }
  ListNodeHeader* prevP = (ListNodeHeader*) arrayGetVoidElemPtr( listP->array, nodeP->prev );
  ListNodeHeader* nextP = (ListNodeHeader*) arrayGetVoidElemPtr( listP->array, nodeP->next );
  prevP->next = nodeP->next;
  nextP->prev = nodeP->prev;
  // Is it even worth changing the removed node's data if nobody connects to it?
  // Maybe not if we make sure we set its relevant data when adding it back in.
}

void listInsertBefore( List* listP, ListNodeHeader* newNodeP, ListNodeHeader* nextNodeP ) {
  assert ( listP && listP->array && newNodeP );
  Key newIdx = listGetNodeIdx( listP, newNodeP );
  Key oldIdx = listGetNodeIdx( listP, nextNodeP );
  ListNodeHeader* oldNodesPreviousP = arrayGetVoidElemPtr( listP->array, nextNodeP->prev );
  newNodeP->prev = nextNodeP->prev;
  newNodeP->next = oldIdx;
  nextNodeP->prev = newIdx;
  oldNodesPreviousP->next = newIdx;
  // TODO bug: previous's next is not the newNodeP yet.
  if ( listP->head == oldIdx ) {
    listP->head = newIdx;
  }
}

void listInsertAfter( List* listP, ListNodeHeader* newNodeP, ListNodeHeader* prevNodeP ) {
  assert ( listP && listP->array && newNodeP && prevNodeP );
  Key newIdx = listGetNodeIdx( listP, newNodeP );
  Key oldIdx = listGetNodeIdx( listP, prevNodeP );
  ListNodeHeader* oldNodesNextP = arrayGetVoidElemPtr( listP->array, prevNodeP->next );
  newNodeP->next = prevNodeP->next;
  newNodeP->prev = oldIdx;
  prevNodeP->next = newIdx;
  oldNodesNextP->prev = newIdx;
  if ( listP->tail == oldIdx ) {
    listP->tail = newIdx;
  }
}

void listPrepend( List* listP, ListNodeHeader* newNodeP ) {
  assert ( listP && listP->array && newNodeP );
  Key newNodeIdx = listGetNodeIdx( listP, newNodeP );
  if ( listP->flags & LIST_HAS_ELEMS ) {
    ListNodeHeader* oldHeadNodeP = arrayGetVoidElemPtr( listP->array, listP->head );
    oldHeadNodeP->prev = newNodeIdx;
    newNodeP->next = listP->head;
    listP->head = newNodeIdx;
  }
  else {
    listP->head = listP->tail =  newNodeP->next = newNodeP->prev = newNodeIdx;
    listP->flags |= LIST_HAS_ELEMS;
  }
}

void listAppend( List* listP, ListNodeHeader* newNodeP ) {
  assert ( listP && listP->array && newNodeP );
  Key newNodeIdx = listGetNodeIdx( listP, newNodeP );
  if ( listP-> flags & LIST_HAS_ELEMS ) {
    ListNodeHeader* oldHeadNodeP = arrayGetVoidElemPtr( listP->array, listP->tail );
    oldHeadNodeP->next = newNodeIdx;
    newNodeP->prev = listP->tail;
    listP->tail = newNodeIdx;
  }
  else {
    listP->head = listP->tail =  newNodeP->next = newNodeP->prev = newNodeIdx;
    listP->flags |= LIST_HAS_ELEMS;
  }
}

