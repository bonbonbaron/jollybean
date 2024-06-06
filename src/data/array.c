#include "array.h"

void* arrayNew( U32 elemSz, U32 nElems) {
	assert (elemSz && nElems);
	U32* ptr = (U32*) jbAlloc( (elemSz * nElems) + (2 * sizeof(U32)), 1);
  ptr[0] = elemSz;
  ptr[1] = nElems;
  // void *arryP = ptr + 2;
  // memset(arryP, 0, elemSz * nElems);  // is there a consequence to leaving this out? 
	return ptr + 2;
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

/* Also provide an external copy of above function. */
void* arrayGetVoidElemPtr(const void *arryP, S32 idx) {
  const U32 nElems = arrayGetNElems(arryP);
  /* If idx < 0, return void pointer past end of array. */
  if ((U32) idx < nElems) {
  /* If idx is valid, return void pointer to indexed element. */
    return (void*) ((U8*) arryP + (idx * arrayGetElemSz(arryP)));
  }
  else if (idx < 0) {
    return (void*) (((U8*) arryP) + (nElems * arrayGetElemSz(arryP)));
  }
  /* Index is invalid. */
  return NULL;  
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
