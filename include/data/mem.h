#ifndef MEM_H
#define MEM_H

#include "data/common.h"

// Basic memory utils 
void* jbAlloc( U32 elemSz, U32 nElems);
void jbFree(void **voidPP);

#endif  // #ifndef MEM_H
