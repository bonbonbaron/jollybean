#include "nectar.h"

ColormapS biggerCM = {
	4,    // bpp
	64,    // w
	256,    // h
	32,    // pitch
	&biggerStripSetS,  // pointer to strip set
	&biggerStripMapS,  // pointer to strip map
	NULL  // place were pixel data will go
};
