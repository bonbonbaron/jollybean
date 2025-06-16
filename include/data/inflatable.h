#ifndef INFLATABLE_H
#define INFLATABLE_H

#include "data/mem.h"

// Inflatables 
typedef struct Inflatable {
	size_t compressedLen;
	size_t inflatedLen;
	void *inflatedDataP;
	U8  *compressedDataA;
} Inflatable;

void inflatableIni(Inflatable *inflatableP, const PoolId poolId);

#endif  // #ifndef INFLATABLE_H
