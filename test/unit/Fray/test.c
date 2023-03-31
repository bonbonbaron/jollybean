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
  // Test an activation.
  frayActivate(aF, 2);
  frayActivate(aF, 5);
  frayActivate(aF, 7);
  _reportFray("After activating 2, 5, and 7", aF);
  // Test a pause.
  frayPause(aF, 1);
  _reportFray("After pausing 5", aF);
  // Test a deactivation.
  frayDeactivate(aF, 0);
  _reportFray("After deactivating 2", aF);
  assert(_frayElemIsActive(aF, 0));
  assert(_frayElemIsPaused(aF, 1));
  assert(!_frayElemIsActive(aF, 2));
  assert(aF[0] == 7);
  assert(aF[1] == 5);
  assert(aF[2] == 2);
  // Test an unpause.
  frayUnpause(aF, 1);
  assert(!_frayElemIsPaused(aF, 1));
  _reportFray("After unpausing 5", aF);
  // Test an activate-all.
  frayActivateAll(aF);
  assert(_frayElemIsActive(aF, 0));
  assert(_frayElemIsActive(aF, N_ELEMS - 1));
  _reportFray("After activating all", aF);
  // Test a deactivate-all.
  frayDeactivateAll(aF);
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
