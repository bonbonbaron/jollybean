#ifndef ARRAY_H
#define ARRAY_H

#include "data/mem.h"

void* arrayNew( U32 elemSz, U32 nElems, const PoolId poolId);
void* array2dNew( U32 w, U32 h, U32 elemSz, U32 nElems, const PoolId poolId);
void arrayDel(void **arryPP);
U32 arrayGetNElems(const void *arryP);
U32 arrayGetElemSz(const void *arryP);
void arrayIniPtrs(const void *arryP, void **startP, void **endP, S32 endIdx);
void* arrayGetVoidElemPtr(const void *arryP, S32 idx);
void arraySetVoidElem(void *arrayP, U32 idx, const void *elemSrcP);

inline static void* _arrayGetElemByIdx(const void *arryP, S32 idx) {
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

inline U32 _fast_arrayGetElemSz(const void *arryP) {
	return *(((U32*)arryP) - 2);
}

inline void* _fast_arrayGetElemByIdx(const void *arryP, U32 idx) {
	return (void*) ((U8*) arryP + (idx * _fast_arrayGetElemSz(arryP)));
}


#endif  // #ifndef ARRAY_H
