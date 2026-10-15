#ifndef SYS_Timer
#define SYS_Timer
#include "x/x.h"
#include "jb.h"

/* Timers count down in frames. Pausing and stopping use the built-in PAUSE/DEACTIVATE
 * commands, so only starting needs a system-specific command. When a timer runs out,
 * TIMER_TIME_UP goes to the Action system's inbox (attn = entity) so behaviors can react. */
typedef enum {
  TIMER_START = MSG_MOTION_STOPPPED + 32,  // offset clear of collision's keychain (both talk to Action)
  TIMER_SET_DURATION,                     // arg = new full duration (frames); doesn't start the timer
  TIMER_TIME_UP
} TimerKeychain;

typedef struct {
  U8 timeLeft;
  U8 timeFull;
  U8 repeat;
} XTimerComp;

// Timers have no mutations; an immutable gene supplies the whole XTimerComp.
typedef struct {} XTimerMutation;

typedef struct {
  System system;
} XTimer;

extern System *sTimerP;
#endif
