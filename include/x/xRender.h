#ifndef SYS_RENDER
#define SYS_RENDER
#include "x/x.h"
#include "interface/interface.h"
#include "data/bt.h"
#include "jb.h"

typedef enum { MSG_MOVED_Y_UP = N_XMAIL_BUILTIN_CMDS, MSG_MOVED_Y_DOWN, MSG_MOVED_Z_UP, MSG_MOVED_Z_DOWN } XRenderMsgCmd;

// TODO is both being 1 a bug, or is that the *inner* key value?
#define GUI_KEY_ (1)

#define RIGHT_RECT (LEFT_CHILD)
#define LOWER_RECT (RIGHT_CHILD)

#define TEXTURE_MAX_DIM_    (4096)
#define CAN_RIGHT     (0x00000001)
#define SHOULD_RIGHT  (0x00000003)  // implies "can"
#define CAN_DOWN      (0x00000004)
#define SHOULD_DOWN   (0x0000000c)  // implies "can"
#define SHOULD_RIGHT_CAN_DOWN (SHOULD_RIGHT | CAN_DOWN)
#define CAN_RIGHT_SHOULD_DOWN (SHOULD_DOWN | CAN_RIGHT)
#define SHOULD_RIGHT_DOWN  (SHOULD_RIGHT | SHOULD_DOWN)

#define IMAGE      (0x40)
#define TILEMAP    (0x80)

#define FRAME_TIME_UP (N_XMAIL_BUILTIN_CMDS + 1)

#define RENDER_SYS_OWNS_SRC_AND_OFFSET (0x04)

typedef struct {
  BtElHeader header;
  Key srcIdx;  // keeps track of element in an unsorted array to relate them
  Rect_ rect;
  U32 maxDim,
    remW,   // remaining atlas width  as if this rect weren't here
    remH;  // remaining atlas height as if this rect weren't here
} AtlasElem;

typedef struct {
  Child extremityA[2];
  AtlasElem *btP;
} Atlas;

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

/* Think this through:
 *  Yes, you want rendering elements to be in a singly linked list.
 *  But collisions don't need to be processed in the same order.
 *  In fact, I want to mimimize collision processing to moving rectangles only.
 *
 *  You need two pieces of information:
 *    1) The z-height group it's in (NO NEED TO WORRY ABOUT Y-MOTION)
 *    2) Whether it's moving
 *    
 *  I want moving rects in the front of their groups so each successive rect has one less
 *  rect to deal with (having already been tested with their predecessors). 
 *
 *  Shit, that means I have to keep track of the first and last moving rect in each Z-height.
 *  
 
  */
typedef struct {
  U8 zHeight;
} ZHeight;

typedef struct {
  Rect_ *srcRectP;
  Rect_ *dstRectP;
  Key   *zHeightP;
} XRenderComp;

typedef struct {} XRenderMutation;

typedef struct {
  System         system;
  U8             atlasPaletteOffset;
  Image        **imgPF;
  Entity        *entityF;  // components aren't added till postProcess(), so track entities here
  Gui           *guiP;   // derived from shared component 
  Texture_      *atlasTextureP;
  Map           *offsetRectMP;
  Map           *srcRectMP;    // shortcut-pointer to shared array of source rectangles
  Map           *dstRectMP;    // shortcut-pointer to shared array of destination rectangles
  Map           *zHeightMP;    // shortcut-pointer to shared array of Z-heights
  // Map           *xRenderCompSourceMP;  // TODO uhhh what was this?
} XRender;

// Images
Atlas* atlasNew( Image **imgPF);
void atlasDel(Atlas **atlasPP);
void atlasPlanPlacements(Atlas *atlasP);
void xRenderIniS(System *sP, void *sParamsP);
void xRenderProcessMessage(System *sP, Message *msgP);
typedef void (*XRenderPresentU)(Renderer_ *rendererP);  // TODO uh.. what was this for again?
// extern XPostprocessCompsDef_(Render);
void updateCmSrcRectIndices(Image **imgPF, Atlas *atlasP);
Color_* assembleTextureAtlas(Image** imgPF, Atlas *atlasP);

extern System *sRenderP;
#endif
