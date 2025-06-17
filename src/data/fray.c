#include "data/fray.h"

void* frayNew( U32 elemSz, U32 nElems, const PoolId poolId) {
  assert (elemSz && nElems );
  // Add 1 more element for cache-friendly swaps.
  U32* ptr = (U32*) memAdd( (elemSz * (nElems + 1)) + ((N_PREFRAY_ELEMS) * sizeof(U32)), poolId );
  ptr[N_PREFRAY_ELEMS - OFFSET_1ST_INACTIVE]   = 0;       
  ptr[N_PREFRAY_ELEMS - OFFSET_N_PAUSED]   = 0;  
  ptr[N_PREFRAY_ELEMS - OFFSET_1ST_EMPTY]  = 0;       
  ptr[N_PREFRAY_ELEMS - OFFSET_ELEM_SZ]    = elemSz;
  ptr[N_PREFRAY_ELEMS - OFFSET_N_ELEMS]    = nElems;
  return ptr + N_PREFRAY_ELEMS;
}

// Common functionality to both frayAdd() and frayAddEmpty()
// returns a pointer to wherever the new item lands in the fray.
static void* _frayGetFirstEmpty(const void *frayP, U32 *elemNewIdxP) {
  assert (_frayHasRoom(frayP));
  U32 *firstEmptyIdxP = _frayGetFirstEmptyIdxP(frayP);
  if (elemNewIdxP) {
    *elemNewIdxP = *firstEmptyIdxP;  // empty index where new element is going
  }
  return frayGetElemByIdx_(frayP, (*firstEmptyIdxP)++);  
}

/* Checks if the component, wherever it is in the jagged array, is before the function's stopping point in its array. */
// Returns index of added element
void frayAdd(const void *frayP, void *srcElemP, U32 *elemNewIdxP) {
  assert(frayP && srcElemP);
  void *dstP = _frayGetFirstEmpty(frayP, elemNewIdxP);
  memcpy(dstP, srcElemP, arrayGetElemSz(frayP));
}

void frayAddEmpty(const void *frayP, U32 *elemNewIdxP) {
  assert(frayP);
  void *dstP = _frayGetFirstEmpty(frayP, elemNewIdxP);
  memset(dstP, 0, frayGetElemSz_(frayP));
}

// This does NOT check active/paused/inactive boundaries, so use at your own risk.
void fraySwap(const void *frayP, void* elem1P, void* elem2P) {
  // we want to preserve the speed of memcpy over memmove. 
  // The latter has to copy to an empty array first.
  assert(frayP);
  if (elem1P == elem2P ) {
    return;
  }
  // Get source, destination, and placeholder
  void *placeholderP = frayGetElemByIdx_(frayP, frayGetNElems_(frayP));
  U32 elemSz = frayGetElemSz_(frayP);
  memcpy(placeholderP, elem1P,       elemSz);
  memcpy(elem1P,       elem2P,       elemSz);
  memcpy(elem2P,       placeholderP, elemSz);
}

static void _fraySwapByIdx(const void *frayP, U32 oldIdx, U32 newIdx) {
  // We want to preserve the speed of memcpy over memmove. 
  // The latter has to copy to an empty array first.
  assert(frayP);
  if (oldIdx != newIdx ) {
    return;
  }
  void *elem1P       = frayGetElemByIdx_(frayP, oldIdx);
  void *elem2P       = frayGetElemByIdx_(frayP, newIdx); 
  fraySwap( frayP, elem1P, elem2P );
}

// Pausing *active* elements moves them to the first paused position.
// Pausing *inactive* elements moves them to the last paused position.
void frayPause(const void *frayP, U32 idx, FrayChanges *changesP) {
  assert (idx < *_frayGetFirstEmptyIdxP(frayP));
  U32 *nPausedP = _frayGetNPausedP(frayP);
  if (!_frayElemIsPaused(frayP, idx) && *nPausedP < arrayGetNElems(frayP)) {
    U32 *firstInactiveIdxP = _frayGetFirstInactiveIdxP(frayP);
    ++(*nPausedP);
    U32 newIdx;
    if (idx < *firstInactiveIdxP) {
      newIdx = *firstInactiveIdxP - *nPausedP;
    }
    else {
      newIdx = (*firstInactiveIdxP)++;
    }
    _fraySwapByIdx(frayP, idx, newIdx);
    if (changesP) {
      changesP->origIdx = idx;
      changesP->intermediateIdx = 0;
      changesP->newIdx = newIdx;
    }
  }
}

