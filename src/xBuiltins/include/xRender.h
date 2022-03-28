#ifndef SYS_RENDER
#define SYS_RENDER
#include "botox.h"
#include "jbInterface.h"

typedef enum {
	XRENDER = 1
} XRenderFocusID;

typedef struct {
	U16 nFlips;
	U16 flipIdxA[];
} FlipSetS;

// Strip set's inflated data is in U32 format.
typedef struct {
	FlipSetS *flipSetP;
	U16 nStrips;    // number of 64-pixel strips in strip set
	Inflatable *stripSetInfP;  // strip set's compressed source data
} StripSetS;

// StripMapS's inflated data is in U16 format.
typedef struct {
	U16 nIndices;
	Inflatable *stripMapInfP;
} StripMapS;

typedef struct {
	U8 bpp;
	U16 w, h, pitch;  // in pixel units; determine actual step size by pixel format
	StripSetS *stripSetP;
	StripMapS *stripMapP;
	U8 *dataP;    // JB only supports 8-bit colormap. If image requires neither strips nor bit-unpacking, this simply points at the inflatable data.
} ColormapS;     // When the inflatable requires neither unpacking nor strip-mapping, go ahead and memcpy over. I can't think of any cleaner way to do it.

typedef struct {
	U8 nColors;
	Color_ *colorA;
	ColormapS *colorMapP;
	TextureS_ *textureP;
} ImageS; 

typedef struct {
	ImageS *imgP;
	Rect_ *srcRectP;
	Rect_ *dstRectP;
} XRenderCompData;
  
// Seems important that every component has its singleton source to enable easy cleanup.
typedef struct {
	XHeader xHeader;
  XRenderCompData data;
} XRenderComp;

// Images
Error cmGen(ColormapS *imgP);
void  cmClr(ColormapS *imgP);
Error xRenderRender(Focus *fP);
Error xRenderIniS(System *sP, void *sParamsP);
Error xRenderProcessMessage(System *sP, Message *msgP);
typedef void (*XRenderPresentFP)(Renderer_ *rendererP);
extern XRenderPresentFP present;
// Implemented by environment of choice (see #defines for each category above)
Error guiNew(Window_ **windowPP, Renderer_ **rendererPP);
Error surfaceNew(Surface_ **surfacePP, XRenderCompData *cP);
Error surfaceIni(Surface_ *surfaceP, XRenderCompData *cP);
Error textureNew(TextureS_ **texturePP, Renderer_ *rendererP, Surface_ *surfaceP);
void  textureDel(TextureS_ **texturePP);
Error textureSetAlpha(TextureS_ *textureP);
void  clearScreen(Renderer_ *rendererP);

typedef struct {
	System system;
	Window_ *windowP;
	Renderer_ *rendererP;
} XRender;

extern System *sRenderP;
#endif
