#include "mem.h"

Error jbAlloc(void **voidPP, U32 elemSz, U32 nElems) {
	if (voidPP == NULL) {
		return E_BAD_ARGS;
  }
	*voidPP = malloc(nElems * elemSz);
	if (*voidPP == NULL) {
		return E_NO_MEMORY;
  }
	return SUCCESS;
}

void jbFree(void **voidPP) {
	if (voidPP != NULL) {
		free(*voidPP);
		*voidPP = NULL;
	}
}
