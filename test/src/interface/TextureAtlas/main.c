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
  Colormap **cmPF = NULL;

  Colormap *cmPA[] = {
    &blehColormap,
    &redColormap,
    &heckColormap
  };

  ColorPalette *cpPA[] = {
    &blehColorPalette,
    &redColorPalette,
    &heckColorPalette
  };

  Error e = frayNew((void**) &cmPF, sizeof(Colormap*), 3);
  for (U32 i = 0; !e && i < 3; ++i) {
    e = frayAdd(cmPF, &cmPA[i], NULL);
  }

  U32 N_SAMPLES = *_frayGetFirstEmptyIdxP(cmPF);

  // Offset color palette and colormap indices to match future texture atlas destinations
  U8 atlasOffset = 0;
  for (int i = 0; !e && i < N_SAMPLES; ++i) {
    cpPA[i]->atlasPaletteOffset = cmPF[i]->sdP->ss.offset = atlasOffset;
    atlasOffset += cpPA[i]->nColors;
  }

  // Extract stripdatas into an array so we can multithread process them all.
  StripDataS **sdPA = NULL;
  if (!e) {
    e = arrayNew((void**) &sdPA, sizeof(StripDataS*), N_SAMPLES);
  }

  if (!e) {
    for (U32 i = 0, iEnd = arrayGetNElems(sdPA); i < iEnd; ++i) {
      sdPA[i] = cmPF[i]->sdP;
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
    e = sdInflate(cmPF[i]->sdP);
    if (!e) {
      sdUnpack(cmPF[i]->sdP);
    }
  }
#endif

  // Create an unfinished atlas with colormap rectangles sorted by size in descending order.
  Atlas *atlasP = NULL;
  if (!e) {
    e = atlasNew(&atlasP, cmPF);
  }
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
  if (!e) {
    e = atlasPlanPlacements(atlasP);
  }
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
  U8 *atlasPixelA = NULL;
  Surface_ *textureSurfaceP = NULL;
  if (!e) {
    e = arrayNew((void**) &atlasPixelA, sizeof(U8), atlasP->btP[0].remW * atlasP->btP[0].remH);
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
#if 0
      printf("drawing to {%d, %d, %d, %d}...\n", 
          atlasP->btP[i].rect.x,
          atlasP->btP[i].rect.y,
          atlasP->btP[i].rect.w,
          atlasP->btP[i].rect.h);
#endif
      srcIdx = atlasP->btP[i].srcIdx;
      nUnitsPerStrip = cmPF[srcIdx]->sdP->ss.nUnitsPerStrip;
      nStripsPerRow = cmPF[srcIdx]->w / nUnitsPerStrip;
      smElemP    = (StripmapElem*) cmPF[srcIdx]->sdP->sm.infP->inflatedDataP;
      // For each row of this sample's atlas rectangle...
      for (int j = 0, h = atlasP->btP[i].rect.h; j < h; ++j) {
        smElemEndP = smElemP + nStripsPerRow;
        dstP       = atlasPixelA + atlasP->btP[i].rect.x + (j + atlasP->btP[i].rect.y) * ATLAS_WIDTH;
        // Paste rectangle row
        for (; smElemP < smElemEndP; ++smElemP, dstP += nUnitsPerStrip) {
          memcpy(dstP, 
                 cmPF[srcIdx]->sdP->ss.unpackedDataP + (*smElemP * nUnitsPerStrip), 
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
  Gui *guiP = NULL;
  if (!e) {
    e = guiNew(&guiP);
  }
  // Texture
  Texture_ *textureP = NULL;
  if (!e) {
    e = textureNew(&textureP, guiP->rendererP, textureSurfaceP);
  }
  // Render it
  if (!e) {
    clearScreen(guiP->rendererP);
    copy_(guiP->rendererP, textureP, NULL, NULL);
    // Show it
    present_(guiP->rendererP);
    SDL_Delay(2000);
  }
#endif

  // Deflate colormap inflatables
  if (!e) {
#if MULTITHREADED
    e = multithread_(stripClr,   (void*) sdPA);
#else
    for (int i = 0; !e && i < N_SAMPLES; ++i) {
      stripClr(cmPF[i]->sdP);
    }
#endif
  }

  // ====================================================
  // Clean up
  // ====================================================
  atlasDel(&atlasP);
  arrayDel((void**) &sdPA);
  frayDel((void**) &cmPF);
  SDL_FreeSurface(textureSurfaceP);
  arrayDel((void**) &atlasPixelA);
  SDL_DestroyTexture(textureP);
  guiDel(&guiP);
  return e;
}
