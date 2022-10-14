#ifndef INFLATABLE_H
#define INFLATABLE_H
#include "botox/data.h"

Error inflatableNew(void *voidA, Inflatable **inflatablePP);
Error inflatableWrite(Inflatable *inflatableP, char *filepathA, char *inflatableNameA);
#endif
