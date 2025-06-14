#ifndef INFLATABLE_UTIL_H
#define INFLATABLE_UTIL_H
#include "data/inflatable.h"

Inflatable* inflatableNew( void* voidA, const MemoryType memType);
void inflatableDel(Inflatable **inflatablePP);
void inflatableWrite(Inflatable *inflatableP, char *filepathA, char *inflatableNameA);
void inflatableAppend(Inflatable *inflatableP, FILE *fP, char *inflatableNameA);
#endif
