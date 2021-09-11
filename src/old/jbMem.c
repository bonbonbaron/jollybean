inline Error jbAlloc(void **voidPP, elemSz, nElems) {
	if (voidPP == NULL)
		return E_BAD_ARGS;
	*voidPP = malloc(elemSz * nElems);
	if (voidPP == NULL)
		return E_NO_MEMORY;
	return SUCCESS;
}

inline void jbFree(void **voidPP) {
	if (voidPP != NULL) {
		free(*voidPP);
		*voidPP = NULL;
	}
}
