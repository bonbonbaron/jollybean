#include "data/list.h"

// Redirect list IDs to a separate list so splitting and merging lists 
// only results in updating the list ID in one place, rather than updating
// the list ID of every member in the merged/split list.

// Returns the index of the node in the raw array it's in.
static U32 _listGetNodeIdx( List* listP, ListNodeHeader* nodeP ) {
  assert( listP && listP->array && nodeP );
  U32 elSz = arrayGetElemSz( listP->array );
  assert( (size_t) nodeP >= (size_t) listP->array );
  assert( (size_t) nodeP <= ( (size_t) listP->array + ( ( arrayGetNElems( listP->array ) - 1 ) * elSz ) ) );
  return ( (size_t) nodeP - (size_t) listP->array ) / elSz;
}

static void _listNodeIni( ListNodeHeader* nodeP) {
  assert( nodeP );
  nodeP->listIdIdx = UNSET_;
  nodeP->next = nodeP->prev = UNSET_;
}

static Key _claimListId( List* listP ) {
  assert( listP );
  assert( listP->metaP );
  assert( listP->metaP->availableIdBitmapA );
  assert( listP->metaP->availableIdIndexBitmapA );
  assert( listP->id == UNSET_ );  // This function is only called when the list is empty.

  S32 firstAvailListId = vbmSetFirstZero( listP->metaP->availableIdBitmapA );
  S32 firstAvailListIdIdx = vbmSetFirstZero( listP->metaP->availableIdIndexBitmapA );

  assert (firstAvailListId >= 0);
  assert (firstAvailListIdIdx >= 0);
  assert (firstAvailListId < UNSET_);
  assert (firstAvailListIdIdx < UNSET_);

  listP->metaP->idA[ firstAvailListIdIdx ] = firstAvailListId;
  listP->id = firstAvailListId;

  return firstAvailListIdIdx;
}

static void _releaseListId( List* listP ) {
  assert( listP );
  assert( listP->metaP );
  assert( listP->metaP->availableIdBitmapA );
  assert( listP->id != UNSET_ );  // This function is only called when the list is empty.

  vbmUnsetBit( listP->metaP->availableIdBitmapA, listP->id );
  listP->id = UNSET_;
}

static void _releaseListIdIdx( List* listP, ListNodeHeader* nodeP ) {
  assert( listP );
  assert( listP->metaP );
  assert( listP->metaP->availableIdIndexBitmapA );

  assert( nodeP );
  assert( nodeP->listIdIdx != UNSET_ );

  vbmUnsetBit( listP->metaP->availableIdIndexBitmapA, nodeP->listIdIdx );
  nodeP->listIdIdx = UNSET_;
}

static void _metadataNew( List* listP, const PoolId poolId ) {
  assert( listP );
  assert( listP->array );
  // Allocate & init list's metadata
  listP->metaP = memAdd( sizeof(ListMetadata), poolId );
  // Make room for as many possible list IDs as there are array elements.
  listP->metaP->idA = arrayNew( sizeof(Key), arrayGetNElems( listP->array ), poolId );
  memset( listP->metaP->idA, UNSET_, arrayGetElemSz( listP->metaP->idA ) * arrayGetNElems( listP->metaP->idA ) );
  // Allocate and init bitmap (only allocating as many words as are required to hold all possible list IDs).
  listP->metaP->availableIdBitmapA = vbmNew( arrayGetNElems(listP->array), poolId );
  listP->metaP->availableIdIndexBitmapA = vbmNew( arrayGetNElems(listP->array), poolId );
}

// Make lists easier to use by initializing all the nodes for the user.
void listIni( List* listP, void* array, const List* leaderListP, const PoolId poolId ) {
  assert( listP && array );
  listP->id = UNSET_;
  listP->head = listP->tail = UNSET_;
  listP->array = array;
  listP->metaP = NULL;
  // All lists for the same array need to point at the same metadata. Leader will initialize it for us.
  if ( !leaderListP ) {
    _metadataNew( listP, poolId );
    const U32 ELEM_SZ = arrayGetElemSz( listP->array );
    const U8* elemEndP = (U8*) listP->array + ( arrayGetNElems( listP->array ) * arrayGetElemSz( listP->array ) );
    for ( U8* elemP = (U8*) listP->array; elemP < elemEndP; elemP += ELEM_SZ ) {
      _listNodeIni ( (ListNodeHeader*) elemP );
    }
  }
  else {
    listP->metaP = leaderListP->metaP;
  }
}

static inline Bln _isInList( const ListNodeHeader* nodeP, const List* listP ) {
  assert( listP );
  assert( listP->metaP );
  return nodeP->listIdIdx != UNSET_ && listP->metaP->idA[nodeP->listIdIdx] == listP->id;
}

