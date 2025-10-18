#ifndef X_GENERIC
#define X_GENERIC
#include "x/x.h"

#define GENERIC (1)
typedef enum GenericSysEnum { GENA, GENB, GENC, GEND } GenericSysEnum;

typedef struct {
  int immutable;      // immutable gene in a composite
  short mutableCompositePc1;    // mutable gene in a composite
  char mutableCompositePc2;     // mutable gene in a composite
  double implicitCompositePc;   // implciit gene in a composite
  int *variantP;  // variant gene, unique for each entity
} XGenericComp;

typedef struct {
  short mutationPc1;
  char mutationPc2;
} XGenericMutation;

typedef struct {
  System system;
  U32 someSystemwideU32;
  Map *mutMP;  // mutation map for A component
  Map *sharedIntMP;
  Entity *entityF;
} XGeneric;

XConsumeGeneFuncDef_(Generic);
XRunFuncDef_(Generic);

extern System *sAP;
#endif  // X_GENERIC
