#ifndef X_GENERIC
#define X_GENERIC
#include "x/x.h"

#define MEDIA_SYS_ID (2)
typedef enum GenericSysEnum { GENA, GENB, GENC, GEND } GenericSysEnum;

typedef struct {
  int    i;
  Image* imgP;  // we won't render them, just want to prove media genes get into the system
} XGenericComp;

typedef struct {
  System system;
  U32 someSystemwideU32;
  Map *sharedIntMP;
  Entity *entityF;
} XGeneric;

extern System *sAP;
#endif  // X_GENERIC
