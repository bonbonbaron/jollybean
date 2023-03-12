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
    remWidth,   // remaining atlas width  as if this rect weren't here
    remHeight;  // remaining atlas height as if this rect weren't here
  U32 srcIdx;   // index in source data  // can you eliminate ths with BT Elem?
} _AtlasElem;

typedef struct {
  Child extremites[2];
  Key nextOrphanIdx;
  _AtlasElem *btP;
} Atlas;

static inline Error _atlasLinkNodes(
    Atlas *atlasP,
    Key parent, 
    Child child, 
    U32 x, 
    U32 y, 
    U32 remW, 
    U32 remH) {
  Key childIdx = atlasP->nextOrphanIdx++;
  if (!childIdx || childIdx >= arrayGetNElems(atlasP->btP)) {
    return E_SEGFAULT;
  }
  printf("birthing child %d out of %d\n", childIdx + 1, arrayGetNElems(atlasP->btP));
  atlasP->btP[childIdx].rect.x = x,
  atlasP->btP[childIdx].rect.y = y,
  atlasP->btP[childIdx].remWidth = remW;
  atlasP->btP[childIdx].remHeight = remH;
  btLinkNodes_(atlasP->btP, &atlasP->btP[parent], &atlasP->btP[childIdx], child);
  printf("setting xy = {%d, %d}\n", atlasP->btP[childIdx].rect.x, atlasP->btP[childIdx].rect.y);
  return SUCCESS;
}

static Error _atlasFill(Atlas *atlasP, const Key parent, Child child) {
  // Current node
  printf("filling node %d out of %d\n", parent + 1, arrayGetNElems(atlasP->btP));
  // rectHeight splits node to a shelf rightward and full width downward
  if (child == RIGHT_RECT) {
    return _atlasLinkNodes(
      atlasP,
      parent,
      RIGHT_RECT, 
      atlasP->btP[parent].rect.x + atlasP->btP[parent].rect.w,
      atlasP->btP[parent].rect.y,
      atlasP->btP[parent].remWidth - atlasP->btP[parent].rect.w,
      atlasP->btP[parent].rect.h);  
  }
  else {  /* implicit "if child == LOWER_RECT */
    return _atlasLinkNodes(
      atlasP,
      parent,
      LOWER_RECT, 
      atlasP->btP[parent].rect.x,
      atlasP->btP[parent].rect.y + atlasP->btP[parent].rect.h,
      atlasP->btP[parent].remWidth,
      atlasP->btP[parent].remHeight - atlasP->btP[parent].rect.h);
  }
}

static inline U32 _rectFits(Rect_ *rectP, _AtlasElem *atlasElemP) {
  return (rectP->w <= atlasElemP->remWidth && rectP->h <= atlasElemP->remHeight);
}

