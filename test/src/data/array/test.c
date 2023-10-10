#include "tau/tau.h"
#include "array.h"


/*
functions to test:
==================

  \arrayNew
  \arrayDel
  arrayGetNElems
  arrayGetElemSz
  arrayGetVoidElemPtr
  arraySetVoidElem
  arrayIniPtrs

==================
*/

struct Array { 
  U32 *P;
  U32 dummyNumber;
};

static const int N_ELEMS = 100;


TAU_MAIN()

TEST_F_SETUP(Array) {
  Error e = arrayNew((void**) &tau->P, sizeof(U32), N_ELEMS);
  REQUIRE_EQ(e, SUCCESS);
  CHECK_NOT_NULL(tau->P);
  REQUIRE_EQ(arrayGetNElems(tau->P), N_ELEMS);
  if (!e) {
    for (U32 i = 0; i < N_ELEMS; ++i) {
      ((U32*) tau->P)[i] = i;
    }
  }
  tau->dummyNumber = 10000;
}

TEST_F_TEARDOWN(Array) {
  arrayDel((void**) &tau->P);
  CHECK_NULL(tau->P);
}

TEST_F(Array, RightNum) {
  CHECK_EQ(arrayGetNElems(tau->P), N_ELEMS);
}

TEST_F(Array, RightElemSize) {
  CHECK_EQ(arrayGetElemSz(tau->P), sizeof(U32));
}

TEST_F(Array, GetVoidElPtrWorks) {
  void *voidP = arrayGetVoidElemPtr((void*) tau->P, 50);
  CHECK_NOT_NULL(voidP);
  CHECK_EQ(*((U32*) voidP), 50);
}

TEST_F(Array, GetLastVoidElPtrWorks) {
  void *voidP = arrayGetVoidElemPtr((void*) tau->P, -1);
  CHECK_EQ ((U32*) voidP - tau->P, arrayGetNElems(tau->P));
}

TEST_F(Array, SetVoidElPtrWorks) {
  Error e = arraySetVoidElem((void*) tau->P, 50, (void*) &tau->dummyNumber);
  REQUIRE_EQ(e, SUCCESS);
  CHECK_EQ(tau->P[50], tau->dummyNumber);
}

TEST_F(Array, IniPtrsWorksToEnd) {
  U32 *iP, *iEndP;
  arrayIniPtrs((void*) tau->P, (void**) &iP, (void**) &iEndP, -1);
  CHECK_EQ(iEndP - iP, arrayGetNElems(tau->P));
}

TEST_F(Array, IniPtrsWorksPartway) {
  U32 *iP, *iEndP;
  arrayIniPtrs((void*) tau->P, (void**) &iP, (void**) &iEndP, 20);
  CHECK_EQ(iEndP - iP, 20);
}

TEST_F(Array, IllegalEndPtr) {
  U32 *iP, *iEndP;
  arrayIniPtrs((void*) tau->P, (void**) &iP, (void**) &iEndP, N_ELEMS + 1);
  CHECK_NULL(iEndP);
}

TEST_F(Array, BadArgsArrayNew) {
  Error e = SUCCESS;
  e = arrayNew(NULL, 1, 1);
  CHECK_NE(e, SUCCESS);
  e = arrayNew((void**) &tau->P, 0, 1);
  CHECK_NE(e, SUCCESS);
  e = arrayNew((void**) &tau->P, 1, 0);
  CHECK_NE(e, SUCCESS);
}

TEST_F(Array, BadArgsGetNElems) {
  CHECK_EQ(arrayGetNElems(NULL), 0);
}

TEST_F(Array, BadArgsGetElemSz) {
  CHECK_EQ(arrayGetElemSz(NULL), 0);
}

TEST_F(Array, BadArgsGetVoidElemPtr) {
  void *voidP = arrayGetVoidElemPtr((void*) tau->P, N_ELEMS + 1);
  CHECK_NULL(voidP);
}

TEST_F(Array, BadArgsSetVoidElem) {
  tau->dummyNumber = N_ELEMS + 1;
  Error e = SUCCESS;
  e = arraySetVoidElem((void*) tau->P, 1, NULL);
  CHECK_NE(e, SUCCESS);
  e = arraySetVoidElem((void*) tau->P, tau->dummyNumber, &tau->dummyNumber);
  CHECK_NE(e, SUCCESS);
  e = arraySetVoidElem((void*) NULL, 1, &tau->dummyNumber);
}

TEST_F(Array, FastElemSzWorks) {
  CHECK_EQ(_fast_arrayGetElemSz((void*) tau->P), sizeof(U32));
}

TEST_F(Array, FastGetVoidElemWorks) {
  void* voidP = _fast_arrayGetElemByIdx((void*) tau->P, 50);
  REQUIRE_NE(voidP, NULL);
  CHECK_EQ(*((U32*) voidP), 50);
}
