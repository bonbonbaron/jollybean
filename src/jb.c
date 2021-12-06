#include "jb.h"

static System *sPA[] = {
	&sRender
}; 

int main() {
	Error e = xIni(sPA, sizeof(sPA) / sizeof(sPA[0]), &grp1);
	while (!e)
		sRun(&sParent);
	return e;
}
