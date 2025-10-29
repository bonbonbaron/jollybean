#ifndef X_GENERIC
#define X_GENERIC
#include "x/x.h"
#include "data/inflatable.h"

#define MEDIA_SYS_ID (2)

typedef struct XMediaComp {
  StripDataS* sdP;  // we're not really doing anything with this, so who cares
} XMediaComp;


typedef struct {
  System system;
  // who cares
} XMedia;

extern System *sAP;
#endif  // X_GENERIC
