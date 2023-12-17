#include "array.h"

Error arrayNew(void **arryPP, U32 elemSz, U32 nElems) {
  Error e = SUCCESS;
  U32 *ptr;
	if (elemSz <= 0 || nElems <= 0 || arryPP == NULL) {
		return E_BAD_ARGS;  
  }
	else {
		e = jbAlloc((void**) &ptr, (elemSz * nElems) + (2 * sizeof(U32)), 1);
    if (!e) {
      ptr[0] = elemSz;
      ptr[1] = nElems;
      *arryPP = (ptr + 2);
      memset(*arryPP, 0, elemSz * nElems);
    }
	}
	return e;
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

Error arraySetVoidElem(void *arrayP, U32 idx, const void *elemSrcompP) {
	if (!arrayP || idx >= arrayGetNElems(arrayP) || !elemSrcompP) {
		return E_BAD_ARGS;
  }
	U32 elemSz = arrayGetElemSz((const void*) arrayP);
	void *dstP = (U8*) arrayP + (idx * elemSz);
	memcpy(dstP, elemSrcompP, elemSz);
	return SUCCESS;
}

// If endIdx is illegal, endP's being NULL should be okay since it stops loops.
void arrayIniPtrs(const void *arryP, void **startP, void **endP, S32 endIdx) {
	*startP = (void*) arryP;
	*endP = _arrayGetElemByIdx(arryP, endIdx);
}
