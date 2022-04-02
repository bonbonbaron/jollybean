#ifndef SYS_REPLACEME
#define SYS_REPLACEME
#include "botox.h"

typedef struct {
} XREPLACEMECompSrc; 

typedef struct {
} XREPLACEMEComp;

// Images
Error xREPLACEMEIniS(System *sP, void *sParamsP);
Error xREPLACEMEProcessMessage(System *sP, Message *msgP);

typedef struct {
  System system;
} XREPLACEME;

extern System *sREPLACEMEP;
#endif
