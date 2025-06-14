#include "tau.h"
#include "data/fray.h"


TAU_MAIN()

typedef struct Tau {
  U32 *uF;
  U32 nElems;
  U32 elemSz;
  U32 iniPop;
  U32 newIdx;
  U32 newVal;
  U32 iniActive;
  FrayChanges fc;
} Tau;

TEST_F_SETUP(Tau) {
  memIni( 1000, MAIN );
  memset(tau, 0, sizeof(Tau));
  tau->nElems = 50;
  tau->elemSz = sizeof(U32);
  tau->newVal = 500;
  tau->iniPop = 20;
  tau->iniActive = 5;

  tau->uF = frayNew( tau->elemSz, tau->nElems, MAIN);

  REQUIRE_EQ(frayGetElemSz_(tau->uF), tau->elemSz);
  REQUIRE_EQ(frayGetNElems_(tau->uF), tau->nElems);
  REQUIRE_EQ(_frayGetFirstPausedIdx(tau->uF), 0);
  REQUIRE_EQ(_frayGetFirstInactiveIdx(tau->uF), 0);
  REQUIRE_EQ(*_frayGetNPausedP(tau->uF), 0);
  REQUIRE_EQ(*_frayGetFirstEmptyIdxP(tau->uF), 0);
  REQUIRE_EQ(*_frayGetFirstInactiveIdxP(tau->uF), 0);

  // Populate the fray. Check that the returned index is as expected (equal to the element itself).
  for (U32 i = 0, j; i < tau->iniPop; ++i) {
    frayAdd((void*) tau->uF, (void*) &i, &j);
    REQUIRE_EQ(j, i);
  }
  REQUIRE_EQ(*_frayGetFirstEmptyIdxP(tau->uF), tau->iniPop);

  // Activate 5 of 20 populated (of 50 capacity) elems by default.
  for (U32 i = 0; i < tau->iniActive; ++i) {
    frayActivate(tau->uF, i * 2, NULL);  // going from 0 - 8 by even numbers.
  }
  REQUIRE_EQ(_frayGetFirstPausedIdx((void*) tau->uF), tau->iniActive);
}

TEST_F_TEARDOWN(Tau) {
  memClr( MAIN );
}

// Unit tests

TEST_F(Tau, frayAdd) {
  for (U32 currNElems = tau->iniPop; currNElems < tau->nElems; ++currNElems, ++tau->newVal) {
    frayAdd((void*) tau->uF, (void*) &tau->newVal, &tau->newIdx);
    CHECK_EQ(tau->uF[tau->newIdx], tau->newVal);  // go till we get the overflow
  }
  CHECK_EQ(tau->newIdx, tau->nElems - 1);
  CHECK_EQ(*_frayGetFirstEmptyIdxP(tau->uF), tau->nElems);
}

TEST_F(Tau, frayAddEmpty) {
  for (U32 currNElems = tau->iniPop; currNElems < tau->nElems; ++currNElems, ++tau->newVal) {
    frayAddEmpty((void*) tau->uF, &tau->newIdx);
    CHECK_EQ(tau->uF[tau->newIdx], 0);
  }
  CHECK_EQ(*_frayGetFirstEmptyIdxP(tau->uF), frayGetNElems_(tau->uF));
}

TEST_F(Tau, frayPause_ActiveElems) {
  // Pause every third active element.
  for (U32 i = 0, nPaused = 1; i < tau->iniActive; i += 3, ++nPaused) {
    // Ini vars.
    U32 srcVal = tau->uF[i];
    U32 dstVal = tau->uF[--tau->iniActive];
    // Do the pause.
    frayPause(tau->uF, i, &tau->fc);
    // Check indices of moved data.
    CHECK_EQ(tau->fc.origIdx, i);
    CHECK_EQ(tau->fc.intermediateIdx, 0);
    CHECK_EQ(tau->fc.newIdx, tau->iniActive);
    // Check values of moved data.
    CHECK_EQ(tau->uF[tau->fc.origIdx], dstVal);
    CHECK_EQ(tau->uF[tau->fc.newIdx], srcVal);
    CHECK_EQ(*_frayGetNPausedP(tau->uF), nPaused);
  }
}

TEST_F(Tau, frayPause_InactiveElems) {
  // Pause every third inactive element, starting from the end.
  for (U32 i = *_frayGetFirstEmptyIdxP(tau->uF) - 1, nPaused = 1; i > tau->iniActive; i -= 3, ++nPaused) {
    // Ini vars.
    U32 srcVal = tau->uF[i];
    U32 dstVal = tau->uF[tau->iniActive];
    // Do the pause.
    frayPause(tau->uF, i, &tau->fc);
    // Check indices of moved data.
    CHECK_EQ(tau->fc.origIdx, i);
    CHECK_EQ(tau->fc.intermediateIdx, 0);
    CHECK_EQ(tau->fc.newIdx, tau->iniActive++);
    // Check values of moved data.
    CHECK_EQ(tau->uF[tau->fc.origIdx], dstVal);
    CHECK_EQ(tau->uF[tau->fc.newIdx], srcVal);
    CHECK_EQ(*_frayGetNPausedP(tau->uF), nPaused);
  }
}

