#include "xRender.h"
#include "SDL.h"
#include <unistd.h>
#include "genes.h"
#include "ecs.h"

extern System sJollybean;
typedef System XJollybeanC;
Error xJollybeanExe(Activity *aP);
static U32 entityCounter;
Error xJollybeanRun(Activity *aP);
Error xJollybeanIniC(XHeader *xhP);
Error xJollybeanIniS();
