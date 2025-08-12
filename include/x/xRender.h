#ifndef SYS_RENDER
#define SYS_RENDER
#include "data/list.h"
#include "x/x.h"
#include "interface/interface.h"
#include "data/bt.h"
#include "jb.h"

//typedef enum {} XRenderMsgCmd;

// TODO move enums all to one file okay!? Geez they're everywhere
#define BLOB_KEY_ (2)

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

typedef struct {
  U8 zHeight;
} ZHeight;

typedef struct {
  ListNodeHeader hdr;
  Rect_ *srcRectP;
  Rect_ *dstRectP;
  Key   *zHeightP;  
} XRenderComp;

typedef struct {} XRenderMutation;

// Feel free to increment as needed. There's nothing *holy* about this number.
#define N_LAYERS_SUPPORTED (16)  

typedef struct Collision {
  ListNodeHeader hdr;
  Entity entity;
  Key blobIdIdx;
  S16 bottomYCoord;
} Collision;

typedef struct {
  System         system;
  U8             atlasPaletteOffset;
  Bln            collisionsDetected;
  Image        **imgPF;
  Entity        *entityF;  // components aren't added till postProcess(), so track entities here
  List           layerListA[ N_LAYERS_SUPPORTED ];
  Texture_      *atlasTextureP;
  Map           *offsetRectMP;
  Map           *srcRectMP;    // shortcut-pointer to shared array of source rectangles
  Map           *dstRectMP;    // shortcut-pointer to shared array of destination rectangles
  Map           *zHeightMP;    // shortcut-pointer to shared array of Z-heights
  List          *blobLF;    // list of blobs of overlaps (so we only sort within each blob)
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