TEST_F(Tau, frayPause_AlreadyPausedElems) {
  U32 idx = _frayGetFirstInactiveIdx(tau->uF);
  U32 val = tau->uF[idx];
  // Pause something
  frayPause(tau->uF, idx, &tau->fc);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 1);
  CHECK_EQ(tau->uF[idx], val);
  // Pause the same element again
  memset(&tau->fc, 0, sizeof(FrayChanges));
  frayPause(tau->uF, idx, &tau->fc);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 1);
  CHECK_EQ(tau->uF[idx], val);
  CHECK_EQ(tau->fc.origIdx, tau->fc.newIdx);
}

TEST_F(Tau, frayPauseAll) {
  _frayPauseAll(tau->uF);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), tau->iniActive);
  CHECK_TRUE(_frayElemIsPaused(tau->uF, 0));
}

TEST_F(Tau, frayUnpause) {
  frayPause(tau->uF, tau->iniActive, &tau->fc);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 1);
  CHECK_EQ(tau->fc.origIdx, tau->fc.newIdx);
  frayUnpause(tau->uF, tau->fc.newIdx, &tau->fc);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 0);
  CHECK_EQ(tau->fc.origIdx, tau->fc.newIdx);
}

TEST_F(Tau, frayUnpause_Active) {
  frayUnpause(tau->uF, 0, &tau->fc);
  CHECK_EQ(tau->fc.origIdx, tau->fc.newIdx);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 0);
}

TEST_F(Tau, frayUnpause_Inactive) {
  frayUnpause(tau->uF, tau->iniPop - 1, &tau->fc);
  // Changes are all zeros when nothing happens.
  CHECK_EQ(tau->fc.origIdx, 0);
  CHECK_EQ(tau->fc.intermediateIdx, 0);
  CHECK_EQ(tau->fc.newIdx, 0);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 0);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive);
}

TEST_F(Tau, frayUnpauseAll) {
  // Pause first 3 elements.
  for (U32 i = 0; i < 3; ++i) {
    frayPause(tau->uF, i, NULL);
  }
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 3);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive - 3);
  // Unpause all of them.
  _frayUnpauseAll(tau->uF);
  CHECK_TRUE(!_frayElemIsPaused(tau->uF, 0));
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive);
}

TEST_F(Tau, frayActivate_StoreChanges) {
  U32 origIdx = tau->iniActive + 2;
  U32 newIdx  = tau->iniActive;
  frayActivate(tau->uF, origIdx, &tau->fc);
  CHECK_EQ(_frayGetFirstPausedIdx(tau->uF), tau->iniActive + 1);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive + 1);
  CHECK_TRUE(tau->fc.origIdx == origIdx);
  CHECK_TRUE(tau->fc.intermediateIdx == 0);
  CHECK_TRUE(tau->fc.newIdx== newIdx);
}

TEST_F(Tau, frayActivate_DontStoreChanges) {
  frayActivate(tau->uF, tau->iniActive + 2, NULL);
  CHECK_EQ(_frayGetFirstPausedIdx(tau->uF), tau->iniActive + 1);
  CHECK_TRUE(tau->fc.origIdx == 
             tau->fc.intermediateIdx == 
             tau->fc.newIdx== 0);
}

TEST_F(Tau, frayActivate_PausedElement) {
  // First pause an element.
  frayPause(tau->uF, tau->iniActive, &tau->fc);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 1);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive);
  CHECK_EQ(tau->fc.origIdx, tau->fc.newIdx);
  // Then activate it.
  frayActivate(tau->uF, tau->fc.newIdx, &tau->fc);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive + 1);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 0);
  CHECK_EQ(tau->fc.origIdx, tau->fc.newIdx);
}

TEST_F(Tau, frayActivate_AcrossPause) {
  // First pause an element.
  frayPause(tau->uF, tau->iniActive, &tau->fc);
  CHECK_TRUE(_frayElemIsPaused(tau->uF, tau->fc.newIdx));
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 1);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive);
  CHECK_EQ(tau->fc.origIdx, tau->fc.newIdx);
  // Then activate an inactive one beyond it.
  U32 origVal = tau->uF[tau->iniPop - 1];
  frayActivate(tau->uF, tau->iniPop - 1, &tau->fc);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 1);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive + 1);
  CHECK_EQ(tau->uF[tau->fc.newIdx], origVal);
  CHECK_TRUE(_frayElemIsActive(tau->uF, tau->fc.newIdx));
  CHECK_TRUE(!_frayElemIsActive(tau->uF, tau->fc.origIdx));
}

