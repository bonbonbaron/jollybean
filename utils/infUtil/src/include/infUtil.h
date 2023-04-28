#ifndef INFLATABLE_H
#define INFLATABLE_H
#include "common.h"

Error inflatableNew(void *voidA, Inflatable **inflatablePP);
void inflatableDel(Inflatable **inflatablePP);
Error inflatableWrite(Inflatable *inflatableP, char *filepathA, char *inflatableNameA);
Error inflatableAppend(Inflatable *inflatableP, FILE *fP, char *inflatableNameA);
#endif
