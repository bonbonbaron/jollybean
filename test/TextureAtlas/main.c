#include "xRender.h"
#include "blehColormap.h"
#include "blehColorPalette.h"
#include "heckColormap.h"
#include "heckColorPalette.h"
#include "redColormap.h"
#include "redColorPalette.h"
#include "previewImg.h"

typedef struct {
  U8  used;       // Keeps track of whether this binary tree node is used or not
  Key srcCmIdx;   // index of colormap in source array
  U32 remWidth;   // remaining width
  U32 remHeight;  // remaining height
} AtlasElem;

typedef struct {
  U32 maxDim;
  U32 idx;
} SortedRect;

void atlasElemIni(AtlasElem *elemP, U32 remWidth, U32 remHeight, Key srcIdx) {
  elemP->remWidth = remWidth;
  elemP->remHeight = remHeight;
  elemP->srcCmIdx = srcIdx;
  elemP->used = 1;
}

#define N_SAMPLES (3)
int main(int argc, char **argv) {
  Gene **genePA = NULL;

  // ====================================================
  // Repeat things done in multithreadingg test for setup
  // ====================================================
  Error e = arrayNew((void**) &genePA, sizeof(void*), N_SAMPLES);

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
  // pritn dimensions
  for (U32 i = 0; i < N_SAMPLES; ++i) {
    printf("%d: w: %d, h: %d\n", i, ((Colormap*) genePA[i]->dataP)->w, ((Colormap*) genePA[i]->dataP)->h);
  }

  // Extract stripdata from each media gene into an array of contiguous strip data pointers.
  StripDataS **sdPA = NULL;
  if (!e) {
    e = arrayNew((void**) &sdPA, sizeof(StripDataS*), arrayGetNElems(genePA));
  }
  if (!e) {
    for (U32 i = 0, iEnd = arrayGetNElems(sdPA); i < iEnd; ++i) {
      sdPA[i] = *((StripDataS**) genePA[i]->dataP);
    }
  }

  // Inflate colormap inflatables
  if (!e) {
    e = multiThread(sdInflate, (void*) sdPA);
  }
  // Unpack stripsets
  if (!e) {
    e = multiThread(sdUnpack, (void*) sdPA);
  }
  // Assemble colormaps from strips
  if (!e) {
    e = multiThread(sdAssemble, (void*) sdPA);
  }
  // Deflate colormap inflatables
  if (!e) {
    e = multiThread(stripClr,   (void*) sdPA);
  }

  // ====================================================
  // Texture atlas algo
  // ====================================================
  // Sort colormaps by largest dimension
  SortedRect *sortedRectA = NULL;
  if (!e) {
    e = arrayNew((void**) &sortedRectA, sizeof(SortedRect), N_SAMPLES);
  }
  if (!e) {
    // Loop through unsorted rectangles
    sortedRectA[0].idx = 0;
    sortedRectA[0].maxDim = ((Colormap*) genePA[0]->dataP)->w > ((Colormap*) genePA[0]->dataP)->h ?
                            ((Colormap*) genePA[0]->dataP)->w : ((Colormap*) genePA[0]->dataP)->h;
    for (U32 i = 1; i < N_SAMPLES; ++i) {
      // Loop through sorted rectangles
      U32 currRectMaxDim = ((Colormap*) genePA[i]->dataP)->w > ((Colormap*) genePA[i]->dataP)->h ?
                           ((Colormap*) genePA[i]->dataP)->w : ((Colormap*) genePA[i]->dataP)->h;
      for (U32 j = 0; j <= i; ++j) {
        if (currRectMaxDim > sortedRectA[j].maxDim) {
          memcpy(&sortedRectA[j + 1], &sortedRectA[j], sizeof(SortedRect) * (1 + i - j));
          sortedRectA[j].maxDim = currRectMaxDim;  // larger of height or width
          sortedRectA[j].idx    = i;  // index of sample in original array
          break;
        }
        else if (j == i) {
          sortedRectA[j].maxDim = currRectMaxDim;  // larger of height or width
          sortedRectA[j].idx    = i;  // index of sample in original array
        }
      }
    }
  }

#define RIGHT (0)
#define DOWN  (1)
#define TEXTURE_MAX_DIM_ = (4096)
  // Binary tree of rects
  AtlasElem *atlasA = NULL;
  if (!e) {
    e = binaryTreeNew_((void**) atlasA, sizeof(AtlasElem), N_SAMPLES);
  }
  if (!e) {
    // For each sorted rectangle...
    SortedRect *sortedRectP = sortedRectA;
    SortedRect *sortedRectEndP = sortedRectP + arrayGetNElems(sortedRectA);
    U32 nAtlasElems = arrayGetNElems(atlasA);
    U32 rightIdx, lowerIdx;
    atlasElemIni(&atlasA[0], TEXTURE_MAX_DIM_ - genePA[sortedRectP->idx]
    for (; sortedRectP < sortedRectEndP; ++sortedRectP) {
      // ... find a spot in a binary tree of rectangles.
      for (U32 i = 0; i < nAtlasElems; /* incrementing handled below */ ) {
//#define binaryTreeDel_ arrayDel 
#define getRightAtlasElemIdx_  getLeftChildIdx_
#define getLowerAtlasElemIdx_  getRightChildIdx_
//#define getRightChildIdx_(parentIdx_) ((parentIdx_ << 1) + 2)
//#define getParentIdx_(childIdx_) ((childIdx_ - 1 - !(childIdx_ & 1)) >> 1)
        // If current node is not used, fill it.
        if (!atlasA[i].used) {
          atlasElemIni(&atlasA[i], rem, U32 remHeight, Key srcIdx);
          break;
        }
        // If current node's right child is not used, fill it if there's room.
        if (atlasA[i].remWidth > 
        rightIdx = getRightAtlasElemIdx_(i);
        if (!atlasA[rightIdx].used)
      }
    }
  }
  arrayDel((void**) &sortedRectA);
  binaryTreeDel_(&atlasA);
  // ====================================================
  // Clean up
  // ====================================================
  arrayDel((void**) &sdPA);
  for (U32 i = 0; i < arrayGetNElems(genePA); ++i) {
    jbFree((void**) &genePA[i]);
  }
  arrayDel((void**) &genePA);
  return e;
}
