#include "xRender.h"
#include "redColormap.h"
#include "redColorPalette.h"

Image redImg = {
	.state = 0,
	.sortedRectIdx = 0,
	.cmP = &redColormap,
	.cpP = &redColorPalette,
};

