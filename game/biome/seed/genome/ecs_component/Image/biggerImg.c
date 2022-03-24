#include "nectar.h"

// The time has come to enforce a singleton standard. 
// 
Image biggerImg = {
	.colorMapP = &biggerCM,
	.nColors   = 16,
	.colorA    = &biggerColorSet[0],
	.textureP  = NULL
};

XRenderC biggerXRenderC = {
	.xHeader = {.owner = 0, .type = RENDER},
	.imgP = &biggerImg,
	.srcRectPP = NULL,
	.dstRectPP = NULL
};
