#include "interface.h"
#include "data.h"
#include "blehColormap.h"
#include "blehColorPalette.h"
#include "heckColormap.h"
#include "heckColorPalette.h"
#include "redColormap.h"
#include "redColorPalette.h"
#include "previewImg.h"

#define TEXTURE_MAX_DIM_    (4096)
#define CAN_RIGHT     (0x00000001)
#define SHOULD_RIGHT  (0x00000003)  // implies "can"
#define CAN_DOWN      (0x00000004)
#define SHOULD_DOWN   (0x0000000c)  // implies "can"
#define SHOULD_RIGHT_CAN_DOWN (SHOULD_RIGHT | CAN_DOWN)
#define CAN_RIGHT_SHOULD_DOWN (SHOULD_DOWN | CAN_RIGHT)
#define SHOULD_RIGHT_DOWN  (SHOULD_RIGHT | SHOULD_DOWN)

typedef struct {
  Rect_ rect;
  U32 maxDim,
    remWidth,   // remaining atlas width  as if this rect weren't here
    remHeight;  // remaining atlas height as if this rect weren't here
  U32 srcIdx;   // index in source data
} _AtlasElem;

static inline void _atlasElemChildIni(_AtlasElem *childP, U32 x, U32 y, U32 remW, U32 remH) {
  childP->rect.x = x,
  childP->rect.y = y,
  childP->remWidth = remW;
  childP->remHeight = remH;
}

static void _atlasElemIni(_AtlasElem *rootP, BinaryTree *btP, BinaryTreeElem *currBinTreeElP) {
  U32 atlasIdx = currBinTreeElP - btP->idxA;
  // Current node
  binaryTreeElemSetUsed(currBinTreeElP);
  // Right rect
  Error e = binaryTreeAddChild(btP, LEFT_CHILD, currBinTreeElP);  // confusing: left child is rect to the right
  if (!e) {
    _atlasElemChildIni(
        &rootP[currBinTreeElP->childIdxA[LEFT_CHILD]], 
        rootP[atlasIdx].rect.x + rootP[atlasIdx].rect.w,
        rootP[atlasIdx].rect.y,
        rootP[atlasIdx].remWidth - rootP[atlasIdx].rect.w,
        rootP[atlasIdx].rect.h);  // rectHeight splits node to a shelf rightward and full width downward
  // Bottom rect
    e = binaryTreeAddChild(btP, RIGHT_CHILD, currBinTreeElP);  // confusing: left child is rect to the right
  }
  if (!e) {
    _atlasElemChildIni(
        &rootP[currBinTreeElP->childIdxA[RIGHT_CHILD]], 
        rootP[atlasIdx].rect.x,
        rootP[atlasIdx].rect.y + rootP[atlasIdx].rect.h,
        rootP[atlasIdx].remWidth,
        rootP[atlasIdx].remHeight - rootP[atlasIdx].rect.h);
  }
}

static inline U32 _rectFits(Rect_ *rectP, _AtlasElem *atlasElemP) {
  return (rectP->w <= atlasElemP->remWidth && rectP->h <= atlasElemP->remHeight);
}

