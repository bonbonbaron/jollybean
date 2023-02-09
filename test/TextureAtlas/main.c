#include "interface.h"
#include "xRender.h"
#include "blehColormap.h"
#include "blehColorPalette.h"
#include "heckColormap.h"
#include "heckColorPalette.h"
#include "redColormap.h"
#include "redColorPalette.h"
#include "previewImg.h"

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

typedef struct {
  U32 used, x, y, remWidth, remHeight;  // rem = "remaining" (space remaining without this rect)
} AtlasElem;

typedef struct {
  U32 maxDim;
  U32 srcIdx;     // index in source data
  Rect_ rect;  // avoid chasing pointers back to the orignal rectangle
} SortedRect;

static inline void _atlasElemChildIni(AtlasElem *childP, U32 x, U32 y, U32 remW, U32 remH) {
  childP->x = x,
  childP->y = y,
  childP->remWidth = remW;
  childP->remHeight = remH;
  childP->used = 0;
}

static void _atlasElemIni(AtlasElem *rootP, U32 atlasIdx, SortedRect *sortedRectP) {
  printf("putting rect %d in elem %d.\n", sortedRectP->srcIdx, atlasIdx);
  // Current node
  rootP[atlasIdx].used = TRUE;
  sortedRectP->rect.x = rootP[atlasIdx].x;
  sortedRectP->rect.y = rootP[atlasIdx].y;
  // Right child
  printf("populating child %d\n", getRightAtlasChildIdx_(atlasIdx));
  _atlasElemChildIni(
      &rootP[getRightAtlasChildIdx_(atlasIdx)], 
      rootP[atlasIdx].x + sortedRectP->rect.w,
      rootP[atlasIdx].y,
      rootP[atlasIdx].remWidth - sortedRectP->rect.w,
      sortedRectP->rect.h);  // rectHeight splits node to a shelf rightward and full width downward
  // Bottom child
  printf("populating child %d\n", getLowerAtlasChildIdx_(atlasIdx));
  _atlasElemChildIni(
      &rootP[getLowerAtlasChildIdx_(atlasIdx)], 
      rootP[atlasIdx].x,
      rootP[atlasIdx].y + sortedRectP->rect.h,
      rootP[atlasIdx].remWidth,
      rootP[atlasIdx].remHeight - sortedRectP->rect.h);
}

static inline U32 _rectFits(Rect_ *rectP, AtlasElem *atlasElemP) {
  return (rectP->w <= atlasElemP->remWidth && rectP->h <= atlasElemP->remHeight);
}

// Sort colormaps by largest dimension
Error sortRects(SortedRect **sortedRectAP, const U32 N_SAMPLES, Gene **genePA) {
  if (!sortedRectAP || !genePA) {
    return E_BAD_ARGS;
  }

  Error e = arrayNew((void**) sortedRectAP, sizeof(SortedRect), N_SAMPLES);

  if (!e) {
    SortedRect *sortedRectA = *sortedRectAP;
    // Loop through unsorted rectangles
    sortedRectA[0].srcIdx = 0;
    sortedRectA[0].maxDim = ((Colormap*) genePA[0]->dataP)->w > ((Colormap*) genePA[0]->dataP)->h ?
                            ((Colormap*) genePA[0]->dataP)->w : ((Colormap*) genePA[0]->dataP)->h;
    sortedRectA[0].rect.w = ((Colormap*) genePA[0]->dataP)->w;
    sortedRectA[0].rect.h = ((Colormap*) genePA[0]->dataP)->h;

    for (U32 i = 1; i < N_SAMPLES; ++i) {
      // Loop through sorted rectangles
      U32 currRectMaxDim = ((Colormap*) genePA[i]->dataP)->w > ((Colormap*) genePA[i]->dataP)->h ?
                           ((Colormap*) genePA[i]->dataP)->w : ((Colormap*) genePA[i]->dataP)->h;
      for (U32 j = 0; j <= i; ++j) {
        if (currRectMaxDim > sortedRectA[j].maxDim) {
          memcpy(&sortedRectA[j + 1], &sortedRectA[j], sizeof(SortedRect) * (1 + i - j));
          sortedRectA[j].maxDim = currRectMaxDim;  // larger of height or width
          sortedRectA[j].srcIdx    = i;  // index of sample in original array
          //sortedRectA[j].rect.x = 0;
          //sortedRectA[j].rect.y = 0;
          sortedRectA[j].rect.w = ((Colormap*) genePA[i]->dataP)->w;
          sortedRectA[j].rect.h = ((Colormap*) genePA[i]->dataP)->h;
          break;
        }
        else if (j == i) {
          sortedRectA[j].maxDim = currRectMaxDim;  // larger of height or width
          sortedRectA[j].srcIdx    = i;  // index of sample in original array
          //sortedRectA[j].rect.x = 0;
          //sortedRectA[j].rect.y = 0;
          sortedRectA[j].rect.w = ((Colormap*) genePA[i]->dataP)->w;
          sortedRectA[j].rect.h = ((Colormap*) genePA[i]->dataP)->h;
        }
      }
    }
  }
  return e;
}

