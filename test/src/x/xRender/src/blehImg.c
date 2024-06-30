#include "xRender.h"
#include "blehImg.h"
#include "blehColormap.h"
#include "blehColorPalette.h"

Image blehImg = {
	.state = 0,
	.sortedRectIdx = 0,
	.cmP = &blehColormap,
	.cpP = &blehColorPalette,
};

