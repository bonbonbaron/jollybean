#include "data.h"
#include "bitCountLUT.h"
#include "byteIdxLUT.h"
#include "bitCountMaskLUT.h"
#include "bitFlagLUT.h"

__inline Error jbAlloc(void **voidPP, U8 elemSz, U8 nElems) {
	if (voidPP == NULL)
		return E_BAD_ARGS;
	*voidPP = malloc(nElems * elemSz);
	if (*voidPP == NULL)
		return E_NO_MEMORY;
	return SUCCESS;
}

__inline void jbFree(void **voidPP) {
	if (voidPP != NULL) {
		free(*voidPP);
		*voidPP = NULL;
	}
}

/************************************/
/********** ARRAYS (1D & 2D) ********/
/************************************/
Error arrayNew(void **arryPP, U32 elemSz, U32 nElems) {
	if (elemSz <= 0 || nElems < 0 || arryPP == NULL) 
		return E_BAD_ARGS;  /* TODO: replace with reasonable error type */
	if (nElems == 0) 
		*arryPP = NULL;
	else {
		U32 *ptr = (U32*) malloc((elemSz * nElems) + (2 * sizeof(U32)));
		if (ptr == NULL) 
			return E_NO_MEMORY;
		ptr[0] = elemSz;
		ptr[1] = nElems;
		*arryPP = (ptr + 2);
		memset(*arryPP, 0, elemSz * nElems);
	}
	return SUCCESS;
}
	
void arrayDel(void **arryPP) {
	if (arryPP != NULL && *arryPP != NULL) {
		U32 *ptr = *arryPP;
		free((ptr) - 2);
		*arryPP = NULL;
	}
}

U32 arrayGetNElems(const void *arryP) {
	U32 *ptr;
	if (arryP == NULL) 
		return 0;
	else {
		ptr = (U32*) arryP;
		return *(ptr - 1);
	}
}

U32 arrayGetElemSz(const void *arryP) {
	U32 *ptr;
	if (arryP == NULL)
		return 0;
	else {
		ptr = (U32*) arryP;
		return *(ptr - 2);
	}
}

__inline static void* _fast_arrayGetVoidElemPtr(const void *arryP, S32 idx) {
	return (void*) ((U8*) arryP + (idx * arrayGetElemSz(arryP)));
}

__inline static void* _arrayGetVoidElemPtr(const void *arryP, S32 idx) {
  const U32 nElems = arrayGetNElems(arryP);
  /* If idx < 0, return void pointer past end of array. */
  if (idx < 0) 
    return (void*) (((U8*) arryP) + (nElems * arrayGetElemSz(arryP)));
  /* If idx is valid, return void pointer to indexed element. */
  else if ((U32) idx < nElems)
    return (void*) ((U8*) arryP + (idx * arrayGetElemSz(arryP)));
  /* Index is invalid. */
  else
    return NULL;  
}

/* Also provide an external copy of above function. */
void* arrayGetVoidElemPtr(const void *arryP, S32 idx) {
  const U32 nElems = arrayGetNElems(arryP);
  /* If idx < 0, return void pointer past end of array. */
  if (idx < 0) 
    return (void*) (((U8*) arryP) + (nElems * arrayGetElemSz(arryP)));
  /* If idx is valid, return void pointer to indexed element. */
  else if ((U32) idx < nElems)
    return (void*) ((U8*) arryP + (idx * arrayGetElemSz(arryP)));
  /* Index is invalid. */
  else
    return NULL;  
}

void arrayIniPtrs(const void *arryP, void **startP, void **endP, S32 endIdx) {
	*startP = (void*) arryP;
	*endP = _arrayGetVoidElemPtr(arryP, endIdx);
}

/* arrayIni(), unlike mapIni(), does not need to allocate any memory. 
 * Instead, it reorders its array elements using the enums. 
 * That's why key and value are separated: so the array is already good! */
