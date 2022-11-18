#ifndef SYS_RENDER
#define SYS_RENDER
#include "botox.h"

// TODO is both being 1 a bug, or is that the *inner* key value?
#define WINDOW_KEY_ (1)
#define RENDERER_KEY_ (1)
typedef enum{WINDOW_GENE_TYPE = 1, RENDERER_GENE_TYPE, N_MASTER_GENES} GeneType;
typedef struct {
  U8 r, g, b, a;
} Color;

typedef struct {
  U8 nColors;
  Color *paletteA;
} ColorPalette;

typedef struct {
  U8 bpp;
  U16 w, h, pitch;  // in pixel units; determine actual step size by pixel format
  StripSetS *stripSetP;
  StripMapS *stripMapP;
  U8 *dataP;    // JB only supports 8-bit colormap. If image requires neither strips nor bit-unpacking, this simply points at the inflatable data.
} ColormapS;     // When the inflatable requires neither unpacking nor strip-mapping, go ahead and memcpy over. I can't think of any cleaner way to do it.

// Backgrounds are made of tiles, although their source images are made of strips.  
// Therefore, the bg's ROM image is the tileset. The tileset gets compressed into
// strips just like all the other (foreground) images.
typedef struct {
  U32 tileDim;    // tiles are assumed to be square, so it's tile "dim" instead of width and height
  U32 nTilesHigh; // number of tiles high
  U32 nTilesWide; // number of tiles wide
  U16 *tileSetA;  // not to be confused with strip set
  U16 *tileMapA;  // not to be confused with strip map
} BgTilemap;

typedef struct {
  U8 textureAtlasRectIdx;
  ColorPalette *colorPalette;
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
