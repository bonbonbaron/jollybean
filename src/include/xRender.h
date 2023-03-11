#ifndef SYS_RENDER
#define SYS_RENDER
#include "jb.h"

// TODO is both being 1 a bug, or is that the *inner* key value?
#define WINDOW_KEY_ (1)
#define RENDERER_KEY_ (1)

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
// Rightward children are odd, downward even. 
// We leverage this knowledge to navigate backwards without re-entering already-explored nodes.
#define getParentAtlasIdx_(childIdx_) ((childIdx_ - 1 - !(cameFromRight = childIdx_ & 1)) >> 1)
#define getNthRightAtlasDescendant_ getNthLeftDescendant_
#define getNthLowerAtlasDescendant_ getNthRightDescendant_
// 
#define COLORMAP      (0x40)
#define COLOR_PALETTE (0x80)
#define TILEMAP       (0xC0)

#define IS_OFFSET (0x01)
#define FRAME_TIME_UP (N_XMAIL_BUILTIN_CMDS + 1)

#define RENDER_SYS_OWNS_SRC_AND_OFFSET (0x04)

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
  Rect_ *srcRectP;
  Rect_ *dstRectP;
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
  System         system;
  U8             atlasPaletteOffset;
  Colormap     **cmPF;
  ColorPalette **cpPF;
  Entity        *entityF;  // components aren't added till postProcess(), so track entities here
  Window_       *windowP;
  Renderer_     *rendererP;
  Surface_      *atlasSurfaceP;
  Texture_      *atlasTextureP;
  Map           *offsetRectMP;
  Map           *srcRectMP;   // shortcut-pointer to shared array of source rectangles
  Map           *dstRectMP;   // shortcut-pointer to shared array of destination rectangles
  Map           *xRenderCompSourceMP;
} XRender;

extern System *sRenderP;
#endif
