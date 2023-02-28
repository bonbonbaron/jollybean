#ifndef SYS_Timer
#define SYS_Timer
#include "x.h"

typedef enum {START_TIMER, PAUSE_TIMER, STOP_TIMER, TIME_UP} TimerKeychain;

typedef struct {
} XTimerCompSrc; 

typedef struct {
  U8 timeLeft;
  U8 timeFull;
  U8 repeat;
} XTimerComp;

// Images
Error xTimerIniS(System *sP, void *sParamsP);
Error xTimerProcessMessage(System *sP, Message *msgP);

typedef struct {
  System system;
} XTimer;

extern System *sTimerP;
#endif
