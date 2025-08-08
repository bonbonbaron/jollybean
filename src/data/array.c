#include "data/array.h"

void* arrayNew( U32 elemSz, U32 nElems, const PoolId poolId) {
	assert (elemSz && nElems);
	U32* ptr = (U32*) memAdd( (elemSz * nElems) + (2 * sizeof(U32)), poolId );
  ptr[0] = elemSz;
  ptr[1] = nElems;
	return ptr + 2;
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

/* Also provide an external copy of above function. */
void* arrayGetVoidElemPtr(const void *arryP, S32 idx) {
  assert( arryP );
  assert( idx < arrayGetNElems(arryP) );
  return (void*) ((U8*) arryP + (idx * arrayGetElemSz(arryP)));
}

void arraySetVoidElem(void *arrayP, U32 idx, const void *elemSrcompP) {
	assert (arrayP && idx < arrayGetNElems(arrayP) && elemSrcompP);
	U32 elemSz = arrayGetElemSz((const void*) arrayP);
	memcpy((void*)((U8*) arrayP + (idx * elemSz)), elemSrcompP, elemSz);
}

// If endIdx is illegal, endP's being NULL should be okay since it stops loops.
void arrayIniPtrs(const void *arryP, void **startP, void **endP, S32 endIdx) {
	*startP = (void*) arryP;
	*endP = _arrayGetElemByIdx(arryP, endIdx);
}
