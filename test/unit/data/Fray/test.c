#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"
#include <assert.h>

static void _printElem(U32 idx, U32 elem) {
  printf("aF[%2d] = %2d\n", idx, elem);
}

static void _reportFray(char *prelude, U32 *frayP) {
  printf("%s:\n", prelude);
  U32 i = 0;
  printf("----- active section -----\n");
  for (; i < _frayGetFirstPausedIdx(frayP); ++i) {
    _printElem(i, frayP[i]);
  }
  printf("----- paused section -----\n");
  for (; i < *_frayGetFirstInactiveIdxP(frayP); ++i) {
    _printElem(i, frayP[i]);
  }
  printf("----- inactive section -----\n");
  for (; i < *_frayGetFirstEmptyIdxP(frayP); ++i) {
    _printElem(i, frayP[i]);
  }
  printf("\n\n");
}

int main(int argc, char **argv) {
  U32 *aF;
  FrayChanges changes;
  frayChangesIni_(changes, -1);
  static const U32 N_ELEMS = 10;
  // Make fray.
  Error e = frayNew((void**) &aF, sizeof(U32), N_ELEMS);
  // Populate fray.
  if (!e) {
    for (U32 i = 0; !e && i < frayGetNElems_(aF); ++i) {
      e = frayAdd(aF, &i, NULL);
    }
  }
  if (e) {
    frayDel((void**) aF);
    return e;
  }
  // Check on it.
  _reportFray("initialized", aF);
  // Activate.
  frayChangesIni_(changes, 2);
  frayActivate(aF, &changes);
  frayChangesIni_(changes, 5);
  frayActivate(aF, &changes);
  frayChangesIni_(changes, 7);
  frayActivate(aF, &changes);
  _reportFray("After activating 2, 5, and 7", aF);
  assert(aF[0] == 2);
  assert(aF[1] == 5);
  assert(aF[2] == 7);
  // Pause.
  frayChangesIni_(changes, 1);
  frayPause(aF, &changes);
  _reportFray("After pausing 5", aF);
  assert(aF[2] == 5);
  // Activate across a pause.
  frayChangesIni_(changes, 9);
  frayActivate(aF, &changes);
  _reportFray("After activating 9 across a pause", aF);
  assert(aF[2] == 9);
  // Deactivate across a pause.
  frayChangesIni_(changes, 2);
  frayDeactivate(aF, &changes);
  frayChangesIni_(changes, 0);
  frayDeactivate(aF, &changes);
  _reportFray("After deactivating 2 and 9 across a pause", aF);
  assert(aF[2] == 2);
  assert(aF[3] == 9);
  assert(_frayElemIsActive(aF, 0));
  assert(_frayElemIsPaused(aF, 1));
  assert(!_frayElemIsActive(aF, 2));
  assert(aF[0] == 7);
  assert(aF[1] == 5);
  assert(aF[2] == 2);
  // Unpause.
  frayChangesIni_(changes, 1);
  frayUnpause(aF, &changes);
  assert(!_frayElemIsPaused(aF, 1));
  _reportFray("After unpausing 5", aF);
  // Pause multiple.
  frayChangesIni_(changes, 5);
  frayPause(aF, &changes);
  frayChangesIni_(changes, 6);
  frayPause(aF, &changes);
  frayChangesIni_(changes, 7);
  frayPause(aF, &changes);
  _reportFray("Pausing 1, 6, and 0", aF);
  assert(aF[2] == 1);
  assert(aF[3] == 6);
  assert(aF[4] == 0);
  // Unpause all.
  _frayUnpauseAll(aF);
  _reportFray("Unpausing all", aF);
  assert(_frayGetFirstPausedIdx(aF) == _frayGetFirstInactiveIdx(aF));
  // Pause all before activate-all.
  _frayPauseAll(aF);
  _reportFray("Pausing all", aF);
  assert(_frayGetFirstPausedIdx(aF) == 0);
  // Activate all.
  _frayActivateAll(aF);
  _reportFray("After activating all", aF);
  assert(_frayElemIsActive(aF, 0));
  assert(_frayElemIsActive(aF, N_ELEMS - 1));
  // Pause all again before deactivate-all.
  _frayPauseAll(aF);
  _reportFray("Pausing all", aF);
  assert(_frayGetFirstPausedIdx(aF) == 0);
  // Deactivate all after pausing multiple.
  _frayDeactivateAll(aF);
  assert(!_frayElemIsActive(aF, 0));
  assert(!_frayElemIsActive(aF, N_ELEMS - 1));
  _reportFray("After deactivating all", aF);
  // Test a clear.
  _frayClr(aF);
  assert(*_frayGetFirstEmptyIdxP(aF) == 0);
  assert(*_frayGetNPausedP(aF) == 0);
  assert(*_frayGetFirstInactiveIdxP(aF) == 0);
  assert(frayGetNElems_(aF) == N_ELEMS);
  assert(frayGetElemSz_(aF) == sizeof(U32));


  frayDel((void**) &aF);
  
  return e;
}
