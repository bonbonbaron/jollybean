#ifndef X_MEDIA
#define X_MEDIA
#include "x/x.h"
#include "data/inflatable.h"

#define MEDIA_SYS_ID (2)

typedef struct XMediaComp {
  StripDataS* sdP;  // we're not really doing anything with this, so who cares
} XMediaComp;


typedef struct XMedia {
  System system;
  // who cares
} XMedia;

extern System *sAP;
#endif  // X_MEDIA
