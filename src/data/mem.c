#include "data/mem.h"

void* jbAlloc( U32 elemSz, U32 nElems) {
	assert (elemSz && nElems);
	void* voidP = malloc(nElems * elemSz);
	assert( voidP != NULL);
  return voidP;
}

void jbFree(void **voidPP) {
	if (voidPP != NULL) {
		free(*voidPP);
		*voidPP = NULL;
	}
}
