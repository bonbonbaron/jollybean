#include "tau.h"
#include "data/array.h"


struct Array { 
  U32 *P;
  U32 dummyNumber;
};

static const int N_ELEMS = 100;


TAU_MAIN()

TEST_F_SETUP(Array) {
  tau->P = arrayNew( sizeof(U32), N_ELEMS, GENERAL );
  CHECK_NOT_NULL(tau->P);
  REQUIRE_EQ(arrayGetNElems(tau->P), N_ELEMS);
  for (U32 i = 0; i < N_ELEMS; ++i) {
    ((U32*) tau->P)[i] = i;
  }
  tau->dummyNumber = 10000;
}

TEST_F_TEARDOWN(Array) {
  memRst( GENERAL );
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

TEST_F(Array, SetVoidElPtrWorks) {
  arraySetVoidElem(tau->P,  50, (void*) &tau->dummyNumber);
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

TEST_F(Array, FastElemSzWorks) {
  CHECK_EQ(_fast_arrayGetElemSz((void*) tau->P), sizeof(U32));
}

TEST_F(Array, FastGetVoidElemWorks) {
  void* voidP = _fast_arrayGetElemByIdx((void*) tau->P, 50);
  REQUIRE_NE(voidP, NULL);
  CHECK_EQ(*((U32*) voidP), 50);
}
