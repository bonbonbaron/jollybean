#include "xRender.h"
#include "xControl.h"
#include "SDL.h"
#include "genes.h"
#include "ecs.h"

typedef Error (*Callback)(Entity entity);

typedef struct {
	Callback callback;
	U8 priority;
	Entity entity;
} Behavior;

typedef struct {
	union {
		System s;
		Behavior b;
	} item;
} XJollybeanC;

enum {JB_RUN = 1, JB_BEHAVE};
extern System sJollybean;
Error xJollybeanExe(Activity *aP);
static U32 entityCounter;
Error xJollybeanRun(Activity *aP);
Error xJollybeanIniC(XHeader *xhP);
Error xJollybeanIniS();
Error xJollybeanBehave(Activity *aP);
