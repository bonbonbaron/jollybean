#ifndef SYS_Blah
#define SYS_Blah
#include "botox.h"

typedef struct {
} XBlahCompSrc; 

typedef struct {
} XBlahComp;

// Images
Error xBlahIniS(System *sP, void *sParamsP);
Error xBlahProcessMessage(System *sP, Message *msgP);

typedef struct {
  System system;
} XBlah;

extern System *sBlahP;
#endif