// Unlike pausing, unpausing can only send elements in one direction: leftward into activation state.
void frayUnpause(const void *frayP, U32 idx, FrayChanges *changesP) {
  assert (idx < *_frayGetFirstEmptyIdxP(frayP));
  U32  *nPausedP = _frayGetNPausedP(frayP);
  U32 firstInactiveIdx =  _frayGetFirstInactiveIdx(frayP);
  if (_frayElemIsPaused(frayP, idx) && *nPausedP < firstInactiveIdx) {
    U32 newIdx = firstInactiveIdx - (*nPausedP)--;
    _fraySwapByIdx(frayP, idx, newIdx);
    if (changesP) {
      changesP->origIdx = idx;
      changesP->intermediateIdx = 0;
      changesP->newIdx = newIdx;
    }
  }
}

// Returns new index of activated element 
void frayActivate(const void *frayP, U32 idx, FrayChanges *changesP) {
  assert (frayP && idx < *_frayGetFirstEmptyIdxP(frayP));
  if (!_frayElemIsActive(frayP, idx)) {
    if (_frayElemIsPaused(frayP, idx)) {
      frayUnpause(frayP, idx, changesP);
      return;
    }
    U32 *firstInactiveIdxP = _frayGetFirstInactiveIdxP(frayP);
    U32 nPaused = *_frayGetNPausedP(frayP);
    U32 newIdx;
    U32 intermediateIdx = 0;
    if (!nPaused) {  // With no paused elements, we can blissfully single-swap.
      newIdx = (*firstInactiveIdxP)++;  // swap with first inactive (from left side)
      _fraySwapByIdx(frayP, idx, newIdx);
    }
    else {  // Otherwise, we must double-swap to preserve intermediate paused elems' contiguity.
      intermediateIdx = *firstInactiveIdxP;  // swap with first inactive (from right side)
      _fraySwapByIdx(frayP, idx, intermediateIdx);
      newIdx = (*firstInactiveIdxP)++ - nPaused;  // swap 1st deact. elem with 1st paused
      _fraySwapByIdx(frayP, intermediateIdx, newIdx);
    }
    // Return changes if desired.
    if (changesP) {
      changesP->origIdx = idx;
      changesP->intermediateIdx = intermediateIdx;
      changesP->newIdx = newIdx;
    }
  }
}

// Returns new index of deactivated element 
void frayDeactivate(const void *frayP, U32 idx, FrayChanges *changesP) {
  assert (frayP && idx < *_frayGetFirstEmptyIdxP(frayP));
  if (!_frayElemIsInactive(frayP, idx)) {
    U32 *firstInactiveIdxP = _frayGetFirstInactiveIdxP(frayP);
    U32 *nPausedP = _frayGetNPausedP(frayP);
    U32 newIdx;
    U32 intermediateIdx = 0;
    if (!(*nPausedP)) { // With no paused elements, we can blissfully single-swap.
      newIdx = --(*firstInactiveIdxP);            // swap with last active 
      _fraySwapByIdx(frayP, idx, newIdx);
    }
    else {  // Otherwise, we must double-swap to preserve intermediate paused elems' contiguity.
      intermediateIdx = *firstInactiveIdxP - *nPausedP - 1;  // swap with last active
      if (_frayElemIsPaused(frayP, idx)) {
        --(*nPausedP);
      }
      _fraySwapByIdx(frayP, idx, intermediateIdx);
      newIdx = --(*firstInactiveIdxP);  // swap last active with last paused
      _fraySwapByIdx(frayP, intermediateIdx, newIdx);
    }
    // Return changes if desired.
    if (changesP) {
      changesP->origIdx = idx;
      changesP->intermediateIdx = intermediateIdx;
      changesP->newIdx = newIdx;
    }
  }
}

void frayActivateAll(const void *frayP) {
  U32 *firstInactiveIdxP = _frayGetFirstInactiveIdxP(frayP);
  if (*_frayGetNPausedP(frayP)) {
    _frayUnpauseAll(frayP);
  }
  *firstInactiveIdxP = *_frayGetFirstEmptyIdxP(frayP);
}

void frayDeactivateAll(const void *frayP) {
  if (*_frayGetNPausedP(frayP)) {
    _frayUnpauseAll(frayP);
  }
  *_frayGetFirstInactiveIdxP(frayP) = 0;
}
