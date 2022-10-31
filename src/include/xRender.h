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
  S16 textureAtlasIdx;  // Keeps track of distinct image indices in tree. -1 if not yet inserted
  Color_ *colorA;
  ColormapS *colorMapP;
  Texture_ *textureP;
} XRenderCompSrc; 

typedef struct {
  Texture_ *textureP;
  Rect_ *srcRectP;
  Rect_ *dstRectP;
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
  Map *  // Here's the problem: The src rect share array only stores the current value, not the entire range of possibilities.
} XRender;

extern System *sRenderP;
#endif
