#include "array.h"
#include <assert.h>

Error test_arrayNew() {
  U32 *aP = NULL;
  Error e = arrayNew((void**) &aP, sizeof(U32), 5);
  assert(!e);
  arrayDel((void**) &aP);
	return SUCCESS;
}

Error test_arrayDel() {
  U32 *aP = NULL;
  Error e = arrayNew((void**) &aP, sizeof(U32), 5);
  assert(!e && aP);
  arrayDel((void**) &aP);
  assert(aP == NULL);
	return SUCCESS;
}

Error test_arrayGetNElems() {
  U32 *aP = NULL;
  Error e = arrayNew((void**) &aP, sizeof(U32), 5);
  assert(!e && aP);
  assert(arrayGetNElems(aP) == 5);
  arrayDel((void**) &aP);
  assert(aP == NULL);
	return SUCCESS;
}

Error test_arrayGetElemSz() {
  U32 *aP = NULL;
  Error e = arrayNew((void**) &aP, sizeof(U32), 5);
  assert(!e && aP);
  assert(arrayGetElemSz(aP) == sizeof(U32));
  arrayDel((void**) &aP);
  assert(aP == NULL);
	return SUCCESS;
}

Error test_arrayGetVoidElemPtr() {
	assert(TRUE);
  void** aP;
  Error e = arrayNew((void**) &aP, sizeof(void*), 5);
  assert(!e && aP);
  void **elemP = arrayGetVoidElemPtr(aP, 3);
  assert(elemP && ((elemP - aP) == 3));
  arrayDel((void**) &aP);
	return SUCCESS;
}

Error test_arraySetVoidElem() {
	assert(TRUE);
	return SUCCESS;
}

Error test_arrayIniPtrs() {
	assert(TRUE);
	return SUCCESS;
}

int main(int argc, char **argv) {
	assert(test_arrayNew() == SUCCESS);
	assert(test_arrayDel() == SUCCESS);
	assert(test_arrayGetNElems() == SUCCESS);
	assert(test_arrayGetElemSz() == SUCCESS);
	assert(test_arrayGetVoidElemPtr() == SUCCESS);
	assert(test_arraySetVoidElem() == SUCCESS);
	assert(test_arrayIniPtrs() == SUCCESS);

	return 0;
}

