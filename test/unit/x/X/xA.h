#ifndef SYS_A
#define SYS_A
#include "x.h"

typedef struct {
  int a, b, c;
} XAComp;

// Images
Error xAIniS(System *sP, void *sParamsP);
Error xAProcessMessage(System *sP, Message *msgP);

typedef struct {
  System system;
} XA;

extern System *sAP;
#endif
