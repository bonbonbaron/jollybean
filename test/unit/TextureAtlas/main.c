#include "interface.h"
#include "data.h"
#include "blehColormap.h"
#include "blehColorPalette.h"
#include "heckColormap.h"
#include "heckColorPalette.h"
#include "redColormap.h"
#include "redColorPalette.h"
#include "previewImg.h"

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

typedef struct {
  BtElHeader header;
  Rect_ rect;
  U32 maxDim,
    remW,   // remaining atlas width  as if this rect weren't here
    remH;  // remaining atlas height as if this rect weren't here
  U32 srcIdx;   // index in source data  // can you eliminate ths with BT Elem?
} _AtlasElem;

typedef struct {
  Child extremityA[2];
  _AtlasElem *btP;
} Atlas;

static inline Error __atlasLinkNodes(
    Atlas *atlasP,
    Key parentIdx, 
    Key childIdx, 
    Child child,   // this is child *type*, not actual child data
    U32 x, 
    U32 y, 
    U32 remW, 
    U32 remH) {
  if (childIdx >= arrayGetNElems(atlasP->btP)) {
    return E_SEGFAULT;
  }
  printf("parent %d adopts child %d out of %d\n", parentIdx, childIdx, arrayGetNElems(atlasP->btP));
  atlasP->btP[childIdx].rect.x = x,
  atlasP->btP[childIdx].rect.y = y,
  atlasP->btP[childIdx].remW = remW;
  atlasP->btP[childIdx].remH = remH;
  btLinkNodes_(atlasP->btP, &atlasP->btP[parentIdx], &atlasP->btP[childIdx], child);
  printf("setting node %d's xy = {%d, %d}\n", childIdx, atlasP->btP[childIdx].rect.x, atlasP->btP[childIdx].rect.y);
  return SUCCESS;
}

static Error _atlasLinkNodes(Atlas *atlasP, const Key parentIdx, const Key childIdx, Child child) {
  // rectHeight splits node to a shelf rightward and full width downward
  if (child == RIGHT_RECT) {
    return __atlasLinkNodes(
      atlasP,
      parentIdx,
      childIdx,
      RIGHT_RECT, 
      atlasP->btP[parentIdx].rect.x + atlasP->btP[parentIdx].rect.w,
      atlasP->btP[parentIdx].rect.y,
      atlasP->btP[parentIdx].remW - atlasP->btP[parentIdx].rect.w - atlasP->btP[childIdx].rect.w,
      atlasP->btP[parentIdx].rect.h - atlasP->btP[childIdx].rect.h);  
  }
  else {  /* implicit "if child == LOWER_RECT */
    return __atlasLinkNodes(
      atlasP,
      parentIdx,
      childIdx,
      LOWER_RECT, 
      atlasP->btP[parentIdx].rect.x,
      atlasP->btP[parentIdx].rect.y + atlasP->btP[parentIdx].rect.h,
      atlasP->btP[parentIdx].remW - atlasP->btP[childIdx].rect.w,
      atlasP->btP[parentIdx].remH - atlasP->btP[parentIdx].rect.h - atlasP->btP[childIdx].rect.h);
  }
}

static inline U32 _rectFitsToTheRight(Rect_ *orphanRectP, _AtlasElem *parentElemP) {
  return (orphanRectP->w <=  parentElemP->remW 
       && orphanRectP->h <= (parentElemP->remH + parentElemP->rect.h));
}

static inline U32 _rectFitsBelow(Rect_ *orphanRectP, _AtlasElem *parentElemP) {
  return (orphanRectP->w <= (parentElemP->remW + parentElemP->rect.w)
       && orphanRectP->h <=  parentElemP->remH);
}

// To guard against segfaults from adding children to last element
#define N_PADDING_ELEMS (2)

inline static void _setRectData(_AtlasElem *elP, U32 _max, S32 w, S32 h, Key srcIdx) {
  elP->maxDim = _max;  // larger of height or width
  elP->srcIdx = srcIdx;  // index of sample in original array
  elP->rect.x = 0;
  elP->rect.y = 0;
  elP->remW = 0;
  elP->remH = 0;
  elP->rect.w = w;
  elP->rect.h = h;
}

