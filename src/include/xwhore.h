#ifndef SYS_whore
#define SYS_whore
#include "botox.h"

typedef struct {
} XwhoreCompSrc; 

typedef struct {
} XwhoreComp;

// Images
Error xwhoreIniS(System *sP, void *sParamsP);
Error xwhoreProcessMessage(System *sP, Message *msgP);

typedef struct {
  System system;
} Xwhore;

extern System *swhoreP;
#endif
