#include "tau/tau.h"
#include "map.h"

static Error _popMap(Map *mP, U32 nElems) {
  Error e = SUCCESS;
  // Populate inner map with 1...100
  for (Key j = 2; !e && j <= nElems; ++j) {
    U32 k = (U32) j;
    e = mapSet(mP, j, &k);
  }
  // In order to cover the shifting of map array elements over by one:
  if (!e) {
    U32 k = 1;
    e = mapSet(mP, 1, &k);
  }
  return e;
}

U32 randomValue = 42;
U32 *randValP = &randomValue;

TAU_MAIN()

typedef struct Tau {
  Map *P;
  Map *cpP;
  Map *mapOfNestedMaps;
  Map *mapOfNestedPtrMaps;
  U32 nElems;
} Tau;
TEST_F_SETUP(Tau) {
  tau->nElems = 100;
  tau->P = NULL;
  tau->cpP = NULL;
  tau->mapOfNestedMaps = NULL;
  tau->mapOfNestedPtrMaps = NULL;

  // Allocate simple map.
  Error e = mapNew(&tau->P, RAW_DATA, sizeof(U32), tau->nElems);
  REQUIRE_EQ(e, SUCCESS);
  CHECK_NOT_NULL(tau->P);
  e = _popMap(tau->P, tau->nElems);
  REQUIRE_EQ(e, SUCCESS);

  // Allocate map to copy keys to.
  e = mapNew(&tau->cpP, RAW_DATA, sizeof(U32), tau->nElems);
  REQUIRE_EQ(e, SUCCESS);
  CHECK_NOT_NULL(tau->cpP);
 
  // Allocate map of maps.
  e = mapNew(&tau->mapOfNestedMaps, MAP_POINTER, sizeof(Map*), tau->nElems);
  REQUIRE_EQ(e, SUCCESS);
  CHECK_NOT_NULL(tau->mapOfNestedMaps);

  // Allocate map of maps of pointer types.
  e = mapNew(&tau->mapOfNestedPtrMaps, MAP_POINTER, sizeof(Map*), tau->nElems);
  REQUIRE_EQ(e, SUCCESS);
  CHECK_NOT_NULL(tau->mapOfNestedPtrMaps);

  // Populate map of maps of pointers
  for (Key i = 1; i <= tau->nElems; ++i) {
    Map *newMP = NULL;
    e = mapNew(&newMP, NONMAP_POINTER, sizeof(U32), tau->nElems);
    REQUIRE_EQ(e, SUCCESS);
    CHECK_NOT_NULL(newMP);
    CHECK_NOT_NULL(newMP->mapA);
    // Populate inner map with 1...100
    for (Key j = 1; !e && j <= tau->nElems; ++j) {
      e = mapSet(newMP, j, &randValP);
    }
    REQUIRE_EQ(e, SUCCESS);
    mapSet(tau->mapOfNestedPtrMaps, i, &newMP);
  }

  // Populate outer map with 100 inner maps
  for (Key i = 1; i <= tau->nElems; ++i) {
    Map *newMP = NULL;
    e = mapNew(&newMP, RAW_DATA, sizeof(U32), tau->nElems);
    REQUIRE_EQ(e, SUCCESS);
    CHECK_NOT_NULL(newMP);
    CHECK_NOT_NULL(newMP->mapA);
    // Populate inner map with 1...100
    e = _popMap(newMP, tau->nElems);
    REQUIRE_EQ(e, SUCCESS);
    mapSet(tau->mapOfNestedMaps, i, &newMP);
  }
}

TEST_F_TEARDOWN(Tau) {
  mapDel(&tau->P);
  mapDel(&tau->cpP);
  mapOfNestedMapsDel(&tau->mapOfNestedMaps);
}

TEST_F(Tau, mapSet_BadArg) {
  Error e = mapSet(tau->P, 0, 0);
  CHECK_EQ(e, E_BAD_ARGS);
}

TEST_F(Tau, mapSet_Full) {
  Error e = mapSet(tau->P, tau->nElems + 1, &tau->nElems);
  CHECK_EQ(e, E_MAP_FULL);
}

TEST_F(Tau, mapGet) {
  U32 *valP = (U32*) mapGet(tau->P, 20);
  CHECK_NOT_NULL(valP);
  CHECK_EQ(*valP, 20);
}

TEST_F(Tau, mapGet_Null) {
  U32 *valP = (U32*) mapGet(tau->P, 0);
  CHECK_NULL(valP);
}

TEST_F(Tau, mapGet_Overreach) {
  U32 *valP = (U32*) mapGet(tau->P, tau->nElems + 1);
  CHECK_NULL(valP);
}

TEST_F(Tau, mapNew_BadArgs) {
  Map *whateverMP;
  Error e = mapNew(NULL, RAW_DATA, 1, 1);
  CHECK_EQ(e, E_BAD_ARGS);
  e = mapNew(&whateverMP, RAW_DATA, 0, 1);
  CHECK_EQ(e, E_BAD_ARGS);
  e = mapNew(&whateverMP, RAW_DATA, 1, 0);
  CHECK_EQ(e, E_BAD_ARGS);
}

