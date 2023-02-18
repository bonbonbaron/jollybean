#ifndef SYS_RENDER
#define SYS_RENDER
#include "jb.h"

// TODO is both being 1 a bug, or is that the *inner* key value?
#define WINDOW_KEY_ (1)
#define RENDERER_KEY_ (1)
typedef enum{WINDOW_GENE_TYPE = 1, RENDERER_GENE_TYPE, N_MASTER_GENES} GeneType;

#define getRightAtlasChildIdx_  getLeftChildIdx_
#define getLowerAtlasChildIdx_  getRightChildIdx_
#define TEXTURE_MAX_DIM_    (4096)
#define CAN_RIGHT     (0x00000001)
#define SHOULD_RIGHT  (0x00000003)  // implies "can"
#define CAN_DOWN      (0x00000004)
#define SHOULD_DOWN   (0x0000000c)  // implies "can"
#define SHOULD_RIGHT_CAN_DOWN (SHOULD_RIGHT | CAN_DOWN)
#define CAN_RIGHT_SHOULD_DOWN (SHOULD_DOWN | CAN_RIGHT)
#define SHOULD_RIGHT_DOWN  (SHOULD_RIGHT | SHOULD_DOWN)
#define getNthRightDescendant_(n_) ((1 << (n_)) - 1)
#define getNthLowerDescendant_(n_) ((2 << (n_)) - 2)
// Rightward children are odd, downward even. 
// We leverage this knowledge to navigate backwards without re-entering already-explored nodes.
#define getParentAtlasIdx_(childIdx_) ((childIdx_ - 1 - !(cameFromRight = childIdx_ & 1)) >> 1)

typedef enum { COLORMAP, COLOR_PALETTE } RenderSubtype;

typedef struct {
  U32 used, x, y, remWidth, remHeight;  // rem = "remaining" (space remaining without this rect)
} AtlasElem;

typedef struct {
  U32 maxDim;
  U32 srcIdx;     // index in source data
  Rect_ rect;  // avoid chasing pointers back to the orignal rectangle
} SortedRect;


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
  Colormap *colorMapP;
} XRenderCompSrc; 

typedef struct {
  Key srcRectIdx;
  Key dstRectIdx;
} XRenderComp;

// Images
Error cmGen(Colormap *imgP);
void  cmClr(Colormap *imgP);
Error xRenderIniS(System *sP, void *sParamsP);
Error xRenderProcessMessage(System *sP, Message *msgP);
typedef void (*XRenderPresentU)(Renderer_ *rendererP);
extern XRenderPresentU present;
extern XPostprocessCompsDef_(Render);

typedef struct {
  System system;
  U8 atlasPaletteOffset;
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
