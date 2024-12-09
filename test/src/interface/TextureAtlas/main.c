#include "interface.h"
#include "blehColormap.h"
#include "blehColorPalette.h"
#include "heckColormap.h"
#include "heckColorPalette.h"
#include "redColormap.h"
#include "redColorPalette.h"
#include "previewImg.h"

int main(int argc, char **argv) {
  // ====================================================
  // Repeat things done in multithreadingg test for setup
  // ====================================================

  Image imgA[] = {
    {
      .state = 0,
      .sortedRectIdx = 0,
      .cmP = &blehColormap,
      .cpP = &blehColorPalette
    },
    {
      .state = 0,
      .sortedRectIdx = 0,
      .cmP = &redColormap,
      .cpP = &redColorPalette
    },
    {
      .state = 0,
      .sortedRectIdx = 0,
      .cmP = &heckColormap,
      .cpP = &heckColorPalette
    }
  };
 
  U32 N_SAMPLES = sizeof(imgA) / sizeof(imgA[0]);

  Image** imgPF = frayNew(sizeof(Image*), N_SAMPLES);
  for (U32 i = 0; i < N_SAMPLES; ++i) {
    frayAdd(imgPF, &imgA[i], NULL);
  }

  // Extract stripdatas into an array so we can multithread process them all.
  StripDataS **sdPA = arrayNew(sizeof(StripDataS*), N_SAMPLES);

  for (U32 i = 0, iEnd = arrayGetNElems(sdPA); i < iEnd; ++i) {
    sdPA[i] = imgPF[i]->cmP->sdP;
  }

#define MULTITHREADED 1
  // Inflate colormap inflatables
#if MULTITHREADED
  multithread_(sdInflate, (void*) sdPA);
  // Unpack stripsets
  multithread_(sdUnpack, (void*) sdPA);
  // Assemble colormaps from strips
  //multithread_(sdAssemble, (void*) sdPA);
#else 
  for (int i = 0; i < N_SAMPLES; ++i) {
    sdInflate(imgPF[i]->cmP->sdP);
    sdUnpack(imgPF[i]->cmP->sdP);
  }
#endif

  // Create an unfinished atlas with colormap rectangles sorted by size in descending order.
  Atlas *atlasP =  atlasNew(imgPF);
#if 0
  for (int i = 0 ; i < N_SAMPLES; i++) {
    printf("BEFORE planning placements:\n{%d, %d, %d, %d}, {%d, %d}, srcIdx = %d\n", 
        atlasP->btP[i].rect.x,
        atlasP->btP[i].rect.y,
        atlasP->btP[i].rect.w,
        atlasP->btP[i].rect.h,
        atlasP->btP[i].remW,
        atlasP->btP[i].remH,
        atlasP->btP[i].srcIdx
        );
  }
#endif
  // Texture atlas
  atlasPlanPlacements(atlasP);
#if 0
  for (int i = 0 ; i < N_SAMPLES; i++) {
    printf("AFTER planning placements:\n{%d, %d, %d, %d}, {%d, %d}, srcIdx = %d\n", 
        atlasP->btP[i].rect.x,
        atlasP->btP[i].rect.y,
        atlasP->btP[i].rect.w,
        atlasP->btP[i].rect.h,
        atlasP->btP[i].remW,
        atlasP->btP[i].remH,
        atlasP->btP[i].srcIdx
        );
  }
#endif
  // Texture atlas array
  U8* atlasPixelA = arrayNew( sizeof(U8), atlasP->btP[0].remW * atlasP->btP[0].remH);
  Surface_ *textureSurfaceP = NULL;
  const U32 ATLAS_WIDTH = atlasP->btP[0].remW;
  U32 nStripsPerRow;
  StripmapElem *smElemP, *smElemEndP;
  U8 *dstP;
  U32 nUnitsPerStrip;
  U32 srcIdx;
  // For each sample...
  for (int i = 0; i < N_SAMPLES; ++i) {
#if 0
    printf("drawing to {%d, %d, %d, %d}...\n", 
        atlasP->btP[i].rect.x,
        atlasP->btP[i].rect.y,
        atlasP->btP[i].rect.w,
        atlasP->btP[i].rect.h);
#endif
    srcIdx = atlasP->btP[i].srcIdx;
    nUnitsPerStrip = imgPF[srcIdx]->cmP->sdP->ss.nUnitsPerStrip;
    nStripsPerRow = imgPF[srcIdx]->cmP->w / nUnitsPerStrip;
    smElemP    = (StripmapElem*) imgPF[srcIdx]->cmP->sdP->sm.infP->inflatedDataP;
    // For each row of this sample's atlas rectangle...
    for (int j = 0, h = atlasP->btP[i].rect.h; j < h; ++j) {
      smElemEndP = smElemP + nStripsPerRow;
      dstP       = atlasPixelA + atlasP->btP[i].rect.x + (j + atlasP->btP[i].rect.y) * ATLAS_WIDTH;
      // Paste rectangle row
      for (; smElemP < smElemEndP; ++smElemP, dstP += nUnitsPerStrip) {
        memcpy(dstP, 
            imgPF[srcIdx]->cmP->sdP->ss.unpackedDataP + (*smElemP * nUnitsPerStrip), 
            nUnitsPerStrip);
      }
    }
  }
#if 1
  // Texture surface
  textureSurfaceP = SDL_CreateRGBSurfaceWithFormatFrom((void*) atlasPixelA, 
      atlasP->btP[0].remW, atlasP->btP[0].remH, 8, atlasP->btP[0].remW, SDL_PIXELFORMAT_RGBA32);
  if (!textureSurfaceP) {
    printf("Error: %s\n", SDL_GetError());
    return 1;
  }
  // Window and renderer 
  Gui *guiP = NULL;
  guiNew(&guiP);
  // Texture
  Texture_ *textureP =  textureNew(guiP->rendererP, textureSurfaceP);
  // Render it
  clearScreen(guiP->rendererP);
  copy_(guiP->rendererP, textureP, NULL, NULL);
  // Show it
  present_(guiP->rendererP);
  SDL_Delay(2000);
#endif

  // Deflate colormap inflatables
#if MULTITHREADED
  multithread_(stripClr,   (void*) sdPA);
#else
  for (int i = 0; !e && i < N_SAMPLES; ++i) {
    stripClr(cmPF[i]->sdP);
  }
#endif

  // ====================================================
  // Clean up
  // ====================================================
  atlasDel(&atlasP);
  arrayDel((void**) &sdPA);
  frayDel((void**) &imgPF);
  SDL_FreeSurface(textureSurfaceP);
  arrayDel((void**) &atlasPixelA);
  SDL_DestroyTexture(textureP);
  guiDel(&guiP);
  return 0;
}