// Sort colormaps by largest dimension
Error atlasNew(Atlas **atlasPP, const U32 N_SAMPLES, Colormap **cmPA) {
  if (!atlasPP || !N_SAMPLES || !cmPA) {
    return E_BAD_ARGS;
  }

  // We give the atlas 2 extra elements to reflect the extra 2 children 
  // its corresponding binary tree may need for safety.
  Error e = jbAlloc((void**) atlasPP, sizeof(Atlas), 1);
  if (!e) {
    e = btNew((void**) &(*atlasPP)->btP, sizeof(_AtlasElem), N_SAMPLES);
  }

  if (!e) {
    _AtlasElem *atlasA = (*atlasPP)->btP;
    // Loop through unsorted rectangles
    atlasA[0].srcIdx = 0;
    atlasA[0].maxDim = cmPA[0]->w > cmPA[0]->h ?  cmPA[0]->w : cmPA[0]->h;
    atlasA[0].rect.w = cmPA[0]->w;
    atlasA[0].rect.h = cmPA[0]->h;

    (*atlasPP)->extremityA[0] = 0;
    (*atlasPP)->extremityA[1] = 0;

    // Loop through the unsorted rectangles
    for (U32 i = 1; i < N_SAMPLES; ++i) {
      U32 currRectMaxDim = cmPA[i]->w > cmPA[i]->h ?
                           cmPA[i]->w : cmPA[i]->h;
      // Loop through sorted rectangles to see where the current unsorted one should go.
      for (U32 j = 0; j < i; ++j) {
        if (currRectMaxDim > atlasA[j].maxDim) {
          memcpy(&atlasA[j + 1], &atlasA[j], sizeof(_AtlasElem) * (i - j));
          _setRectData(&atlasA[j], currRectMaxDim, cmPA[i]->w, cmPA[i]->h, i);
          goto nextUnsortedRect;
        }
      }
      // If loop ended without placing rect anywhere, it belongs in last element.
      _setRectData(&atlasA[i], currRectMaxDim, cmPA[i]->w, cmPA[i]->h, i);
      nextUnsortedRect:
      continue;
    }
  }
  return e;
}

void atlasDel(Atlas **atlasPP) {
  if (atlasPP) {
    btDel_((void**) &(*atlasPP)->btP);
    jbFree((void**) atlasPP);
  }
}

