#ifndef SYS_CONTROL
#define SYS_CONTROL
#include "botox.h"
#include "jbInterface.h"

typedef enum {
	CTRL_LISTEN = 1
} XControlFocusID;

typedef struct {
	XHeader xHeader;
	U32 buttonsDown;
} XControlComp;

extern U32 buttonsPressed;
#define CTRL_LEFT   1 << 0
#define CTRL_RIGHT  1 << 1
#define CTRL_UP     1 << 2
#define CTRL_DOWN   1 << 3
#define CTRL_A      1 << 4
#define CTRL_B      1 << 5
#define CTRL_START  1 << 6
#define CTRL_SELECT 1 << 7

extern System xControl;
Error ctrlListen(Focus *fP);
#endif
