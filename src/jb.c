#include "jb.h"

static System *sPA[] = {
	&sRender
}; 

NEW_GENOME_(bigger, &biggerXRenderC.xHeader);
GENOME_GROUP_(gg, &bigger, &bigger, &bigger);
U16 nSystems = sizeof(sPA) / sizeof(sPA[0]);

int main() {
	Error e = xIni(sPA, nSystems, &gg);
	while (!e)
		e = xRun();
	return e;
}
