#include "map.h"

Map* mapNew( MapElemType elemType, const U8 elemSz, const Key nElems) {
	assert (elemSz && nElems);
  Map* mapP = jbAlloc(sizeof(Map), 1);
	mapP->mapA = arrayNew(elemSz, nElems);
  mapP->population = 0;
  mapP->elemType = elemType;
  return mapP;
}

void mapDel(Map **mapPP) {
	if (mapPP != NULL && *mapPP != NULL) {
    if ((*mapPP)->mapA != NULL) {
      arrayDel(&(*mapPP)->mapA);
    }
		jbFree((void**) mapPP);
	}
}

inline static U8 _isMapValid(const Map *mapP) {
	return (mapP != NULL && mapP->mapA != NULL); 
}	

/* Map GETTING functions */
inline static U8 _countBits(Key bitfield) {
#ifdef __ARM_NEON__
  asm("vmov.8 d21[0], %0\n\t"
      "vcnt.i8 d20, d21\n\t"
      "vmov.u8 %0, d20[0]"
      : "+r" (bitfield));
	return bitfield;
#else
	register Key count = bitfield - ((bitfield >> 1) & 0x55555555);
	count = (count & 0x33333333) + ((count >> 2) & 0x33333333);
	count = (count + (count >> 4)) & 0x0f0f0f0f;
	return (count * 0x01010101) >> 24;
#endif
}
inline static FlagInfo _getFlagInfo(const Map *mapP, const Key key) {
  return mapP->flagA[byteIdx_(key)];
}

#if 0
inline static U8 _isFlagSet(const U8 flags, const Key key) {
	return flags & (1 << ((key - 1) & 0x07));
}
#endif

inline static U32 _getElemIdx(const FlagInfo f, const Key key) {
	return f.prevBitCount + _countBits(f.flags & (bitFlag_(key) - 1));
}

Key mapGetIndex(const Map *mapP, const Key key) {
	const register Key keyMinus1 = key - 1;
	const register FlagInfo f = mapP->flagA[keyMinus1 >> 3];  // Divide N by 8 for byte with Nth bit.
	const register Key bitFlag = 1 << (keyMinus1 & 0x07);     // 0x07 keeps bit inside 8-bit bounds.
	assert (f.flags & bitFlag);
  return _getElemIdx(f, key);
}

inline static void* _getElemP(const Map *mapP, const FlagInfo f, const Key key) {
	return _fast_arrayGetElemByIdx(mapP->mapA, _getElemIdx(f, key));
}	

inline static U32 _getMapElemSz(const Map *mapP) {
  return arrayGetElemSz(mapP->mapA);
}

void* mapGet(const Map *mapP, const Key key) {
  assert (mapP && key);  // key has to be 1 or greater.
	const register U32 keyMinus1 = key - 1;
	const register FlagInfo f = mapP->flagA[keyMinus1 >> 3];
	const register U32 bitFlag = 1 << (keyMinus1 & 0x07);
	// If the bit flag in question is set, that means a value exists for the input key.
	if (f.flags & bitFlag) {
    register U32 count = f.flags & (bitFlag - 1);  // initialize count with the bits we're counting
    // The desired array index is the number of bits set before the key'th bit.
#ifdef __ARM_NEON__
    // This counts the number of high bits in a word in just a few single CPU cycles.
    asm("vmov.8 d21[0], %0\n\t"
        "vcnt.i8 d20, d21\n\t"
        "vmov.u8 %0, d20[0]"
        : "+r" (count));
#else
		count = count - ((count >> 1) & 0x55555555);
		count = (count & 0x33333333) + ((count >> 2) & 0x33333333);
		count = (count + (count >> 4)) & 0x0f0f0f0f;
		count = (count * 0x01010101) >> 24;
#endif
		return _fast_arrayGetElemByIdx(mapP->mapA, count + f.prevBitCount);
	}
	return NULL;
}

/* Map SETTING functions */
/* If any bits exist to the left of the key's bit, array elements exist in target spot. */
inline static U8 _idxIsPopulated(const U32 nBitsSet, U32 idx) {
  return (idx < nBitsSet);
}