// Texture atlas
Error atlasGen(AtlasElem **atlasAP, const U32 N_SAMPLES, SortedRect *sortedRectA) {
  // Binary tree of rects

  // Allocate enough room for even the extraneous children to avoid child init branching.
  Error e = binaryTreeNew_((void**) atlasAP, sizeof(AtlasElem), N_SAMPLES * 2 + 2);

  if (!e) {
    AtlasElem *atlasA = *atlasAP;
    // For each sorted rectangle...
    SortedRect *sortedRectP = sortedRectA + 1;  // 0th elem gets pre-inserted into atlas
    SortedRect *sortedRectEndP = sortedRectA + arrayGetNElems(sortedRectA);
    U32 nAtlasElems = arrayGetNElems(atlasA), searchIdx, cameFromRight,
    nGrowthsRight = 0, nGrowthsDown  = 0;
    // Initialize the first atlas elem manually so the elem ini magic works branchlessly.
    atlasA[0].x = 0;
    atlasA[0].y = 0;
    atlasA[0].remWidth = sortedRectA[0].rect.w;
    atlasA[0].remHeight = sortedRectA[0].rect.h;
    _atlasElemIni(atlasA, 0, &sortedRectA[0]);
    for (searchIdx = 0; sortedRectP < sortedRectEndP; ++sortedRectP) {
      // Forward search in texture atlas
      searchForward:  // moves only right and down till a fit or a dead-end is found
      while (searchIdx < nAtlasElems) {
        // If current node is not used, fill it (since we're sorted).
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

      // TODO implement if (searchIdx >= nAtlasElems) safety check here if necessary.

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
      switch (todoDeleteThis
      ) {
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

  return e;
}

#define N_SAMPLES (3)
int main(int argc, char **argv) {
  Gene **genePA = NULL;

  // ====================================================
  // Repeat things done in multithreadingg test for setup
  // ====================================================
  Error e = arrayNew((void**) &genePA, sizeof(void*), N_SAMPLES);

  ColorPalette *cpPA[N_SAMPLES] = {
    &blehColorPalette,
    &redColorPalette,
    &heckColorPalette
  };
  // Mock up some genes.
  if (!e) {
    for (U32 i = 0; !e && i < N_SAMPLES; ++i) {
      Gene *geneP = NULL;
      e = jbAlloc((void**) &geneP, sizeof(Gene), 1);
      if (!e) {
        genePA[i] = geneP;
        genePA[i]->geneClass = MEDIA_GENE;
        genePA[i]->key = 0;
        genePA[i]->size = sizeof(XRenderComp);
        genePA[i]->type = RENDER;
      }
    }
    genePA[0]->dataP = &blehColormap;
    genePA[1]->dataP = &redColormap;
    genePA[2]->dataP = &heckColormap;
  }

  // Extract stripdata from each media gene into an array of contiguous strip data pointers.
  StripDataS **sdPA = NULL;
  if (!e) {
    e = arrayNew((void**) &sdPA, sizeof(StripDataS*), arrayGetNElems(genePA));
  }
  if (!e) {
    for (U32 i = 0, iEnd = arrayGetNElems(sdPA); i < iEnd; ++i) {
      // dataP should point to a struct whose first member is a StripDataS pointer,
      // so treat it like a StripDataS double-pointer.
      sdPA[i] = *((StripDataS**) genePA[i]->dataP);  
    }
    printf("i\n");
  }

  // Inflate colormap inflatables
#if 0
  if (!e) {
    e = multithread_(sdInflate, (void*) sdPA);
  }
  // Unpack stripsets
  if (!e) {
    e = multithread_(sdUnpack, (void*) sdPA);
  }
  // Assemble colormaps from strips
  if (!e) {
    e = multithread_(sdAssemble, (void*) sdPA);
  }
#else 
  for (int i = 0; !e && i < N_SAMPLES; ++i) {
    e = stripIni(*((StripDataS**) genePA[i]->dataP));
  }
#endif

  // Sort rectangles
  SortedRect *sortedRectA = NULL;
  if (!e) {
    e = sortRects(&sortedRectA, N_SAMPLES, genePA);
  }
  if (!e) {
    for (U32 i = 0; i < N_SAMPLES; ++i) {
      printf("%d: w: %d, h: %d\n", i, sortedRectA[i].rect.w, sortedRectA[i].rect.h);
    }
  }

  // Make texture atlas
  AtlasElem *atlasA = NULL;
  if (!e) {
    e = atlasGen(&atlasA, N_SAMPLES, sortedRectA);
  }

  for (int i = 0; i < N_SAMPLES; ++i) {
    Surface_ *tempSurfaceP = NULL;
    if (!e) {
      printf("looking at colormap %d\n", sortedRectA[i].srcIdx);
      e = surfaceNew(&tempSurfaceP, (Colormap*) genePA[sortedRectA[i].srcIdx]->dataP);
    }
    if (!e) {
      e = surfaceIni(tempSurfaceP, (Colormap*) genePA[i]->dataP, cpPA[i]); 
    }
    if (tempSurfaceP) {
      //SDL_FreeSurface(tempSurfaceP);
      tempSurfaceP = NULL;
    }
  }

  // Deflate colormap inflatables
  if (!e) {
    printf("before clear\n");
#if 0
    e = multithread_(stripClr,   (void*) sdPA);
#else
    for (int i = 0; !e && i < N_SAMPLES; ++i) {
      stripClr(*((StripDataS**) genePA[i]->dataP));
    }
#endif
    printf("after clear\n");
  }

  // ====================================================
  // Clean up
  // ====================================================
  arrayDel((void**) &sortedRectA);
  binaryTreeDel_((void**) &atlasA);
  arrayDel((void**) &sdPA);
  for (U32 i = 0; i < arrayGetNElems(genePA); ++i) {
    jbFree((void**) &genePA[i]);
  }
  arrayDel((void**) &genePA);
  return e;
}