// NOTE: This assumes the address of the header is the same as the address of the array element.
void listRemove( List* listP, ListNodeHeader* nodeP ) {
  if ( !_isInList( (const ListNodeHeader*) nodeP, (const List*) listP )) {
    return;
  }
  U32 nodeIdx = _listGetNodeIdx( listP, nodeP );
  if ( listP->head == nodeIdx ) {
    listP->head = nodeP->next;
  }
  if ( listP->tail == nodeIdx ) {
    listP->tail = nodeP->prev;
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
  _releaseListIdIdx( listP, nodeP );
  // If list is empty now, release its ID.
  if ( listP->head == UNSET_ && listP->tail == UNSET_ ) {
    _releaseListId( listP );
  }
}

void listInsertBefore( List* listP, ListNodeHeader* newNodeP, ListNodeHeader* tgtNodeP ) {
  assert ( listP && listP->array && newNodeP );
  if (newNodeP->listIdIdx != UNSET_ ) {
    return;
  }
  newNodeP->listIdIdx = tgtNodeP->listIdIdx;
  Key newIdx = _listGetNodeIdx( listP, newNodeP );
  Key tgtIdx = _listGetNodeIdx( listP, tgtNodeP );
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
  if (newNodeP->listIdIdx != UNSET_ ) {
    return;
  }
  newNodeP->listIdIdx = tgtNodeP->listIdIdx;
  Key newIdx = _listGetNodeIdx( listP, newNodeP );
  Key tgtIdx = _listGetNodeIdx( listP, tgtNodeP );
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
  assert ( !_isInList( newNodeP, listP ) );
  if (newNodeP->listIdIdx != UNSET_ ) {
    return;
  }
  Key newNodeIdx = _listGetNodeIdx( listP, newNodeP );
  if ( listP->head != UNSET_ ) {
    ListNodeHeader* oldHeadNodeP = arrayGetVoidElemPtr( listP->array, listP->head );
    oldHeadNodeP->prev = newNodeIdx;
    newNodeP->next = listP->head;
    listP->head = newNodeIdx;
    newNodeP->listIdIdx = oldHeadNodeP->listIdIdx;
  }
  else {  // if list is empty (has no tail), add new elem as both head and tail with no next or prev. Give it the list's ID.
    listP->head = listP->tail =  newNodeIdx;
    newNodeP->next = newNodeP->prev = UNSET_;
    newNodeP->listIdIdx = _claimListId( listP );  // sets list ID internally.
  }
}

void listAppend( List* listP, ListNodeHeader* newNodeP ) {
  assert ( listP && listP->array && newNodeP );
  assert ( !_isInList( newNodeP, listP ) );
  if (newNodeP->listIdIdx != UNSET_ ) {
    return;
  }
  Key newNodeIdx = _listGetNodeIdx( listP, newNodeP );
  if ( listP->tail != UNSET_ ) {
    ListNodeHeader* oldTailNodeP = arrayGetVoidElemPtr( listP->array, listP->tail );
    oldTailNodeP->next = newNodeIdx;
    newNodeP->prev = listP->tail;
    listP->tail = newNodeIdx;
    newNodeP->listIdIdx = oldTailNodeP->listIdIdx;
  }
  else {  // if list is empty (has no tail), add new elem as both head and tail with no next or prev. Give it the list's ID.
    listP->head = listP->tail = newNodeIdx;
    newNodeP->prev = newNodeP->next = UNSET_;  // There's nothing before or after this node.
    newNodeP->listIdIdx = _claimListId( listP );  // sets list ID internally.
  }
}

// NOTE: This assumes the lists you're merging use the same array. That means there can be no identical node indices.
// listMerge() appends srcList to dstList.
void listMerge( List* srcListP, List* dstListP ) {
  assert ( srcListP && srcListP->head != UNSET_ && srcListP->tail != UNSET_ && dstListP && dstListP->head != UNSET_ && dstListP->tail != UNSET_ );
  assert ( srcListP != dstListP );
  assert ( srcListP->id != dstListP->id );
  assert ( srcListP->array == dstListP->array );
  assert ( srcListP->metaP );
  assert ( dstListP->metaP );
  assert ( srcListP->metaP == dstListP->metaP );  // These BETTER be in the same!
  assert ( dstListP->metaP->idA );  // don't need to check src if prev line is true


  ListNodeHeader* srcHeadP = (ListNodeHeader*) arrayGetVoidElemPtr( srcListP->array, srcListP->head );
  ListNodeHeader* dstOriginalTailP = (ListNodeHeader*) arrayGetVoidElemPtr( dstListP->array, dstListP->tail );

  // Once assertions are made, it's safe to change the list ID that the source list nodes belong to.
  srcListP->metaP->idA[ srcHeadP->listIdIdx ] = dstListP->metaP->idA[ dstOriginalTailP->listIdIdx ];

  // dest list's *new* tail
  srcHeadP->prev = dstListP->tail;
  dstOriginalTailP->next = srcListP->head;
  dstListP->tail = srcListP->tail;

  srcListP->head = srcListP->tail = UNSET_;
  // srcListP->array = NULL;   // <-- Should I even do this if I'm not necessarily deleting the array yet?
  _releaseListId( srcListP );
}

void* listGetHead( List* listP ) {
  assert( listP );
  assert( listP->head != UNSET_ );
  return arrayGetVoidElemPtr( listP->array, listP->head );
}

void* listGetTail( List* listP ) {
  assert( listP );
  assert( listP->tail != UNSET_ );
  return arrayGetVoidElemPtr( listP->array, listP->tail );
}

void* listNodeGetNext( List* listP, ListNodeHeader* nodeP ) {
  assert( listP );
  assert( nodeP );
  assert( nodeP->next != UNSET_ );
  return arrayGetVoidElemPtr( listP->array, nodeP->next );
}

VolatileBitmap* vbmNew( U32 maxBitIdx, const PoolId poolId ) {
  VolatileBitmap* vbmA = arrayNew( sizeof(VolatileBitmap), globalBitToBfIdx_( maxBitIdx ) + 1, poolId );
  memset(vbmA, 0, arrayGetNElems( vbmA ) * arrayGetElemSz( vbmA ) );
  return vbmA;
}

