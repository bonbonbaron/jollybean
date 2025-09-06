#ifndef X_GENERIC
#define X_GENERIC
#include "x/x.h"

typedef enum GenericEnum { GENA, GENB, GENC, GEND } GenericEnum;

typedef struct {
  int a;      // immutable gene in a composite
  short b;    // mutable gene in a composite
  char c;     // mutable gene in a composite
  double d;   // implciit gene in a composite
  int *intP;  // variant gene, unique for each entity
} XGenericComp;

typedef struct {
  short bb;
  char cc;
} XGenericMutation;

typedef struct {
  System system;
  U32 someSystemwideU32;
  Map *aMP;
  Map *dMP;
  Map *sharedIntMP;
  Entity *entityF;
} XGeneric;

XConsumeGeneFuncDef_(Generic);
XRunFuncDef_(Generic);

extern System *sAP;
#endif  // X_GENERIC
