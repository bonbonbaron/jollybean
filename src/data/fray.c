#include "fray.h"

Error frayNew(void **fPP, U32 elemSz, U32 nElems) {
  Error e = SUCCESS;
  U32 *ptr;
	if (elemSz <= 0 || nElems <= 0 || fPP == NULL) {
		return E_BAD_ARGS;  
  }
	else {
    // Add 1 more element for swaps. 
		e = jbAlloc((void**) &ptr, (elemSz * (nElems + 1)) + ((N_PREFRAY_ELEMS) * sizeof(U32)), 1);
    if (!e) {
      ptr[N_PREFRAY_ELEMS - OFFSET_1ST_INACTIVE]   = 0;       
      ptr[N_PREFRAY_ELEMS - OFFSET_N_PAUSED]   = 0;  
      ptr[N_PREFRAY_ELEMS - OFFSET_1ST_EMPTY]  = 0;       
      ptr[N_PREFRAY_ELEMS - OFFSET_ELEM_SZ]    = elemSz;
      ptr[N_PREFRAY_ELEMS - OFFSET_N_ELEMS]    = nElems;
      *fPP = (ptr + N_PREFRAY_ELEMS);
      memset(*fPP, 0, elemSz * nElems);
    }
  }
  return e;
}

void frayDel(void **frayPP) {
	if (frayPP != NULL && *frayPP != NULL) {
		U32 *ptr = *frayPP;
		free((ptr) - N_PREFRAY_ELEMS);
		*frayPP = NULL;
	}
}

// Common functionality to both frayAdd() and frayAddEmpty()
static Error _frayAdd(const void *frayP, void **dstPP, U32 *elemNewIdxP) {
  if (!_frayHasRoom(frayP)) {
    return E_FRAY_FULL;
  }
  U32 *firstEmptyIdxP = _frayGetFirstEmptyIdxP(frayP);
  if (elemNewIdxP) {
    *elemNewIdxP = *firstEmptyIdxP;  // empty index where new element is going
  }
  *dstPP = frayGetElemByIdx_(frayP, (*firstEmptyIdxP)++);
  return SUCCESS;
}

/* Checks if the component, wherever it is in the jagged array, is before the function's stopping point in its array. */
// Returns index of added element
Error frayAdd(const void *frayP, void *elemP, U32 *elemNewIdxP) {
  void *dstP;
  Error e = _frayAdd(frayP, &dstP, elemNewIdxP);
  memcpy(dstP, elemP, arrayGetElemSz(frayP));
  return e;
}

Error frayAddEmpty(const void *frayP, U32 *elemNewIdxP) {
  void *dstP;
  Error e = _frayAdd(frayP, &dstP, elemNewIdxP);
  memset(dstP, 0, frayGetElemSz_(frayP));
  return e;
}

static void _fraySwap(const void *frayP, U32 oldIdx, U32 newIdx) {
  // Get source, destination, and placeholder
  register void *elem1P       = frayGetElemByIdx_(frayP, oldIdx);
  register void *placeholderP = frayGetElemByIdx_(frayP, frayGetNElems_(frayP));
  register void *elem2P       = frayGetElemByIdx_(frayP, newIdx); 
  // Swap with the first inactive.
  register U32   elemSz       = frayGetElemSz_(frayP);
  memcpy(placeholderP, elem1P,       elemSz);
  memcpy(elem1P,       elem2P,       elemSz);
  memcpy(elem2P,       placeholderP, elemSz);
}

// Pausing *active* elements moves them to the first paused position.
// Pausing *inactive* elements moves them to the last paused position.
Error frayPause(const void *frayP, U32 idx, FrayChanges *changesP) {
  if (idx >= *_frayGetFirstEmptyIdxP(frayP)) {
    return E_FRAY_SEGFAULT;
  }
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
    _fraySwap(frayP, idx, newIdx);
    if (changesP) {
      changesP->origIdx = idx;
      changesP->intermediateIdx = 0;
      changesP->newIdx = newIdx;
    }
  }
  return SUCCESS;
}

