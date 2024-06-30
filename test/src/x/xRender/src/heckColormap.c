#include "xRender.h"

U8 heckStripsetInfInfCompressedDataA[] = {
	0x90, 0x0b, 0xfb, 0xa9, 0x7f, 0x3f, 0x9f, 0x1f, 0x80, 0x80, 0x90, 0xc0, 0x1f, 0x0f, 0x0f, 0x0f, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

Inflatable heckStripsetInf = {
	.compressedLen  = 24,
	.inflatedDataP  = NULL,
	.inflatedLen    = 16,
	.compressedDataA = heckStripsetInfInfCompressedDataA
};



StripDataS heckStripData = {
	.flags = 20,
	.sm = {
		.nIndices = 5,
		.infP = NULL
	},
	.ss = {
		.nUnitsPerStrip = 29,
		.nUnits  = 116,
		.bpu  = 1,
		.infP = &heckStripsetInf,
	},
	.assembledDataA = NULL
};

Colormap heckColormap = {
	.bpp = 1,
	.w = 29,
	.h = 5,
	.pitch = 116,
	.sdP = &heckStripData,
};