Error arrayIni(void **arryPP, HardCodedArray *hcaP) {
	if (arryPP == NULL || hcaP == NULL) 
		return E_BAD_ARGS;
	Error e = arrayNew(arryPP, hcaP->_elemSz, hcaP->_nEnumValPairs);
	if (!e) {
		EnumValPair *evP    = &hcaP->enumValA[0];
		EnumValPair *evEndP = &hcaP->enumValA[hcaP->_nEnumValPairs];
		U8 *dstP;
		for (; evP < evEndP; evP++) {
			dstP = _fast_arrayGetVoidElemPtr(*arryPP, evP->_enum);
			if (dstP == NULL) {
				e = E_BAD_INDEX;
				break;
			}
			memcpy(dstP, evP->valP, hcaP->_elemSz);
		}
	}
	if (e)
		arrayDel(arryPP);
	return e;
}

/***********************/
/********* MAPS ********/
/***********************/
Error mapNew(Map **mapPP, const U8 elemSz, const U16 nElems) {
	if (elemSz == 0 || nElems == 0) {
    return E_BAD_ARGS;
  }
  Error e = jbAlloc((void**) mapPP, sizeof(Map), 1);
	if (!e)
		e = arrayNew(&(*mapPP)->mapA, elemSz, nElems);
  if (!e) 
    memset((*mapPP)->flagA, 0, sizeof(FlagInfo) * N_FLAG_BYTES);
  else {
    arrayDel((*mapPP)->mapA);
    jbFree((void**)mapPP);
  }
	return e;
}

Error mapIni(Map **mapPP, HardCodedMap *hcMapP) {
	Error e = SUCCESS;

	//U32 arrayLen = getNullTermALen((void*) keyValA, sizeof(KeyValPair));
	if (hcMapP == NULL) 
		return E_BAD_ARGS;
	if (hcMapP->mapP == NULL) {
		e = mapNew(&hcMapP->mapP, hcMapP->_elemSz, hcMapP->_nKeyValPairs);
		if (!e) 
			for (U32 i = 0; !e && i < hcMapP->_nKeyValPairs; i++) 
				e = mapSet(*mapPP, hcMapP->keyValA[i].key, hcMapP->keyValA[i].valueP);
	}
	
	/* Dev may or may not want to store the mapP somewhere else for easy access. */
	if (!e && mapPP != NULL)
		*mapPP = hcMapP->mapP;

	return e;
}

void mapDel(Map **mapPP) {
	if (mapPP != NULL && *mapPP != NULL) {
		arrayDel(&(*mapPP)->mapA);
		jbFree((void**) mapPP);
	}
}

__inline static U8 _isMapValid(const Map *mapP) {
	return (mapP != NULL && mapP->mapA != NULL); 
}	

__inline static U8 _isKeyValid(const U8 key) {
  return (key > 0);
}

/* Map GETTING functions */
__inline static FlagInfo _getFlagInfo(const Map *mapP, const U8 key) {
  return mapP->flagA[byteIdxLUT[key]];
}
__inline static U8 _isFlagSet(const U8 flags, const U8 key) {
	return bitFlagLUT[key] & flags;
}

__inline static U32 _getElemIdx(const FlagInfo f, const U8 key) {
	return (bitCountLUT[f.flags & bitCountMaskLUT[key]]) + f.prevBitCount;
}

__inline static void* _getElemP(const Map *mapP, const FlagInfo f, const U8 key) {
	return _fast_arrayGetVoidElemPtr(mapP->mapA, _getElemIdx(f, key));
}	

__inline static U32 _getMapElemSz(const Map *mapP) {
  return arrayGetElemSz(mapP->mapA);
}
#define TRY
void* mapGet(const Map *mapP, const U8 key) {
	if (_isMapValid(mapP)) {
	  const FlagInfo f  = _getFlagInfo(mapP, key);
	  if (!_isFlagSet(f.flags, key)) {
		  return NULL;
    }
		return _getElemP(mapP, f, key);  /* f is 2 bytes, so don't pass its pointer. */
  }
	return NULL;
}

