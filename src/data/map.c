#include "data/map.h"

#if __WORDSIZE == 32
const static UWord BITCOUNT_MASK[] = {
  0x00000000, 0x00000001, 0x00000003, 0x00000007, 
  0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f, 
  0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff, 
  0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff, 
  0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff, 
  0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff, 
  0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff, 
  0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff, 
};
#elif __WORDSIZE == 64
// AIN'T IT BEAUTIFUL!!!
const static UWord BITCOUNT_MASK[] = {
  // First 32 bits
  0x0000000000000000, 0x0000000000000001, 0x0000000000000003, 0x0000000000000007, 
  0x000000000000000f, 0x000000000000001f, 0x000000000000003f, 0x000000000000007f, 
  0x00000000000000ff, 0x00000000000001ff, 0x00000000000003ff, 0x00000000000007ff, 
  0x0000000000000fff, 0x0000000000001fff, 0x0000000000003fff, 0x0000000000007fff, 
  0x000000000000ffff, 0x000000000001ffff, 0x000000000003ffff, 0x000000000007ffff, 
  0x00000000000fffff, 0x00000000001fffff, 0x00000000003fffff, 0x00000000007fffff, 
  0x0000000000ffffff, 0x0000000001ffffff, 0x0000000003ffffff, 0x0000000007ffffff, 
  0x000000000fffffff, 0x000000001fffffff, 0x000000003fffffff, 0x000000007fffffff, 
  // Last 32 bits
  0x00000000ffffffff, 0x00000001ffffffff, 0x00000003ffffffff, 0x00000007ffffffff, 
  0x0000000fffffffff, 0x0000001fffffffff, 0x0000003fffffffff, 0x0000007fffffffff, 
  0x000000ffffffffff, 0x000001ffffffffff, 0x000003ffffffffff, 0x000007ffffffffff, 
  0x00000fffffffffff, 0x00001fffffffffff, 0x00003fffffffffff, 0x00007fffffffffff, 
  0x0000ffffffffffff, 0x0001ffffffffffff, 0x0003ffffffffffff, 0x0007ffffffffffff, 
  0x000fffffffffffff, 0x001fffffffffffff, 0x003fffffffffffff, 0x007fffffffffffff, 
  0x00ffffffffffffff, 0x01ffffffffffffff, 0x03ffffffffffffff, 0x07ffffffffffffff, 
  0x0fffffffffffffff, 0x1fffffffffffffff, 0x3fffffffffffffff, 0x7fffffffffffffff, 
};
#else
static_assert( 0, "Jollybean only supports 32- and 64-bit architectures.");
#endif

// Done for *local* popcounting (base added after).
Map* mapNew( MapElemType elemType, const U8 elemSz, const Key nElems, const PoolId poolId) {
	assert (elemSz && nElems);
  Map* mapP = memAdd(sizeof(Map), poolId );
  mapP->sbmP = sbmNew( nElems /* assume max bit == nElems for now */, poolId );
	mapP->mapA = arrayNew(elemSz, nElems, poolId );
  mapP->elemType = elemType;
  return mapP;
}

Key mapGetIndex(const Map *mapP, const Key key) {
	const BasedWord* basedWordP = sbmGetBasedWord( mapP->sbmP, key );
  return rawBitCount( basedWordP->bits & BITCOUNT_MASK[ key & LOCAL_BIT_MASK ] ) + basedWordP->base;
}

inline static void* _getElemP(const Map *mapP, const Key key) {
  assert( mapP );
	return _fast_arrayGetElemByIdx(mapP->mapA, mapGetIndex(mapP, key));
}	

inline static U32 _getMapElemSz(const Map *mapP) {
  return arrayGetElemSz(mapP->mapA);
}

U32 mapHasKey(const Map* mP, const Key key ) {
  assert (mP);
  return sbmIsBitSet( mP->sbmP, key );
}

void* mapGet(const Map *mapP, const Key key) {
  assert (mapP );
  BasedWord* basedWordP;
  // This "Ex" function lets us see if a bit is set without having to reload its bitfield afterward.
	if ( sbmIsBitSetEx( mapP->sbmP, key, &basedWordP ) ) {
     UWord popcount = bmSum( basedWordP->bits & BITCOUNT_MASK[ key & LOCAL_BIT_MASK ], basedWordP->base);
		return _fast_arrayGetElemByIdx(mapP->mapA, popcount);
	}
	return NULL;
}

/* Map SETTING functions */
/* If any bits exist to the left of the key's bit, array elements exist in target spot. */
inline static U8 _idxIsPopulated(const U32 nBitsSet, U32 idx) {
  return (idx < nBitsSet);
}

// If something's already in the target index, move everything over one. 
static U32 countBytesToShiftOver(const Map *mapP, const Key key, void **elemPP, void **nextElemPP) {
  *elemPP = _getElemP(mapP, key);
  if (*elemPP) {
    U32 keyElemIdx = mapGetIndex( mapP, key );
    if (_idxIsPopulated(mapP->sbmP->population, keyElemIdx)) {
      U32 mapElemSz = _getMapElemSz(mapP);
      *nextElemPP = (U8*) *elemPP + mapElemSz;
      return (mapP->sbmP->population - keyElemIdx) * mapElemSz;
    }
  }
  return 0;
}

void mapSet(Map *mapP, const Key key, const void *valP) {
  assert (mapP && valP);
  assert (mapP->sbmP->population < arrayGetNElems(mapP->mapA));
	void *elemP, *nextElemP;
  U32 nBytesToMove = countBytesToShiftOver(mapP, key, &elemP, &nextElemP);
  if (nBytesToMove) {
    memmove(nextElemP, (const void*) elemP, nBytesToMove);
  }
  /* Write value in array. */
  memcpy(elemP, valP, _getMapElemSz(mapP));
  sbmSetBit( mapP->sbmP, key );
}

void mapRem(Map *mapP, const Key key) {
  assert (mapP);
	void *elemP, *nextElemP;
  U32 nBytesToMove = countBytesToShiftOver( mapP, key, &elemP, &nextElemP );
  if (nBytesToMove) {
    nBytesToMove -= _getMapElemSz(mapP);
    memmove(elemP, (const void*) nextElemP, nBytesToMove);
  }
  sbmUnsetBit( mapP->sbmP, key );
}

void mapCopyKeys(Map *dstMP, Map *srcMP) {
  assert (srcMP);
  assert (dstMP);
  assert( !dstMP->sbmP );  // Don't want to leak memory.
  dstMP->sbmP = sbmClone(srcMP->sbmP, GENERAL);
}

Map* mapGetNestedMapP(Map *outerMP, Key mapKey) {
  assert (outerMP && outerMP->elemType == MAP_POINTER);
  Map **innerMapPP = (Map**) mapGet(outerMP, mapKey);
  assert( innerMapPP );
  assert( *innerMapPP );
  return *innerMapPP;
}

void* mapGetNestedMapPElem(Map *mapP, Key mapKey, Key elemKey, MapElemType expectedElemType) {
  assert (mapP);

  Map *nestedMP = mapGetNestedMapP(mapP, mapKey);
  assert (nestedMP->elemType == expectedElemType);

  void **valPP;
  // No case-switch necessary since there are only two possibilities: raw and some kind of pointer.
  // Non-matching type is impossible due to check above.
  if (nestedMP->elemType == RAW_DATA) {
    // This safely returns a single-pointer.
    return mapGet(nestedMP, elemKey);
  }
  else {
    valPP = mapGet(nestedMP, elemKey);
    if (valPP) {
      assert( *valPP);
      return *valPP;
    }
  }
  return NULL;
}
