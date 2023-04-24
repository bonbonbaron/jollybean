#include "array.h"
#include <assert.h>

Error test_arrayNew() {
	assert(FALSE);
	return SUCCESS;
}

Error test_arrayDel() {
	assert(FALSE);
	return SUCCESS;
}

Error test_arrayGetNElems() {
	assert(FALSE);
	return SUCCESS;
}

Error test_arrayGetElemSz() {
	assert(FALSE);
	return SUCCESS;
}

Error test_arrayGetVoidElemPtr() {
	assert(FALSE);
	return SUCCESS;
}

Error test_arraySetVoidElem() {
	assert(FALSE);
	return SUCCESS;
}

Error test_arrayIniPtrs() {
	assert(FALSE);
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