// To guard against segfaults from adding children to last element
#define N_PADDING_ELEMS (2)

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

    // Loop through the unsorted rectangles
    for (U32 i = 1; i < N_SAMPLES; ++i) {
      U32 currRectMaxDim = cmPA[i]->w > cmPA[i]->h ?
                           cmPA[i]->w : cmPA[i]->h;
      // Loop through sorted rectangles to see where the current unsorted one should go.
      for (U32 j = 0; j < i; ++j) {
        if (currRectMaxDim > atlasA[j].maxDim) {
          memcpy(&atlasA[j + 1], &atlasA[j], sizeof(_AtlasElem) * (i - j));
          atlasA[j].maxDim = currRectMaxDim;  // larger of height or width
          atlasA[j].srcIdx = i;  // index of sample in original array
          //atlasA[j].rect.x = 0;
          //atlasA[j].rect.y = 0;
          atlasA[j].rect.w = cmPA[i]->w;
          atlasA[j].rect.h = cmPA[i]->h;
          goto nextUnsortedRect;
        }
      }
      // If loop ended without placing rect anywhere, it belongs in last element.
      atlasA[i].maxDim = currRectMaxDim;  // larger of height or width
      atlasA[i].srcIdx = i;  // index of sample in original array
      //atlasA[i].rect.x = 0;
      //atlasA[i].rect.y = 0;
      atlasA[i].rect.w = cmPA[i]->w;
      atlasA[i].rect.h = cmPA[i]->h;
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
  if (!atlasP) {
    return E_BAD_ARGS;
  }

  // For each sorted rectangle...
  _AtlasElem *elemP = atlasP->btP + 1;  // 0th elem gets pre-inserted into atlas
  _AtlasElem *elemEndP = atlasP->btP + arrayGetNElems(atlasP->btP);
  Child child = LOWER_RECT;
  U32 searchIdx, 
      parent, 
      cameFromRight;
  // Initialize the first atlas elem manually so the elem ini magic works branchlessly.
  atlasP->btP[0].rect.x = 0;
  atlasP->btP[0].rect.y = 0;
  atlasP->btP[0].remWidth = atlasP->btP[0].rect.w;
  atlasP->btP[0].remHeight = atlasP->btP[0].rect.h;
  Key *childA;
  Error e = SUCCESS;
  for (searchIdx = 0; !e && elemP < elemEndP; ++elemP) {
    // Forward search in texture atlas
    searchForward:  // moves only right and down till a fit or a dead-end is found
    while (searchIdx < atlasP->nextOrphanIdx) {
      // If current node is not used, fill it (since we've already proven it fits here).
      if (!btIsAnOrphan_(&atlasP->btP[searchIdx].header)) {
        e = _atlasFill(atlasP, searchIdx, child);
        goto nextRect;
      }
      // Does rect fit to the right? (left in binary tree-speak means rect to the right)
      childA = atlasP->btP[searchIdx].header.childA;  // left is index 0
      if (_rectFits(&elemP->rect, &atlasP->btP[childA[RIGHT_RECT]])) {
        searchIdx = childA[child = RIGHT_RECT];
        continue;
      }
      // Does rect fit beneath? (right in binary tree-speak means rect to the right)
      if (_rectFits(&elemP->rect, &atlasP->btP[childA[LOWER_RECT]])) {
        searchIdx = childA[child = LOWER_RECT];
        continue;
      }
      // Dead-end. Back out until an unexplored lower direction is found.
      goto backOut;
    }

    // Backing out of dead ends
    backOut:  // moves only up and left till an unexplored downward direction or root is found
    while (searchIdx) {
      parent = btP->idxA[searchIdx].parent;
      cameFromRight = searchIdx == btP->idxA[parent].childA[RIGHT_RECT];
      searchIdx = parent;
      if (cameFromRight // sneakily populated in getParentAtlasIdx_()
          && _rectFits(&elemP->rect, atlasA + btP->idxA[searchIdx].childA[LOWER_RECT])) {
        searchIdx = btP->idxA[searchIdx].childA[LOWER_RECT];
        goto searchForward;
      }
    }

    // If no space was found (searchIdx == 0 now), it's time to expand the atlas.
    U32 todoDeleteThis = 
        ((elemP->rect.h <= atlasA[0].remHeight))                              // can right
      | ((atlasA[0].remHeight >= atlasA[0].remWidth  + elemP->rect.w)  << 1)  // should right
      | ((elemP->rect.w <= atlasA[0].remWidth)  << 2)                         // can down
      | ((atlasA[0].remWidth  >= atlasA[0].remHeight + elemP->rect.h)  << 3); // should down
    switch (todoDeleteThis) {
      case SHOULD_RIGHT_CAN_DOWN:
      case SHOULD_RIGHT_DOWN:
      case SHOULD_RIGHT:
      case CAN_RIGHT:
      default:
        if (!e) {
          atlasA[0].remWidth += elemP->rect.w;
          atlasA[btP->extremityA[RIGHT_RECT]].remWidth = elemP->rect.w;
          e = _atlasFill(atlasA, btP, btP->extremityA[RIGHT_RECT]);
        }
        break;
      case CAN_RIGHT_SHOULD_DOWN:
      case SHOULD_DOWN:
      case CAN_DOWN:
        if (!e) {
          atlasA[0].remHeight += elemP->rect.h;
          atlasA[btP->extremityA[LOWER_RECT]].remHeight = elemP->rect.h;
          e = _atlasFill(atlasA, btP, btP->extremityA[LOWER_RECT]);
        }
        break;
    }
    nextRect:
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
#if 1
  // Texture atlas array
  U8 *atlasPixelA = NULL;
  Surface_ *textureSurfaceP = NULL;
  if (!e) {
    e = arrayNew((void**) &atlasPixelA, sizeof(U8), atlasA[0].remWidth * atlasA[0].remHeight);
  }
  if (!e) {
    const U32 ATLAS_WIDTH = atlasA[0].remWidth;
    U32 nStripsPerRow;
    StripmapElem *smElemP, *smElemEndP;
    U8 *dstP;
    U32 nUnitsPerStrip;
    U32 srcIdx;
    // For each sample...
    for (int i = 0; i < N_SAMPLES; ++i) {
      srcIdx = atlasA[i].srcIdx;
      nUnitsPerStrip = cmPA[srcIdx]->sdP->ss.nUnitsPerStrip;
      nStripsPerRow = cmPA[srcIdx]->w / nUnitsPerStrip;
      smElemP    = (StripmapElem*) cmPA[srcIdx]->sdP->sm.infP->inflatedDataP;
      // For each row of this sample's atlas rectangle...
      for (int j = 0; j < atlasA[i].rect.h; ++j) {
        smElemEndP = smElemP + nStripsPerRow;
        dstP       = atlasPixelA + atlasA[i].rect.x + (j + atlasA[i].rect.y) * ATLAS_WIDTH;
        // Paste rectangle row
        for (; smElemP < smElemEndP; ++smElemP, dstP += nUnitsPerStrip) {
          memcpy(dstP, 
                 cmPA[srcIdx]->sdP->ss.unpackedDataP + (*smElemP * nUnitsPerStrip), 
                 nUnitsPerStrip);
        }
      }
    }
  }
  // Texture surface
  if (!e) {
    textureSurfaceP = SDL_CreateRGBSurfaceWithFormatFrom((void*) atlasPixelA, 
        atlasA[0].remWidth, atlasA[0].remHeight, 8, atlasA[0].remWidth, SDL_PIXELFORMAT_INDEX8);
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
  arrayDel((void**) &atlasA);
  atlasDel((void**) &atlasP);
  arrayDel((void**) &sdPA);
  return e;
}