__inline static U32 _getNBitsSet(const Map *mapP) {
  return mapP->flagA[LAST_FLAG_BYTE_IDX].prevBitCount + bitCountLUT[mapP->flagA[LAST_FLAG_BYTE_IDX].flags];
}
/* Map SETTING functinos */
/* If any bits exist to the left of the key's bit, array elements exist in target spot. */
__inline static U8 _idxIsPopulated(const U8 nBitsSet, U32 idx) {
  return (idx < nBitsSet);
}

static Error preMapSet(const Map *mapP, const U8 key, void **elemPP, void **nextElemPP, U32 *nBytesTMoveP) {
  *nBytesTMoveP = 0;
  if (_isMapValid(mapP) && _isKeyValid(key)) {
    FlagInfo f;
    f = _getFlagInfo(mapP, key);
    *elemPP = _getElemP(mapP, f, key);
	  if (*elemPP) {  /* Side-stepping mapGet() to avoid NULL pointers and double-calling _isMapValid() */
      U32 nBitsSet = _getNBitsSet(mapP);
			U32 keyElemIdx = _getElemIdx(f, key);
      /* If something's already in the target index, move everything over one. */
      if (_idxIsPopulated(nBitsSet, keyElemIdx)) {
        U32 mapElemSz = _getMapElemSz(mapP);
        *nBytesTMoveP = (nBitsSet - keyElemIdx) * mapElemSz;
        *nextElemPP = (U8*) *elemPP + mapElemSz;
        printf("moving %d bytes %d indices forward\n", *nBytesTMoveP, (((U32) *nextElemPP - (U32) *elemPP) / _getMapElemSz(mapP)));
      }
      return SUCCESS;
    } else {
      return E_BAD_KEY;
    }
  } else 
    return E_BAD_ARGS;
}

Error mapSet(Map *mapP, const U8 key, const void *valP) {
	void *elemP, *nextElemP;
  U32 nBytesToMove;
  Error e = preMapSet(mapP, key, &elemP, &nextElemP, &nBytesToMove);
  if (!e) {
    if (nBytesToMove) 
      memcpy(nextElemP, (const void*) elemP, nBytesToMove);
		/* Write value to map element. */
		memcpy(elemP, valP, _getMapElemSz(mapP));
		/* Set flag. */
		U8 byteIdx = byteIdxLUT[key];
		mapP->flagA[byteIdx].flags |= bitFlagLUT[key];  /* flagNum & 0x07 gives you # of bits in the Nth byte */
		/* Increment all prevBitCounts in bytes above affected one. */
		while (++byteIdx < N_FLAG_BYTES) 
		  ++mapP->flagA[byteIdx].prevBitCount;
	}
	return e;
}

Error mapRem(Map *mapP, const U8 key) {
	void *elemP, *nextElemP;
  U32 nBytesToMove;
  Error e = preMapSet(mapP, key, &elemP, &nextElemP, &nBytesToMove);
  if (!e) {
    if (nBytesToMove) 
      memcpy(elemP, (const void*) nextElemP, nBytesToMove);
		/* Unset flag. */
		U8 byteIdx = byteIdxLUT[key];
		mapP->flagA[byteIdx].flags &= ~bitFlagLUT[key];  /* key's bit position byteIdx'th byte */
		/* Increment all prevBitCounts in bytes above affected one. */
		while (++byteIdx < N_FLAG_BYTES) 
			--mapP->flagA[byteIdx].prevBitCount;
	}
	return e;
}

/************************************/
/************ HISTOGRAMS ************/
/************************************/

Error histoNew(U32 **histoPP, const U32 maxVal) {
	if (histoPP == NULL)
		return E_BAD_ARGS;
	return arrayNew((void**) histoPP, sizeof(U32), maxVal);
}	

void histoU8Del(U32 **histoPP) {
	arrayDel((void**) histoPP);
}
