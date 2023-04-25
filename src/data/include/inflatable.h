#ifndef INFLATABLE_H
#define INFLATABLE_H

#include "mem.h"

// Inflatables 
typedef struct Inflatable {
	U32 compressedLen;
	U32 inflatedLen;
	void *inflatedDataP;
	U8  *compressedDataA;
} Inflatable;

Error inflatableIni(Inflatable *inflatableP);
void inflatableClr(Inflatable *inflatableP);

#endif  // #ifndef INFLATABLE_H