TEST_F(Tau, frayActivateAll) {
  frayActivateAll(tau->uF);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniPop);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 0);
  CHECK_EQ(_frayGetFirstPausedIdx(tau->uF), tau->iniPop);
  CHECK_EQ(_frayGetFirstInactiveIdx(tau->uF), tau->iniPop);
  CHECK_EQ(*_frayGetFirstInactiveIdxP(tau->uF), tau->iniPop);
}

TEST_F(Tau, frayActivateAll_WithPauses) {
  frayPause(tau->uF, tau->iniPop - 1, &tau->fc);
  CHECK_TRUE(_frayElemIsPaused(tau->uF, tau->fc.newIdx));
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 1);
  frayActivateAll(tau->uF);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniPop);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 0);
  CHECK_EQ(_frayGetFirstPausedIdx(tau->uF), tau->iniPop);
  CHECK_EQ(_frayGetFirstInactiveIdx(tau->uF), tau->iniPop);
  CHECK_EQ(*_frayGetFirstInactiveIdxP(tau->uF), tau->iniPop);
  REQUIRE_TRUE(1);
}

TEST_F(Tau, frayDeactivate) {
  U32 origVal = tau->uF[0];
  frayDeactivate(tau->uF, 0, &tau->fc);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive - 1);
  CHECK_EQ(tau->uF[tau->fc.newIdx], origVal);
}

TEST_F(Tau, frayDeactivate_PausedElement) {
  // Pause last inactive element.
  frayPause(tau->uF, tau->iniPop - 1, &tau->fc);
  CHECK_TRUE(_frayElemIsPaused(tau->uF, tau->fc.newIdx));
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 1);
  // Deactivate paused element.
  frayDeactivate(tau->uF, tau->fc.newIdx, &tau->fc);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive);  // exp 5, actual 4
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 0);
}

TEST_F(Tau, frayDeactivate_AcrossPause) {
  // Pause last element.
  frayPause(tau->uF, tau->iniPop - 1, &tau->fc);
  CHECK_TRUE(_frayElemIsPaused(tau->uF, tau->fc.newIdx));
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 1);
  // Deactivate first element.
  frayDeactivate(tau->uF, 0, &tau->fc);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive - 1);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 1);
}

TEST_F(Tau, frayDeactivateAll) {
  frayDeactivateAll(tau->uF);
  CHECK_EQ(_frayGetNActive(tau->uF), 0);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 0);
  CHECK_EQ(*_frayGetFirstEmptyIdxP(tau->uF), tau->iniPop);
  CHECK_EQ(_frayGetFirstPausedIdx(tau->uF), 0);
  CHECK_EQ(_frayGetFirstInactiveIdx(tau->uF), 0);
  CHECK_EQ(*_frayGetFirstInactiveIdxP(tau->uF), 0);
}

TEST_F(Tau, frayDeactivateAll_WithPauses) {
  // Pause last element.
  frayPause(tau->uF, tau->iniPop - 1, &tau->fc);
  CHECK_TRUE(_frayElemIsPaused(tau->uF, tau->fc.newIdx));
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 1);
  // Deactivate all.
  frayDeactivateAll(tau->uF);
  CHECK_EQ(_frayGetNActive(tau->uF), 0);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 0);
  CHECK_EQ(*_frayGetFirstEmptyIdxP(tau->uF), tau->iniPop);
  CHECK_EQ(_frayGetFirstPausedIdx(tau->uF), 0);
  CHECK_EQ(_frayGetFirstInactiveIdx(tau->uF), 0);
  CHECK_EQ(*_frayGetFirstInactiveIdxP(tau->uF), 0);
}

TEST_F(Tau, frayClr) {
  _frayClr(tau->uF);
  CHECK_EQ(*_frayGetFirstEmptyIdxP(tau->uF), 0);
  CHECK_EQ(_frayGetFirstPausedIdx(tau->uF), 0);
}

TEST_F(Tau, frayActivateThenDeactivateFirstElement) {
  // Deactvate everything
  frayDeactivateAll(tau->uF);
  CHECK_FALSE(_frayElemIsActive(tau->uF, 0));
  // Activate
  frayActivate(tau->uF, 0, &tau->fc);
  // Make sure it's active
  CHECK_EQ(tau->uF[0], 0); // this is failing
  CHECK_TRUE(_frayElemIsActive(tau->uF, 0));
  // Deactivate
  frayDeactivate(tau->uF, 0, &tau->fc);
  // Make sure it's inactive
  CHECK_FALSE(_frayElemIsActive(tau->uF, 0)); // this is failing
  CHECK_EQ(tau->uF[0], 0); // this is failing
}
