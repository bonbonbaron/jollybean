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
	U32 *ptr;

	if (elemSz <= 0 || nElems < 0 || arryPP == NULL) {
		return E_BAD_ARGS;  /* TODO: replace with reasonable error type */
	}
	if (nElems == 0) {
		*arryPP = NULL;
	}
	else {
		ptr = (U32*) malloc((elemSz * nElems) + (2 * sizeof(U32)));
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
	if (arryPP != NULL) {
		if (*arryPP != NULL) {
			U32 *ptr = *arryPP;
			free((ptr) - 2);
			*arryPP = NULL;
		}
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

/* Rule here is "while (ptr < endPtr)". */
void* arrayGetEndPtr(const void *arryP, S32 endIdx) {
  if (endIdx < 0)
	  return ((U8*) arryP) + (arrayGetNElems(arryP) * arrayGetElemSz(arryP));
  else 
    return ((U8*) arryP) + (endIdx * arrayGetElemSz(arryP));
}

void arrayIniPtrs(const void *arryP, void **startP, void **endP, S32 endIdx) {
  *startP = (void*) arryP;
  *endP = arrayGetEndPtr(arryP, endIdx);
}

/***********************/
/********* MAPS ********/
/***********************/
Error mapNew(Map **mapPP, const U8 elemSz, const U16 nElems) {
	Error e = jbAlloc((void**) mapPP, sizeof(Map), 1);
	if (!e)
		e = arrayNew(&(*mapPP)->mapA, elemSz, nElems);
	return e;
}

void mapDel(Map **mapPP) {
	arrayDel(&(*mapPP)->mapA);
	jbFree((void**) mapPP);
}

static __inline U8 _mapIsValid(const Map *mapP) {
	return (mapP != NULL && mapP->mapA != NULL); 
}	

/* Map GETTING functions */
static __inline U8 _isFlagSet(U8 flags, const U8 key) {
	return bitFlagLUT[key] & flags;
}

static __inline U8 _mapGetElemIdx(const U8 flags, const U8 prevBitCount, const U8 key) {
	return (bitCountLUT[flags & bitCountMaskLUT[key]]) + prevBitCount - 1;
}

static __inline void* _mapGetElemP(const Map *mapP, const U8 key) {
	const FlagInfo f = mapP->flagA[byteIdxLUT[key]];
	const U8 flags = f.flags;
  if (!_isFlagSet(flags, key)) {
		return NULL;
  }
	const U8 elemIdx = _mapGetElemIdx(flags, f.prevBitCount, key);
	return (void*) ((U32) mapP->mapA + ((U32) elemIdx * (U32) arrayGetElemSz(mapP->mapA)));
}	

void* mapGet(const Map *mapP, const U8 key) {
	if (_mapIsValid(mapP)) 
		return _mapGetElemP(mapP, key);
	return NULL;
}
/* Map SETTING functinos */
static __inline U8 _mapGetNextEmptyElemIdx(const Map *mapP, const U8 key) {
	const FlagInfo f = mapP->flagA[byteIdxLUT[key]];
	const U8 flags = f.flags;
	return (bitCountLUT[flags & bitCountMaskLUT[key]]) + f.prevBitCount;
}

static __inline void* _mapGetNextEmptyElemP(const Map *mapP, const U8 key) {
	const U8 elemIdx = _mapGetNextEmptyElemIdx(mapP, key);
	void *ptr = (void*) ((U32) mapP->mapA + ((U32) elemIdx * (U32) arrayGetElemSz(mapP->mapA)));
	return (void*) ((U32) mapP->mapA + ((U32) elemIdx * (U32) arrayGetElemSz(mapP->mapA)));
}

Error mapSet(Map *mapP, const U8 key, const void *valP) {
	if (_mapIsValid(mapP)) {
		void *elemP = _mapGetNextEmptyElemP(mapP, key);

		if (elemP != NULL) {
			/* Copy value into map element. */
			memcpy(elemP, valP, arrayGetElemSz(mapP->mapA));
			/* Set flag. */
			U8 byteIdx = byteIdxLUT[key];
			mapP->flagA[byteIdx].flags |= 1 << (key & 0x07);  /* flagNum & 0x07 gives you # of bits in the Nth byte */
			/* Increment all prevBitCounts in bytes above affected one. */
			while (++byteIdx < N_FLAG_BYTES) 
				mapP->flagA[byteIdx].prevBitCount++;
		}
		else
			return E_BAD_INDEX;
	}
	return SUCCESS;
}

/************************************/
/************ HISTOGRAMS ************/
/************************************/

Error histoU8New(U32 **histoPP, const U8 *srcA, const U32 maxVal) {
	if (histoPP == NULL || srcA == NULL) {
		return E_BAD_ARGS;
  }
	Error e = arrayNew((void**) histoPP, sizeof(U32), maxVal);
	if (!e) {
		U8 *u8P = (U8*) srcA;
		const U8 *u8EndP = u8P + maxVal;
		U32 *histoP = *histoPP;
		while (u8P < u8EndP) 
			histoP[*u8P++]++;
	}
	return e;
}	

void histoU8Del(U32 **histoPP) {
	arrayDel((void**) histoPP);
}

/* testing testing 1 2 3 */
