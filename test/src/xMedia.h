#ifndef X_GENERIC
#define X_GENERIC
#include "x/x.h"
#include "gene/Image.h"

#define MEDIA_SYS_ID (2)
typedef enum GenericSysEnum { GENA, GENB, GENC, GEND } GenericSysEnum;

typedef Image* XGenericComp;

typedef struct {
  System system;
  U32 someSystemwideU32;
  Map *sharedIntMP;
  Entity *entityF;
} XGeneric;

extern System *sAP;
#endif  // X_GENERIC
