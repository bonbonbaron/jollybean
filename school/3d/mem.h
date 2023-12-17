#ifndef MEM_H
#define MEM_H

#include "common.h"

// Basic memory utils 
Error jbAlloc(void **voidPP, U32 elemSz, U32 nElems);
void  jbFree(void **voidPP);

#endif  // #ifndef MEM_H
