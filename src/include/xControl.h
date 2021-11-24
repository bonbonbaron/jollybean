#ifndef SYS_CONTROL
#define SYS_CONTROL
#include "ecs.h"
#include "SDL.h"
typedef enum {
	CTRL_LISTEN = 1
} XControlActivityID;

typedef struct {
	XHeader xHeader;
	U32 buttonsDown;
} XControlC;

extern U32 buttonsPressed;
#define CTRL_LEFT   1 << 0
#define CTRL_RIGHT  1 << 1
#define CTRL_UP     1 << 2
#define CTRL_DOWN   1 << 3
#define CTRL_A      1 << 4
#define CTRL_B      1 << 5
#define CTRL_START  1 << 6
#define CTRL_SELECT 1 << 7

extern System sControl;
Error ctrlListen(Activity *aP);
#endif