TEST_F(Tau, mapRem) {
  Error e = mapRem(tau->P, 20);
  CHECK_EQ(e, SUCCESS);
  U32 *valP = mapGet(tau->P, 20);
  CHECK_NULL(valP);
  e = mapRem(tau->P, 0);
  CHECK_EQ(e, E_BAD_ARGS);
}

TEST_F(Tau, mapCopyKeys) {
  Error e = mapCopyKeys(tau->cpP, tau->P);
  CHECK_EQ(e, SUCCESS);
  e = mapCopyKeys(tau->cpP, NULL);
  CHECK_EQ(e, E_BAD_ARGS);
  e = mapCopyKeys(NULL, tau->P);
  CHECK_EQ(e, E_BAD_ARGS);
}

TEST_F(Tau, mapGetIndex) {
  Key idx = 0;
  Error e = mapGetIndex(tau->P, 5, &idx);
  CHECK_EQ(e, SUCCESS);
  CHECK_EQ(idx, 4);
  e = mapGetIndex(tau->P, tau->nElems + 1, &idx);
  CHECK_EQ(e, E_BAD_KEY);
}

TEST_F(Tau, mapGetNestedMapP) {
  Map *mP = NULL;
  Error e = mapGetNestedMapP(tau->mapOfNestedMaps, 5, &mP);
  CHECK_EQ(e, SUCCESS);
  CHECK_NOT_NULL(mP);
  e = mapGetNestedMapP(NULL, 5, &mP);
  CHECK_EQ(e, E_BAD_ARGS);
  e = mapGetNestedMapP(tau->mapOfNestedMaps, 0, &mP);
  CHECK_EQ(e, E_BAD_ARGS);
}

TEST_F(Tau, mapGetNestedMapP_BadKey) {
  Map *mP = NULL;
  Error e = mapGetNestedMapP(tau->mapOfNestedMaps, tau->nElems + 1, &mP);
  CHECK_EQ(e, E_BAD_KEY);
}

TEST_F(Tau, mapGetNestedMapPElem) {
  U32 *valP = NULL;
  Error e = mapGetNestedMapPElem(tau->mapOfNestedMaps, 5, 20, RAW_DATA, (void*) &valP);
  CHECK_EQ(e, SUCCESS);
  CHECK_NOT_NULL(valP);
  CHECK_EQ(*valP, 20);
}

TEST_F(Tau, mapGetNestedMapPElem_WrongType) {
  U32 *valP = NULL;
  Error e = mapGetNestedMapPElem(tau->mapOfNestedMaps, 5, 20, MAP_POINTER, (void*) &valP);
  CHECK_EQ(e, E_MAP_WRONG_ELEM_TYPE);
}

TEST_F(Tau, mapGetNestedMapPElem_BadArgs) {
  U32 *valP = NULL;
  Error e = mapGetNestedMapPElem(tau->mapOfNestedMaps, 0, 20, MAP_POINTER, (void*) &valP);
  CHECK_EQ(e, E_BAD_ARGS);
  e = mapGetNestedMapPElem(tau->mapOfNestedMaps, 5, 0, MAP_POINTER, (void*) &valP);
  CHECK_EQ(e, E_BAD_ARGS);
  e = mapGetNestedMapPElem(tau->mapOfNestedMaps, 5, 20, MAP_POINTER, NULL);
  CHECK_EQ(e, E_BAD_ARGS);
}

TEST_F(Tau, mapGetNestedMapPElem_BadElemType) {
  Map *mP = NULL;
  U32 *valP;
  Error e = mapGetNestedMapP(tau->mapOfNestedMaps, 5, &mP);
  CHECK_EQ(e, SUCCESS);
  CHECK_NOT_NULL(mP);
  mP->elemType = 100000;  // set it to something bogus after the fact
  e = mapGetNestedMapPElem(tau->mapOfNestedMaps, 5, 20, MAP_POINTER, (void**) &valP);
  CHECK_EQ(e, E_MAP_WRONG_ELEM_TYPE);
}

TEST_F(Tau, mapGetNestedMapP_PointerType) {
  U32 *valP;
  Error e = mapGetNestedMapPElem(tau->mapOfNestedPtrMaps, 5, 20, NONMAP_POINTER, (void**) &valP);
  CHECK_EQ(e, SUCCESS);
  CHECK_NOT_NULL(valP);
  CHECK_EQ(*valP, randomValue);
}

TEST_F(Tau, mapGetNestedMapP_PointerType_BadKey) {
  U32 *valP;
  Error e = mapGetNestedMapPElem(tau->mapOfNestedPtrMaps, 5, tau->nElems + 1, NONMAP_POINTER, (void**) &valP);
  CHECK_EQ(e, E_BAD_KEY);
  CHECK_NULL(valP);
}
