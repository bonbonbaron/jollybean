#include "tau/tau.h"
#include "fray.h"


TAU_MAIN()

typedef struct Tau {
  Error e;
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
  memset(tau, 0, sizeof(Tau));
  tau->nElems = 50;
  tau->elemSz = sizeof(U32);
  tau->newVal = 500;
  tau->iniPop = 20;
  tau->iniActive = 5;

  tau->e = frayNew((void**) &tau->uF, tau->elemSz, tau->nElems);

  REQUIRE_EQ(tau->e, SUCCESS);
  REQUIRE_EQ(frayGetElemSz_(tau->uF), tau->elemSz);
  REQUIRE_EQ(frayGetNElems_(tau->uF), tau->nElems);
  REQUIRE_EQ(_frayGetFirstPausedIdx(tau->uF), 0);
  REQUIRE_EQ(_frayGetFirstInactiveIdx(tau->uF), 0);
  REQUIRE_EQ(*_frayGetNPausedP(tau->uF), 0);
  REQUIRE_EQ(*_frayGetFirstEmptyIdxP(tau->uF), 0);
  REQUIRE_EQ(*_frayGetFirstInactiveIdxP(tau->uF), 0);

  // Populate the fray. Check that the returned index is as expected (equal to the element itself).
  for (U32 i = 0, j; !tau->e && i < tau->iniPop; ++i) {
    tau->e = frayAdd((void*) tau->uF, (void*) &i, &j);
    REQUIRE_EQ(j, i);
  }
  REQUIRE_EQ(tau->e, SUCCESS);
  REQUIRE_EQ(*_frayGetFirstEmptyIdxP(tau->uF), tau->iniPop);

  // Activate 5 of 20 populated (of 50 capacity) elems by default.
  for (U32 i = 0; !tau->e && i < tau->iniActive; ++i) {
    tau->e = frayActivate(tau->uF, i * 2, NULL);  // going from 0 - 8 by even numbers.
  }
  REQUIRE_EQ(tau->e, SUCCESS);
  REQUIRE_EQ(_frayGetFirstPausedIdx((void*) tau->uF), tau->iniActive);
}

TEST_F_TEARDOWN(Tau) {
  frayDel((void**) &tau->uF);
  REQUIRE_TRUE(tau->uF == NULL);
}

// Unit tests

TEST_F(Tau, frayNew_BadArgs) {
  frayDel((void**) &tau->uF);
  CHECK_TRUE(tau->uF == NULL);
  tau->e = frayNew((void**) &tau->uF, 0, 1);
  CHECK_EQ(tau->e, E_BAD_ARGS);
  CHECK_TRUE(tau->uF == NULL);
  tau->e = frayNew((void**) &tau->uF, 0, 1);
  CHECK_EQ(tau->e, E_BAD_ARGS);
  CHECK_TRUE(tau->uF == NULL);
}

TEST_F(Tau, frayAdd) {
  for (; !tau->e; ++tau->newVal) {
    tau->e = frayAdd((void*) tau->uF, (void*) &tau->newVal, &tau->newIdx);
    CHECK_EQ(tau->uF[tau->newIdx], tau->newVal);
  }
  CHECK_EQ(tau->e, E_FRAY_FULL);
  CHECK_EQ(tau->newIdx, tau->nElems - 1);
  CHECK_EQ(*_frayGetFirstEmptyIdxP(tau->uF), tau->nElems);
}

TEST_F(Tau, frayAddEmpty) {
  for (; !tau->e; ++tau->newVal) {
    tau->e = frayAddEmpty((void*) tau->uF, &tau->newIdx);
    CHECK_EQ(tau->uF[tau->newIdx], 0);
  }
  CHECK_EQ(tau->e, E_FRAY_FULL);
  CHECK_EQ(*_frayGetFirstEmptyIdxP(tau->uF), frayGetNElems_(tau->uF));
}

TEST_F(Tau, frayPause_ActiveElems) {
  // Pause every third active element.
  for (U32 i = 0, nPaused = 1; !tau->e && i < tau->iniActive; i += 3, ++nPaused) {
    // Ini vars.
    U32 srcVal = tau->uF[i];
    U32 dstVal = tau->uF[--tau->iniActive];
    // Do the pause.
    frayPause(tau->uF, i, &tau->fc);
    if (!tau->e) {
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
}

TEST_F(Tau, frayPause_InactiveElems) {
  // Pause every third inactive element, starting from the end.
  for (U32 i = *_frayGetFirstEmptyIdxP(tau->uF) - 1, nPaused = 1; !tau->e && i > tau->iniActive; i -= 3, ++nPaused) {
    // Ini vars.
    U32 srcVal = tau->uF[i];
    U32 dstVal = tau->uF[tau->iniActive];
    // Do the pause.
    tau->e = frayPause(tau->uF, i, &tau->fc);
    if (!tau->e) {
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
  CHECK_EQ(tau->e, SUCCESS);
}

TEST_F(Tau, frayPause_AlreadyPausedElems) {
  U32 idx = _frayGetFirstInactiveIdx(tau->uF);
  U32 val = tau->uF[idx];
  // Pause something
  tau->e = frayPause(tau->uF, idx, &tau->fc);
  CHECK_EQ(tau->e, SUCCESS);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 1);
  CHECK_EQ(tau->uF[idx], val);
  // Pause the same element again
  memset(&tau->fc, 0, sizeof(FrayChanges));
  tau->e = frayPause(tau->uF, idx, &tau->fc);
  CHECK_EQ(tau->e, SUCCESS);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 1);
  CHECK_EQ(tau->uF[idx], val);
  CHECK_EQ(tau->fc.origIdx, tau->fc.newIdx);
}

TEST_F(Tau, frayPause_Segfault) {
  tau->e = frayPause(tau->uF, tau->nElems, NULL);
  CHECK_EQ(tau->e, E_FRAY_SEGFAULT);
}

TEST_F(Tau, frayPauseAll) {
  _frayPauseAll(tau->uF);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), tau->iniActive);
  CHECK_TRUE(_frayElemIsPaused(tau->uF, 0));
}

