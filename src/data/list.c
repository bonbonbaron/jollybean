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
  listP->head = listP->tail = UNSET_;
  listP->array = array;
}

// NOTE: This assumes the address of the header is the same as the address of the array element.
void listRemove( List* listP, ListNodeHeader* nodeP ) {
  U32 nodeIdx = listGetNodeIdx( listP, nodeP );
  if ( listP->head == nodeIdx ) {
    listP->head = UNSET_;
  }
  if ( listP->tail == nodeIdx ) {
    listP->tail = UNSET_;
  }
  // If the removed node has a previous node, then set the previous's next to the current's next.
  if ( nodeP->prev != UNSET_ ) {
    ListNodeHeader* prevP = (ListNodeHeader*) arrayGetVoidElemPtr( listP->array, nodeP->prev );
    prevP->next = nodeP->next;  // valid even if next is UNSET_
  }
  // If the removed node has a next node, then set the next's previous to the current's previous.
  if ( nodeP->next != UNSET_ ) {
    ListNodeHeader* nextP = (ListNodeHeader*) arrayGetVoidElemPtr( listP->array, nodeP->next );
    nextP->prev = nodeP->prev;  // valid even if prev is UNSET_
  }
  // Make it clear to the user this node is OUTSIDE the list now.
  nodeP->next = nodeP->prev = UNSET_;  
}

void listInsertBefore( List* listP, ListNodeHeader* newNodeP, ListNodeHeader* tgtNodeP ) {
  assert ( listP && listP->array && newNodeP );
  Key newIdx = listGetNodeIdx( listP, newNodeP );
  Key tgtIdx = listGetNodeIdx( listP, tgtNodeP );
  if ( tgtNodeP->prev != UNSET_ ) {
    // get pointer to node previously before target
    ListNodeHeader* nodePreviouslyBeforeTgt = arrayGetVoidElemPtr( listP->array, tgtNodeP->prev );
    // stick new node between target's previous and target
    newNodeP->prev = tgtNodeP->prev;
    newNodeP->next = tgtIdx;
    tgtNodeP->prev = newIdx;
    nodePreviouslyBeforeTgt->next = newIdx;
  }
  else {
    // Same as the above, except there's nothing before target
    newNodeP->prev = tgtNodeP->prev;
    newNodeP->next = tgtIdx;
    tgtNodeP->prev = newIdx;
  }
  if ( listP->head == tgtIdx ) {
    listP->head = newIdx;
  }
}

void listInsertAfter( List* listP, ListNodeHeader* newNodeP, ListNodeHeader* tgtNodeP ) {
  assert ( listP && listP->array && newNodeP && tgtNodeP );
  Key newIdx = listGetNodeIdx( listP, newNodeP );
  Key tgtIdx = listGetNodeIdx( listP, tgtNodeP );
  if ( tgtNodeP->next != UNSET_ ) {
    ListNodeHeader* nodePreviouslyAfterTgt = arrayGetVoidElemPtr( listP->array, tgtNodeP->next );
    newNodeP->next = tgtNodeP->next;
    newNodeP->prev = tgtIdx;
    tgtNodeP->next = newIdx;
    nodePreviouslyAfterTgt->prev = newIdx;
  }
  else {
    newNodeP->next = tgtNodeP->next;
    newNodeP->prev = tgtIdx;
    tgtNodeP->next = newIdx;
  }
  if ( listP->tail == tgtIdx ) {
    listP->tail = newIdx;
  }
}

void listPrepend( List* listP, ListNodeHeader* newNodeP ) {
  assert ( listP && listP->array && newNodeP );
  Key newNodeIdx = listGetNodeIdx( listP, newNodeP );
  if ( listP->head != UNSET_ ) {
    ListNodeHeader* oldHeadNodeP = arrayGetVoidElemPtr( listP->array, listP->head );
    oldHeadNodeP->prev = newNodeIdx;
    newNodeP->next = listP->head;
    listP->head = newNodeIdx;
  }
  else {
    listP->head = listP->tail =  newNodeIdx;
    newNodeP->next = newNodeP->prev = UNSET_;
  }
}

void listAppend( List* listP, ListNodeHeader* newNodeP ) {
  assert ( listP && listP->array && newNodeP );
  Key newNodeIdx = listGetNodeIdx( listP, newNodeP );
  if ( listP->tail != UNSET_ ) {
    ListNodeHeader* oldHeadNodeP = arrayGetVoidElemPtr( listP->array, listP->tail );
    oldHeadNodeP->next = newNodeIdx;
    newNodeP->prev = listP->tail;
    listP->tail = newNodeIdx;
  }
  else {
    listP->head = listP->tail = newNodeIdx;
    newNodeP->prev = newNodeP->next = UNSET_;  // There's nothing before or after this node.
  }
}