// Unlike pausing, unpausing can only send elements in one direction: leftward into activation state.
Error frayUnpause(const void *frayP, U32 idx, FrayChanges *changesP) {
  if (idx >= *_frayGetFirstEmptyIdxP(frayP)) {
    return E_FRAY_SEGFAULT;
  }
  register U32  *nPausedP = _frayGetNPausedP(frayP);
  U32 firstInactiveIdx =  _frayGetFirstInactiveIdx(frayP);
  if (_frayElemIsPaused(frayP, idx) && *nPausedP < firstInactiveIdx) {
    U32 newIdx = firstInactiveIdx - (*nPausedP)--;
    _fraySwap(frayP, idx, newIdx);
    if (changesP) {
      changesP->origIdx = idx;
      changesP->intermediateIdx = 0;
      changesP->newIdx = newIdx;
    }
  }
  return SUCCESS;
}

// Returns new index of activated element 
Error frayActivate(const void *frayP, U32 idx, FrayChanges *changesP) {
  if (idx >= *_frayGetFirstEmptyIdxP(frayP)) {
    return E_FRAY_SEGFAULT;
  }
  if (!_frayElemIsActive(frayP, idx)) {
    if (_frayElemIsPaused(frayP, idx)) {
      frayUnpause(frayP, idx, changesP);
      return SUCCESS;
    }
    U32 *firstInactiveIdxP = _frayGetFirstInactiveIdxP(frayP);
    U32 nPaused = *_frayGetNPausedP(frayP);
    U32 newIdx;
    U32 intermediateIdx = 0;
    if (!nPaused) {  // With no paused elements, we can blissfully single-swap.
      newIdx = (*firstInactiveIdxP)++;  // swap with first inactive (from left side)
      _fraySwap(frayP, idx, newIdx);
    }
    else {  // Otherwise, we must double-swap to preserve intermediate paused elems' contiguity.
      intermediateIdx = *firstInactiveIdxP;  // swap with first inactive (from right side)
      _fraySwap(frayP, idx, intermediateIdx);
      newIdx = (*firstInactiveIdxP)++ - nPaused;  // swap 1st deact. elem with 1st paused
      _fraySwap(frayP, intermediateIdx, newIdx);
    }
    // Return changes if desired.
    if (changesP) {
      changesP->origIdx = idx;
      changesP->intermediateIdx = intermediateIdx;
      changesP->newIdx = newIdx;
    }
  }
  return SUCCESS;
}

// Returns new index of deactivated element 
Error frayDeactivate(const void *frayP, U32 idx, FrayChanges *changesP) {
  if (idx >= *_frayGetFirstEmptyIdxP(frayP)) {
    return E_FRAY_SEGFAULT;
  }
  if (!_frayElemIsInactive(frayP, idx)) {
    U32 *firstInactiveIdxP = _frayGetFirstInactiveIdxP(frayP);
    U32 *nPausedP = _frayGetNPausedP(frayP);
    U32 newIdx;
    U32 intermediateIdx = 0;
    if (!(*nPausedP)) { // With no paused elements, we can blissfully single-swap.
      newIdx = --(*firstInactiveIdxP);            // swap with last active 
      _fraySwap(frayP, idx, newIdx);
    }
    else {  // Otherwise, we must double-swap to preserve intermediate paused elems' contiguity.
      intermediateIdx = *firstInactiveIdxP - *nPausedP - 1;  // swap with last active
      if (_frayElemIsPaused(frayP, idx)) {
        --(*nPausedP);
      }
      _fraySwap(frayP, idx, intermediateIdx);
      newIdx = --(*firstInactiveIdxP);  // swap last active with last paused
      _fraySwap(frayP, intermediateIdx, newIdx);
    }
    // Return changes if desired.
    if (changesP) {
      changesP->origIdx = idx;
      changesP->intermediateIdx = intermediateIdx;
      changesP->newIdx = newIdx;
    }
  }
  return SUCCESS;
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