// Texture atlas
Error atlasPlanPlacements(Atlas *atlasP) {
  // We never search for rect space in orphan nodes, so we must only check the root for orphan-hood.
  if (!atlasP || btIsAnOrphan_(atlasP->btP, 0)) {
    return E_BAD_ARGS;
  }
  // For each sorted rectangle...
  _AtlasElem *btP = atlasP->btP;;
  _AtlasElem *orphanP = btP + 1;  // 0th elem gets pre-inserted into atlas
  _AtlasElem *elemEndP = btP + arrayGetNElems(btP);
  U32 parentIdx, cameFromRight, parentEndIdx;
  assert(atlasP->extremityA[RIGHT_RECT] == 0);
  // Initialize the first atlas elem manually so the elem ini magic works branchlessly.
  btP[0].remW = btP[0].rect.w;
  btP[0].remH = btP[0].rect.h;
  Key *childA;
  Error e = SUCCESS;
  // Loop through orphan elements
  for (parentEndIdx = arrayGetNElems(btP); !e && orphanP < elemEndP; ++orphanP) {
    // Forward search in texture atlas
    searchForward:  // moves only right and down till a fit or a dead-end is found
    /* Since you're going to navigate rightward till you find nothing else to the right,
       you  may as well start your search there. */
    for (parentIdx = atlasP->extremityA[RIGHT_RECT]; parentIdx < parentEndIdx;) {
      // Navigate to the right as far as you can go.
      childA = btP[parentIdx].header.childA;
      if (childA[RIGHT_RECT]) {  // nonzero child is a popualted one unless both children = 0x33
        parentIdx = childA[RIGHT_RECT];
        continue;
      }
      if (childA[LOWER_RECT]) {
        parentIdx = childA[LOWER_RECT];
        continue;
      }
      if (parentIdx) {
        if (_rectFitsToTheRight(&orphanP->rect, &btP[parentIdx])) {
          e = _atlasLinkNodes(atlasP, parentIdx, orphanP - btP, RIGHT_RECT);
          goto nextOrphan;
        }
        // Does rect fit beneath? (right in binary tree-speak means rect to the right)
        if (_rectFitsBelow(&orphanP->rect, &btP[parentIdx])) {
          e = _atlasLinkNodes(atlasP, parentIdx, orphanP - btP, LOWER_RECT);
          goto nextOrphan;
        }
      }
      // Dead-end. Back out until an unexplored lower direction is found.
      goto backOut;
    }

    // Back out of dead ends
    backOut:  // moves only up and left till an unexplored downward direction or root is found
    while (parentIdx) {
      parentIdx = btP[parentIdx].header.parent;
      // Avoid re-entering a path you just exited by only going up and left.
      cameFromRight = parentIdx == btP[parentIdx].header.childA[RIGHT_RECT];
      // If you came from the right and see an unexplored path below, go down.
      if (cameFromRight && _rectFitsBelow(&orphanP->rect, &btP[parentIdx])) {
        parentIdx = btP[parentIdx].header.childA[LOWER_RECT];
        goto searchForward;
      }
    }

    // If no suitable parent was found (parentIdx == 0 now), it's time to expand the atlas.
    U32 todoDeleteThis = 
        ((orphanP->rect.h <= btP[0].remH))                              // can right
      | ((btP[0].remH >= btP[0].remW  + orphanP->rect.w)  << 1)  // should right
      | ((orphanP->rect.w <= btP[0].remW)  << 2)                         // can down
      | ((btP[0].remW  >= btP[0].remH + orphanP->rect.h)  << 3); // should down
    switch (todoDeleteThis) {
      case SHOULD_RIGHT_CAN_DOWN:
      case SHOULD_RIGHT_DOWN:
      case SHOULD_RIGHT:
      case CAN_RIGHT:
      default:
        if (!e) {
          btP[0].remW += orphanP->rect.w;
          //btP[atlasP->extremityA[RIGHT_RECT]].remW = orphanP->rect.w;
          e = _atlasLinkNodes(atlasP, atlasP->extremityA[RIGHT_RECT], orphanP - btP, RIGHT_RECT);
          if (!e) {
            atlasP->extremityA[RIGHT_RECT] = 
              btP[atlasP->extremityA[RIGHT_RECT]].header.childA[RIGHT_RECT];
          }
        }
        break;
      case CAN_RIGHT_SHOULD_DOWN:
      case SHOULD_DOWN:
      case CAN_DOWN:
        if (!e) {
          btP[0].remH += orphanP->rect.h;
          e = _atlasLinkNodes(atlasP, atlasP->extremityA[LOWER_RECT], orphanP - btP, LOWER_RECT);
          if (!e) {
            // Update the lowest extremity.
            atlasP->extremityA[LOWER_RECT] = 
              btP[atlasP->extremityA[LOWER_RECT]].header.childA[LOWER_RECT];
            //btP[atlasP->extremityA[LOWER_RECT]].remH = orphanP->rect.h;
          }
        }
        break;
    }
    nextOrphan:
    continue;
  }  // for each sorted rectangle

  return e;
}