static void _preMapSet(const Map *mapP, const Key key, void **elemPP, void **nextElemPP, U32 *nBytesTMoveP) {
  *nBytesTMoveP = 0;
  FlagInfo f;
  f = _getFlagInfo(mapP, key);
  *elemPP = _getElemP(mapP, f, key);
  if (*elemPP) {  /* Side-stepping mapGet() to avoid NULL pointers and double-calling _isMapValid() */
    U32 keyElemIdx = _getElemIdx(f, key);
    /* If something's already in the target index, move everything over one. */
    if (_idxIsPopulated(mapP->population, keyElemIdx)) {
      U32 mapElemSz = _getMapElemSz(mapP);
      *nBytesTMoveP = (mapP->population - keyElemIdx) * mapElemSz;
      *nextElemPP = (U8*) *elemPP + mapElemSz;
    }
  }
}

void mapSetFlag(Map *mapP, const Key key) {
  Key byteIdx = byteIdx_(key);
  mapP->flagA[byteIdx].flags |= bitFlag_(key);  /* flagNum & 0x07 gives you # of bits in the Nth byte */
  /* Increment all prevBitCounts in bytes above affected one. */
  // TODO vectorize the below if it's available
  while (++byteIdx < N_FLAG_BYTES) {
    ++mapP->flagA[byteIdx].prevBitCount;
  }
}

void mapSet(Map *mapP, const Key key, const void *valP) {
  assert (mapP && key && valP);
  assert (mapP->population < arrayGetNElems(mapP->mapA));
	void *elemP, *nextElemP;
  U32 nBytesToMove;
  _preMapSet(mapP, key, &elemP, &nextElemP, &nBytesToMove);
  //printf("nb2m: %d\n", nBytesToMove);
  if (nBytesToMove) {
    memmove(nextElemP, (const void*) elemP, nBytesToMove);
  }
  /* Write value to map element. */
  memcpy(elemP, valP, _getMapElemSz(mapP));
  /* Set flag. */
  mapSetFlag(mapP, key);
  /* Increment map's population. */
  ++mapP->population;
}

void mapRem(Map *mapP, const Key key) {
  assert (mapP && key);
	void *elemP, *nextElemP;
  U32 nBytesToMove;
  _preMapSet(mapP, key, &elemP, &nextElemP, &nBytesToMove);
  if (nBytesToMove) {
    memmove(elemP, (const void*) nextElemP, nBytesToMove);
  }
  /* Unset flag. */
  U8 byteIdx = byteIdx_(key);
  mapP->flagA[byteIdx].flags &= ~bitFlag_(key);  /* key's bit position byteIdx'th byte */
  /* Increment all prevBitCounts in bytes above affected one. */
  while (++byteIdx < N_FLAG_BYTES) {
    --mapP->flagA[byteIdx].prevBitCount;
  }
  --mapP->population;
}

void mapCopyKeys(Map *dstMP, Map *srcMP) {
  assert (dstMP &&  srcMP);
  memcpy(dstMP->flagA, srcMP->flagA, N_FLAG_BYTES * sizeof(FlagInfo));
}

Map* mapGetNestedMapP(Map *outerMP, Key mapKey) {
  assert (outerMP && mapKey && outerMP->elemType == MAP_POINTER);
  Map **_innerMapPP = (Map**) mapGet(outerMP, mapKey);
  assert (_innerMapPP && *_innerMapPP);
  return *_innerMapPP;
}

void* mapGetNestedMapPElem(Map *mapP, Key mapKey, Key elemKey, MapElemType expectedElemType) {
  assert (mapP && mapKey && elemKey);

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

void mapOfNestedMapsDel(Map **outerMapPP) {
  if (outerMapPP && *outerMapPP) {
    Map **mapPP = (Map**) (*outerMapPP)->mapA;
    Map **mapEndPP = mapPP + (*outerMapPP)->population;
    for (; mapPP < mapEndPP; ++mapPP) {
      if (*mapPP) {
        mapDel(mapPP);
      }
    }
    mapDel(outerMapPP);
  }
}
