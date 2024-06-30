#include "xRender.h"
#include "heckColormap.h"
#include "heckColorPalette.h"

Image heckImg = {
	.state = 0,
	.sortedRectIdx = 0,
	.cmP = &heckColormap,
	.cpP = &heckColorPalette,
};

