#ifndef INFLATABLE_H
#define INFLATABLE_H

#include "mem.h"

// Inflatables 
typedef struct Inflatable {
	size_t compressedLen;
	size_t inflatedLen;
	void *inflatedDataP;
	U8  *compressedDataA;
} Inflatable;

void inflatableIni(Inflatable *inflatableP);
void inflatableClr(Inflatable *inflatableP);

#endif  // #ifndef INFLATABLE_H
