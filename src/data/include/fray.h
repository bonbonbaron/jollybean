#ifndef FRAY_H
#define FRAY_H

#include "array.h"

// Efficient Arrays (frays)
#define N_PREFRAY_ELEMS (5)
#define OFFSET_1ST_INACTIVE (5)  /* ptr[0] */
#define OFFSET_N_PAUSED     (4)  /* ptr[1] */
#define OFFSET_1ST_EMPTY    (3)  /* ptr[2] */
#define OFFSET_ELEM_SZ      (2)  /* ptr[3] */
#define OFFSET_N_ELEMS      (1)  /* ptr[4] */
#define frayGetNElems_ arrayGetNElems
#define frayGetElemSz_ arrayGetElemSz
#define frayGetElemByIdx_ _fast_arrayGetElemByIdx

typedef struct {
  S32 origIdx;
  S32 intermediateIdx;
  S32 newIdx;
} FrayChanges;

#define frayChangesIni_(fc_, origIdx_) \
  fc_.origIdx = origIdx_;\
  fc_.intermediateIdx = -1;\
  fc_.newIdx = -1;

Error frayNew(void **fPP, U32 elemSz, U32 nElems);
void  frayDel(void **frayPP);
Error frayAdd(const void *frayP, void *elemP, U32 *elemNewIdxP);
Error frayAddEmpty(const void *frayP, U32 *elemNewIdxP);
Error frayActivate(const void *frayP, U32 idx, FrayChanges *changesP);
Error frayDeactivate(const void *frayP, U32 idx, FrayChanges *changesP);
Error frayPause(const void *frayP, U32 idx, FrayChanges *changesP);
Error frayUnpause(const void *frayP, U32 idx, FrayChanges *changesP);
U8    frayElemIsActive(const void *frayP, U32 idx);
void  frayActivateAll(const void *frayP);
void  frayDeactivateAll(const void *frayP);

// Pointers beat values. We usually inc/decrement it after using it. Avoids double-queries.
inline U32 _frayGetFirstInactiveIdx(const void *frayP) {
  return *(((U32*) frayP - OFFSET_1ST_INACTIVE));
}

inline U32* _frayGetFirstInactiveIdxP(const void *frayP) {
  return ((U32*) frayP - OFFSET_1ST_INACTIVE);
}

inline U32* _frayGetNPausedP(const void *frayP) {
  return ((U32*) frayP - OFFSET_N_PAUSED);
}

inline U32 _frayGetNActive(const void *frayP) {
  return _frayGetFirstInactiveIdx(frayP) - *_frayGetNPausedP(frayP);
}

inline U32 _frayGetFirstPausedIdx(const void *frayP) {
  return *_frayGetFirstInactiveIdxP(frayP) - *_frayGetNPausedP(frayP);
}

inline U32* _frayGetFirstEmptyIdxP(const void *frayP) {
  return ((U32*) frayP - OFFSET_1ST_EMPTY);
}

inline void _frayClr(void *fP) {
  memset(((U32*) fP) - N_PREFRAY_ELEMS, 0, sizeof(U32) * 3);
  /*  The above has this effect:
      *_frayGetFirstEmptyIdxP(fP) = 0;
      *_frayGetNPausedP(fP) = 0;
      *_frayGetFirstInactiveIdxP(fP) = 0;

      This is faster than zeroing out the whole fray.
  */
}

inline U8 _frayElemIsActive(const void *frayP, U32 idx) {
  return idx < _frayGetFirstPausedIdx(frayP);
}

inline U8 _frayElemIsInactive(const void *frayP, U32 idx) {
  return idx >= _frayGetFirstInactiveIdx(frayP);
}

inline U8 _frayElemIsPaused(const void *frayP, U32 idx) {
  return (*_frayGetNPausedP(frayP) > 0) && idx >= _frayGetFirstPausedIdx(frayP) && idx <  _frayGetFirstInactiveIdx(frayP);
}

inline void _frayPauseAll(const void *frayP) {
  *_frayGetNPausedP(frayP) = _frayGetFirstInactiveIdx(frayP);
}

inline void _frayUnpauseAll(const void *frayP) {
  *_frayGetNPausedP(frayP) = 0;
}

inline U8 _frayHasRoom(const void *frayP) {
  return (*_frayGetFirstEmptyIdxP(frayP) < frayGetNElems_(frayP));
}
#endif  // #ifndef FRAY_H
