#ifndef SYS_RENDER
#define SYS_RENDER
#include "x.h"
#include "SDL.h"

typedef enum {
	XRENDER = 1
} XRenderFocusID;

typedef struct {
	U16 nFlips;
	U16 flipIdxA[];
} FlipSet;

// Strip set's inflated data is in U32 format.
typedef struct {
	FlipSet *flipSetP;
	U16 nStrips;    // number of 64-pixel strips in strip set
	Inflatable *stripSetInfP;  // strip set's compressed source data
} StripSet;

// StripMap's inflated data is in U16 format.
typedef struct {
	U16 nIndices;
	Inflatable *stripMapInfP;
} StripMap;

typedef struct {
	U8 bpp;
	U16 w, h, pitch;  // in pixel units; determine actual step size by pixel format
	StripSet *stripSetP;
	StripMap *stripMapP;
	U8 *dataP;    // JB only supports 8-bit colormap. If image requires neither strips nor bit-unpacking, this simply points at the inflatable data.
} Colormap;     // When the inflatable requires neither unpacking nor strip-mapping, go ahead and memcpy over. I can't think of any cleaner way to do it.

typedef struct {
	U8 nColors;
	SDL_Color *colorA;
	Colormap *colorMapP;
	SDL_Texture *textureP;
} Image; 

typedef struct {
	XHeader xHeader;
	Image *imgP;
	SDL_Rect **srcRectPP;
	SDL_Rect **dstRectPP;
} XRenderComp;

// Images
Error cmGen(Colormap *imgP);
void cmClr(Colormap *imgP);
Error xRender(Focus *fP);
Error xRenderIniS();
extern System sRender;
#endif
