#include "tau/tau.h"
#include "map.h"

static void _popMap(Map *mP, U32 nElems) {
  // Populate inner map with 1...100
  for (Key j = 2; j <= nElems; ++j) {
    U32 k = (U32) j;
    mapSet(mP, j, &k);
  }
  // In order to cover the shifting of map array elements over by one:
  U32 k = 1;
  mapSet(mP, 1, &k);
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
  tau->P = mapNew( RAW_DATA, sizeof(U32), tau->nElems);
  CHECK_NOT_NULL(tau->P);
  _popMap(tau->P, tau->nElems);

  // Allocate map to copy keys to.
  tau->cpP = mapNew( RAW_DATA, sizeof(U32), tau->nElems);
  CHECK_NOT_NULL(tau->cpP);
 
  // Allocate map of maps.
  tau->mapOfNestedMaps = mapNew( MAP_POINTER, sizeof(Map*), tau->nElems);
  CHECK_NOT_NULL(tau->mapOfNestedMaps);

  // Allocate map of maps of pointer types.
  tau->mapOfNestedPtrMaps = mapNew( MAP_POINTER, sizeof(Map*), tau->nElems);
  CHECK_NOT_NULL(tau->mapOfNestedPtrMaps);

  // Populate map of maps of pointers
  for (Key i = 1; i <= tau->nElems; ++i) {
    Map *newMP = NULL;
    newMP = mapNew( NONMAP_POINTER, sizeof(void*), tau->nElems);
    CHECK_NOT_NULL(newMP);
    CHECK_NOT_NULL(newMP->mapA);
    // Populate inner map with 1...100
    for (Key j = 1; j <= tau->nElems; ++j) {
      mapSet(newMP, j, &randValP);
    }
    mapSet(tau->mapOfNestedPtrMaps, i, &newMP);
  }

  // Populate outer map with 100 inner maps
  for (Key i = 1; i <= tau->nElems; ++i) {
    Map *newMP = NULL;
    newMP = mapNew( RAW_DATA, sizeof(U32), tau->nElems);
    CHECK_NOT_NULL(newMP);
    CHECK_NOT_NULL(newMP->mapA);
    // Populate inner map with 1...100
    _popMap(newMP, tau->nElems);
    mapSet(tau->mapOfNestedMaps, i, &newMP);
  }
}

TEST_F_TEARDOWN(Tau) {
  mapDel(&tau->P);
  mapDel(&tau->cpP);
  mapOfNestedMapsDel(&tau->mapOfNestedMaps);
  mapOfNestedMapsDel(&tau->mapOfNestedPtrMaps);
}

TEST_F(Tau, mapGet) {
  for (Key key = 1; key <= tau->nElems; ++key) {
    U32 *valP = (U32*) mapGet(tau->P, key);
    CHECK_NOT_NULL(valP);
    CHECK_EQ(*valP, key);
  }
}

TEST_F(Tau, mapGetIndex) {
  Key idx = mapGetIndex(tau->P, 5);
  CHECK_EQ(idx, 4);
}
#if 0

TEST_F(Tau, mapGetNestedMapP) {
  Map *mP = NULL;
  mP = mapGetNestedMapP(tau->mapOfNestedMaps, 5);
  CHECK_NOT_NULL(mP);
}

TEST_F(Tau, mapGetNestedMapPElem) {
  U32 *valP = mapGetNestedMapPElem(tau->mapOfNestedMaps, 5, 20, RAW_DATA);
  CHECK_NOT_NULL(valP);
  CHECK_EQ(*valP, 20);
}

TEST_F(Tau, mapGetNestedMapP_PointerType) {
  U32 *valP = mapGetNestedMapPElem(tau->mapOfNestedPtrMaps, 5, 20, NONMAP_POINTER);
  CHECK_NOT_NULL(valP);
  CHECK_EQ(*valP, randomValue);
}

TEST_F(Tau, mapGet_Null) {
  U32 *valP = (U32*) mapGet(tau->P, 1);
  CHECK_NULL(valP);
}


TEST_F(Tau, mapGet_Overreach) {
  U32 *valP = (U32*) mapGet(tau->P, tau->nElems + 1);
  CHECK_NULL(valP);
}

TEST_F(Tau, mapRem) {
  mapRem(tau->P, 20);
  U32 *valP = mapGet(tau->P, 20);
  CHECK_NULL(valP);
}

TEST_F(Tau, mapCopyKeys) {
  mapCopyKeys(tau->cpP, tau->P);
}
#endif