TEST_F(Tau, frayUnpause) {
  tau->e = frayPause(tau->uF, tau->iniActive, &tau->fc);
  CHECK_EQ(tau->e, SUCCESS);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 1);
  CHECK_EQ(tau->fc.origIdx, tau->fc.newIdx);
  if (!tau->e) {
    tau->e = frayUnpause(tau->uF, tau->fc.newIdx, &tau->fc);
    CHECK_EQ(tau->e, SUCCESS);
    CHECK_EQ(*_frayGetNPausedP(tau->uF), 0);
    CHECK_EQ(tau->fc.origIdx, tau->fc.newIdx);
  }
}

TEST_F(Tau, frayUnpause_Active) {
  tau->e = frayUnpause(tau->uF, 0, &tau->fc);
  CHECK_EQ(tau->e, SUCCESS);
  CHECK_EQ(tau->fc.origIdx, tau->fc.newIdx);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 0);
}

TEST_F(Tau, frayUnpause_Inactive) {
  tau->e = frayUnpause(tau->uF, tau->iniPop - 1, &tau->fc);
  CHECK_EQ(tau->e, SUCCESS);
  // Changes are all zeros when nothing happens.
  CHECK_EQ(tau->fc.origIdx, 0);
  CHECK_EQ(tau->fc.intermediateIdx, 0);
  CHECK_EQ(tau->fc.newIdx, 0);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 0);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive);
}

TEST_F(Tau, frayUnpauseAll) {
  // Pause first 3 elements.
  for (U32 i = 0; !tau->e && i < 3; ++i) {
    tau->e = frayPause(tau->uF, i, NULL);
  }
  CHECK_EQ(tau->e, SUCCESS);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 3);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive - 3);
  // Unpause all of them.
  _frayUnpauseAll(tau->uF);
  CHECK_TRUE(!_frayElemIsPaused(tau->uF, 0));
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive);
}

TEST_F(Tau, frayUnpause_Segfault) {
  tau->e = frayUnpause(tau->uF, tau->nElems, NULL);
  CHECK_EQ(tau->e, E_FRAY_SEGFAULT);
}

TEST_F(Tau, frayActivate_StoreChanges) {
  U32 origIdx = tau->iniActive + 2;
  U32 newIdx  = tau->iniActive;
  tau->e = frayActivate(tau->uF, origIdx, &tau->fc);
  CHECK_EQ(tau->e, SUCCESS);
  CHECK_EQ(_frayGetFirstPausedIdx(tau->uF), tau->iniActive + 1);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive + 1);
  CHECK_TRUE(tau->fc.origIdx == origIdx);
  CHECK_TRUE(tau->fc.intermediateIdx == 0);
  CHECK_TRUE(tau->fc.newIdx== newIdx);
}

TEST_F(Tau, frayActivate_DontStoreChanges) {
  tau->e = frayActivate(tau->uF, tau->iniActive + 2, NULL);
  CHECK_EQ(tau->e, SUCCESS);
  CHECK_EQ(_frayGetFirstPausedIdx(tau->uF), tau->iniActive + 1);
  CHECK_TRUE(tau->fc.origIdx == 
             tau->fc.intermediateIdx == 
             tau->fc.newIdx== 0);
}

TEST_F(Tau, frayActivate_Segfault) {
  tau->e = frayActivate(tau->uF, tau->nElems, NULL);
  CHECK_EQ(tau->e, E_FRAY_SEGFAULT);
}

TEST_F(Tau, frayActivate_PausedElement) {
  // First pause an element.
  tau->e = frayPause(tau->uF, tau->iniActive, &tau->fc);
  CHECK_EQ(tau->e, SUCCESS);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 1);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive);
  CHECK_EQ(tau->fc.origIdx, tau->fc.newIdx);
  // Then activate it.
  tau->e = frayActivate(tau->uF, tau->fc.newIdx, &tau->fc);
  CHECK_EQ(tau->e, SUCCESS);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive + 1);
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 0);
  CHECK_EQ(tau->fc.origIdx, tau->fc.newIdx);
}