#define N_SAMPLES (3)
int main(int argc, char **argv) {
  // ====================================================
  // Repeat things done in multithreadingg test for setup
  // ====================================================
  ColorPalette *cpPA[N_SAMPLES] = {
    &blehColorPalette,
    &redColorPalette,
    &heckColorPalette
  };
  // Mock up some genes.
  Colormap *cmPA[N_SAMPLES] = {
    &blehColormap,
    &redColormap,
    &heckColormap
  };

  // Offset color palette and colormap indices to match future texture atlas destinations
  U8 atlasOffset = 0;
  for (int i = 0; i < N_SAMPLES; ++i) {
    cpPA[i]->atlasPaletteOffset = cmPA[i]->sdP->ss.offset = atlasOffset;
    atlasOffset += cpPA[i]->nColors;
  }

  // Extract stripdatas into an array so we can multithread process them all.
  StripDataS **sdPA = NULL;
  Error e = arrayNew((void**) &sdPA, sizeof(StripDataS*), N_SAMPLES);

  if (!e) {
    for (U32 i = 0, iEnd = arrayGetNElems(sdPA); i < iEnd; ++i) {
      sdPA[i] = cmPA[i]->sdP;
    }
  }

#define MULTITHREADED 1
  // Inflate colormap inflatables
#if MULTITHREADED
  if (!e) {
    e = multithread_(sdInflate, (void*) sdPA);
  }
  // Unpack stripsets
  if (!e) {
    e = multithread_(sdUnpack, (void*) sdPA);
  }
  // Assemble colormaps from strips
  if (!e) {
    //e = multithread_(sdAssemble, (void*) sdPA);
  }
#else 
  for (int i = 0; !e && i < N_SAMPLES; ++i) {
    e = sdInflate(cmPA[i]->sdP);
    if (!e) {
      sdUnpack(cmPA[i]->sdP);
    }
  }
#endif

  // Create an unfinished atlas with colormap rectangles sorted by size in descending order.
  Atlas *atlasP = NULL;
  if (!e) {
    e = atlasNew(&atlasP, N_SAMPLES, cmPA);
  }
  // Texture atlas
  if (!e) {
    e = atlasPlanPlacements(atlasP);
  }
  // Texture atlas array
  U8 *atlasPixelA = NULL;
  Surface_ *textureSurfaceP = NULL;
  if (!e) {
    e = arrayNew((void**) &atlasPixelA, sizeof(U8), atlasP->btP[0].remW * atlasP->btP[0].remH);
  }
  for (int i = 0 ; i < 3; i++) {
    printf("{%d, %d, %d, %d}, {%d, %d}\n", 
        atlasP->btP[i].rect.x,
        atlasP->btP[i].rect.y,
        atlasP->btP[i].rect.w,
        atlasP->btP[i].rect.h,
        atlasP->btP[i].remW,
        atlasP->btP[i].remH
        );
  }
  if (!e) {
    const U32 ATLAS_WIDTH = atlasP->btP[0].remW;
    U32 nStripsPerRow;
    StripmapElem *smElemP, *smElemEndP;
    U8 *dstP;
    U32 nUnitsPerStrip;
    U32 srcIdx;
    // For each sample...
    for (int i = 0; i < N_SAMPLES; ++i) {
      srcIdx = atlasP->btP[i].srcIdx;
      nUnitsPerStrip = cmPA[srcIdx]->sdP->ss.nUnitsPerStrip;
      nStripsPerRow = cmPA[srcIdx]->w / nUnitsPerStrip;
      smElemP    = (StripmapElem*) cmPA[srcIdx]->sdP->sm.infP->inflatedDataP;
      // For each row of this sample's atlas rectangle...
      for (int j = 0; j < atlasP->btP[i].rect.h; ++j) {
        smElemEndP = smElemP + nStripsPerRow;
        dstP       = atlasPixelA + atlasP->btP[i].rect.x + (j + atlasP->btP[i].rect.y) * ATLAS_WIDTH;
        // Paste rectangle row
        for (; smElemP < smElemEndP; ++smElemP, dstP += nUnitsPerStrip) {
          memcpy(dstP, 
                 cmPA[srcIdx]->sdP->ss.unpackedDataP + (*smElemP * nUnitsPerStrip), 
                 nUnitsPerStrip);
        }
      }
    }
  }
#if 1
  // Texture surface
  if (!e) {
    textureSurfaceP = SDL_CreateRGBSurfaceWithFormatFrom((void*) atlasPixelA, 
        atlasP->btP[0].remW, atlasP->btP[0].remH, 8, atlasP->btP[0].remW, SDL_PIXELFORMAT_INDEX8);
  }
  if (!textureSurfaceP) {
    e = E_NO_MEMORY;
  }
  // Texture surface palette
  if (!e) {
    for (int i = 0 ; i < N_SAMPLES; ++i) {
      SDL_SetPaletteColors(textureSurfaceP->format->palette, 
          cpPA[i]->colorA, cpPA[i]->atlasPaletteOffset, cpPA[i]->nColors);
    }
  }
  // Sanity check
  // Window and renderer 
  Renderer_ *rendererP = NULL;
  Window_ *windowP = NULL;
  if (!e) {
    e = guiNew(&windowP, &rendererP);
  }
  // Texture
  Texture_ *textureP = NULL;
  if (!e) {
    e = textureNew(&textureP, rendererP, textureSurfaceP);
  }
  // Render it
  if (!e) {
    clearScreen(rendererP);
    copy_(rendererP, textureP, NULL, NULL);
    // Show it
    present_(rendererP);
    SDL_Delay(2000);
  }
#endif

  // Deflate colormap inflatables
  if (!e) {
#if MULTITHREADED
    e = multithread_(stripClr,   (void*) sdPA);
#else
    for (int i = 0; !e && i < N_SAMPLES; ++i) {
      stripClr(cmPA[i]->sdP);
    }
#endif
  }

  // ====================================================
  // Clean up
  // ====================================================
  arrayDel((void**) &atlasPixelA);
  atlasDel(&atlasP);
  arrayDel((void**) &sdPA);
  SDL_DestroyTexture(textureP);
  SDL_FreeSurface(textureSurfaceP);
  SDL_Quit();
  return e;
}