// Sort colormaps by largest dimension
Error atlasNew(_AtlasElem **atlasAP, const U32 N_SAMPLES, Colormap **cmPA) {
  if (!atlasAP || N_SAMPLES || !cmPA) {
    return E_BAD_ARGS;
  }

  Error e = arrayNew((void**) atlasAP, sizeof(_AtlasElem), N_SAMPLES);

  if (!e) {
    _AtlasElem *atlasA = *atlasAP;
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

// Texture atlas
Error atlasPlanPlacements(_AtlasElem *atlasA) {
  if (!atlasA) {
    return E_BAD_ARGS;
  }
  // Binary tree tracks parent-child relationships of all atlas array elements.
  BinaryTree *btP;
  Error e = binaryTreeNew(&btP, arrayGetNElems(atlasA));

  if (!e) {
    _AtlasElem *atlasA = *atlasAP;
    // For each sorted rectangle...
    _SortedRect *sortedRectP = sortedRectA + 1;  // 0th elem gets pre-inserted into atlas
    _SortedRect *sortedRectEndP = sortedRectA + arrayGetNElems(sortedRectA);
    U32 n_AtlasElems = arrayGetNElems(atlasA), searchIdx, cameFromRight,
    nGrowthsRight = 0, nGrowthsDown  = 0;
    // Initialize the first atlas elem manually so the elem ini magic works branchlessly.
    atlasA[0].rect.x = 0;
    atlasA[0].rect.y = 0;
    atlasA[0].remWidth = sortedRectA[0].rect.w;
    atlasA[0].remHeight = sortedRectA[0].rect.h;
    _atlasElemIni(atlasA, 0, &sortedRectA[0]);
    for (searchIdx = 0; sortedRectP < sortedRectEndP; ++sortedRectP) {
      // Forward search in texture atlas
      searchForward:  // moves only right and down till a fit or a dead-end is found
      while (searchIdx < n_AtlasElems) {
        // If current node is not used, fill it (since we've already proven it fits here).
        if (!atlasA[searchIdx].used) {
          _atlasElemIni(atlasA, searchIdx, sortedRectP);
          goto nextRect;
        }
        // Does rect fit to the right?
        if (_rectFits(&sortedRectP->rect, atlasA + getRightAtlasChildIdx_(searchIdx))) {
          searchIdx = getRightAtlasChildIdx_(searchIdx);
          continue;
        }
        // Does rect fit beneath?
        else if (_rectFits(&sortedRectP->rect, atlasA + getRightAtlasChildIdx_(searchIdx))) {
          searchIdx = getLowerAtlasChildIdx_(searchIdx);
          continue;
        }
        // If you've hit a dead-end, back out until an unexplored lower direction is found.
        goto backOut;
      }

      // Backing out of dead ends
      backOut:  // moves only up and left till an unexplored downward direction or root is found
      while (searchIdx) {
        searchIdx = getParentAtlasIdx_(searchIdx);
        if (cameFromRight // sneakily populated in getParentAtlasIdx_()
            && _rectFits(&sortedRectP->rect, atlasA + getLowerAtlasChildIdx_(searchIdx))) {
          searchIdx = getLowerAtlasChildIdx_(searchIdx);
          goto searchForward;
        }
      }

      // If no space was found (searchIdx == 0 now), it's time to expand the atlas.
      U32 todoDeleteThis = ((sortedRectP->rect.h <= atlasA[0].remHeight))         // can right
        | ((atlasA[0].remHeight >= atlasA[0].remWidth  + sortedRectP->rect.w)  << 1) // should right
        | ((sortedRectP->rect.w <= atlasA[0].remWidth)  << 2)  // can down
        | ((atlasA[0].remWidth  >= atlasA[0].remHeight + sortedRectP->rect.h)  << 3); // should down
      switch (todoDeleteThis) {
        case SHOULD_RIGHT_CAN_DOWN:
        case SHOULD_RIGHT_DOWN:
        case SHOULD_RIGHT:
        case CAN_RIGHT:
          atlasA[0].remWidth += sortedRectP->rect.w;
          atlasA[getNthRightDescendant_(++nGrowthsRight)].remWidth = sortedRectP->rect.w;
          _atlasElemIni(atlasA, getNthRightDescendant_(nGrowthsRight), sortedRectP);
          break;
        case CAN_RIGHT_SHOULD_DOWN:
        case SHOULD_DOWN:
        case CAN_DOWN:
          atlasA[0].remHeight += sortedRectP->rect.h;
          atlasA[getNthLowerDescendant_(++nGrowthsDown)].remHeight = sortedRectP->rect.h;
          _atlasElemIni(atlasA, getNthLowerDescendant_(nGrowthsDown), sortedRectP);
          break;
        default:
          break;
      }
      nextRect:
      continue;
    }  // for each sorted rectangle
  }  // if binary tree is successfully made

  binaryTreeDel(&btP);
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

  // Sort rectangles
  _SortedRect *sortedRectA = NULL;
  if (!e) {
    e = sortRects(&sortedRectA, N_SAMPLES, cmPA);
  }

  // Texture atlas
  _AtlasElem *atlasA = NULL;
  U8 *atlasPixelA = NULL;
  Surface_ *textureSurfaceP = NULL;
  if (!e) {
    e = atlasPlanPlacements(&atlasA, N_SAMPLES, sortedRectA);
  }
  // Texture atlas array
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
      srcIdx = sortedRectA[i].srcIdx;
      nUnitsPerStrip = cmPA[srcIdx]->sdP->ss.nUnitsPerStrip;
      nStripsPerRow = cmPA[srcIdx]->w / nUnitsPerStrip;
      smElemP    = (StripmapElem*) cmPA[srcIdx]->sdP->sm.infP->inflatedDataP;
      // For each row of this sample's atlas rectangle...
      for (int j = 0; j < sortedRectA[i].rect.h; ++j) {
        smElemEndP = smElemP + nStripsPerRow;
        dstP       = atlasPixelA + sortedRectA[i].rect.x + (j + sortedRectA[i].rect.y) * ATLAS_WIDTH;
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
  arrayDel((void**) &sortedRectA);
  binaryTreeDel_((void**) &atlasA);
  arrayDel((void**) &sdPA);
  return e;
}