TEST_F(Tau, frayActivate_AcrossPause) {
  // First pause an element.
  tau->e = frayPause(tau->uF, tau->iniActive, &tau->fc);
  CHECK_EQ(tau->e, SUCCESS);
  CHECK_TRUE(_frayElemIsPaused(tau->uF, tau->fc.newIdx));
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 1);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive);
  CHECK_EQ(tau->fc.origIdx, tau->fc.newIdx);
  // Then activate an inactive one beyond it.
  U32 origVal = tau->uF[tau->iniPop - 1];
  tau->e = frayActivate(tau->uF, tau->iniPop - 1, &tau->fc);
  CHECK_EQ(tau->e, SUCCESS);
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
  tau->e = frayPause(tau->uF, tau->iniPop - 1, &tau->fc);
  CHECK_EQ(tau->e, SUCCESS);
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
  tau->e = frayDeactivate(tau->uF, 0, &tau->fc);
  CHECK_EQ(tau->e, SUCCESS);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive - 1);
  CHECK_EQ(tau->uF[tau->fc.newIdx], origVal);
}

TEST_F(Tau, frayDeactivate_Segfault) {
  tau->e = frayDeactivate(tau->uF, tau->iniPop, &tau->fc);
  CHECK_EQ(tau->e, E_FRAY_SEGFAULT);
}

TEST_F(Tau, frayDeactivate_PausedElement) {
  // Pause last inactive element.
  tau->e = frayPause(tau->uF, tau->iniPop - 1, &tau->fc);
  CHECK_EQ(tau->e, SUCCESS);
  CHECK_TRUE(_frayElemIsPaused(tau->uF, tau->fc.newIdx));
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 1);
  // Deactivate paused element.
  tau->e = frayDeactivate(tau->uF, tau->fc.newIdx, &tau->fc);
  CHECK_EQ(tau->e, SUCCESS);
  CHECK_EQ(_frayGetNActive(tau->uF), tau->iniActive);  // exp 5, actual 4
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 0);
}

TEST_F(Tau, frayDeactivate_AcrossPause) {
  // Pause last element.
  tau->e = frayPause(tau->uF, tau->iniPop - 1, &tau->fc);
  CHECK_EQ(tau->e, SUCCESS);
  CHECK_TRUE(_frayElemIsPaused(tau->uF, tau->fc.newIdx));
  CHECK_EQ(*_frayGetNPausedP(tau->uF), 1);
  // Deactivate first element.
  tau->e = frayDeactivate(tau->uF, 0, &tau->fc);
  CHECK_EQ(tau->e, SUCCESS);
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
  tau->e = frayPause(tau->uF, tau->iniPop - 1, &tau->fc);
  CHECK_EQ(tau->e, SUCCESS);
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

TEST_F(Tau, frayActivateThenDeactivateSameElement) {
  CHECK_FALSE(_frayElemIsActive(tau->uF, 5));
  for (int i = 0; i < 10; ++i) {
    printf("tau->uF[%d] = %d\n", i, tau->uF[i]);
  }
  tau->e = frayActivate(tau->uF, 9, &tau->fc);
  CHECK_EQ(tau->uF[5], 9); // this is failing
  printf("first inactive idx: %d\n", _frayGetFirstInactiveIdx(tau->uF));
  printf("num paused: %d\n", *_frayGetNPausedP(tau->uF));
  printf("num active: %d\n", _frayGetNActive(tau->uF));
  printf("first paused idx: %d\n", _frayGetFirstPausedIdx(tau->uF));
  printf("first empty idx: %d\n", *_frayGetFirstEmptyIdxP(tau->uF));

  // changesP->origIdx = idx;
  // changesP->intermediateIdx = intermediateIdx;
  // changesP->newIdx = newIdx;
  printf("activation: orig = %d, int = %d, new = %d\n", tau->fc.origIdx, tau->fc.intermediateIdx, tau->fc.newIdx);
  REQUIRE_EQ(tau->e, SUCCESS);
  CHECK_TRUE(_frayElemIsActive(tau->uF, 5));
  tau->e = frayDeactivate(tau->uF, 5, &tau->fc);
  printf("first inactive idx: %d\n", _frayGetFirstInactiveIdx(tau->uF));
  printf("num paused: %d\n", *_frayGetNPausedP(tau->uF));
  printf("num active: %d\n", _frayGetNActive(tau->uF));
  printf("first paused idx: %d\n", _frayGetFirstPausedIdx(tau->uF));
  printf("first empty idx: %d\n", *_frayGetFirstEmptyIdxP(tau->uF));
  printf("deactivation: orig = %d, int = %d, new = %d\n", tau->fc.origIdx, tau->fc.intermediateIdx, tau->fc.newIdx);
  REQUIRE_EQ(tau->e, SUCCESS);
  CHECK_FALSE(_frayElemIsActive(tau->uF, 5)); // this is failing
}
