#include "nectar.h"

Colormap biggerCM = {
	4,    // bpp
	64,    // w
	256,    // h
	32,    // pitch
	&biggerStripSet,  // pointer to strip set
	&biggerStripMap,  // pointer to strip map
	NULL  // place were pixel data will go
};
