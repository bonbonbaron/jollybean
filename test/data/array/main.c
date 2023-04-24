#include "array.h"
#include <assert.h>

Error test_arrayNew() {
	assert(TRUE);
	return SUCCESS;
}

Error test_arrayDel() {
	assert(TRUE);
	return SUCCESS;
}

Error test_arrayGetNElems() {
	assert(TRUE);
	return SUCCESS;
}

Error test_arrayGetElemSz() {
	assert(TRUE);
	return SUCCESS;
}

Error test_arrayGetVoidElemPtr() {
	assert(TRUE);
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

