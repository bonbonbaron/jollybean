#ifndef SYS_RENDER
#define SYS_RENDER
#include "botox.h"

#define WINDOW_KEY_ (1)
#define RENDERER_KEY_ (1)

typedef enum{WINDOW_GENE_TYPE = 1, RENDERER_GENE_TYPE, N_MASTER_GENES} GeneType;

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
  U8 textureAtlasRectIdx;
  Color_ *colorA;
  ColormapS *colorMapP;
} XRenderCompSrc; 

typedef struct {
  Key srcRectIdx;
  Key dstRectIdx;
} XRenderComp;

// Images
Error cmGen(ColormapS *imgP);
void  cmClr(ColormapS *imgP);
Error xRenderIniS(System *sP, void *sParamsP);
Error xRenderProcessMessage(System *sP, Message *msgP);
typedef void (*XRenderPresentU)(Renderer_ *rendererP);
extern XRenderPresentU present;

typedef struct {
  System system;
  Window_ *windowP;
  Renderer_ *rendererP;
  Texture_ *textureP;
  Map *srcRectMAMP;  // Lets rendering system update the source rectangles' positions according to their placement in the texture atlas.
  Map *srcRectMP;   // Keeps rendering system posted on the animation frame of each animated entity.
  Map *dstRectMP;   // Keeps rendering system posted on the position and scale of each mobile entity.
  Map *xRenderCompSourceMP;
} XRender;

extern System *sRenderP;
#endif
