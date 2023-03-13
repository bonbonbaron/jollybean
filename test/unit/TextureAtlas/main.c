#include "interface.h"
#include "data.h"
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
  ColorPalette *cpPA[] = {
    &blehColorPalette,
    &redColorPalette,
    &heckColorPalette
  };
  // Mock up some genes.
  Colormap *cmPA[] = {
    &blehColormap,
    &redColormap,
    &heckColormap
  };

  U32 N_SAMPLES = sizeof(cmPA) / sizeof(cmPA[0]);

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
  for (int i = 0 ; i < N_SAMPLES; i++) {
    printf("{%d, %d, %d, %d}, {%d, %d}, srcIdx = %d\n", 
        atlasP->btP[i].rect.x,
        atlasP->btP[i].rect.y,
        atlasP->btP[i].rect.w,
        atlasP->btP[i].rect.h,
        atlasP->btP[i].remW,
        atlasP->btP[i].remH,
        atlasP->btP[i].header.srcIdx
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
      printf("drawing to {%d, %d, %d, %d}...\n", 
          atlasP->btP[i].rect.x,
          atlasP->btP[i].rect.y,
          atlasP->btP[i].rect.w,
          atlasP->btP[i].rect.h);
      srcIdx = atlasP->btP[i].header.srcIdx;
      nUnitsPerStrip = cmPA[srcIdx]->sdP->ss.nUnitsPerStrip;
      nStripsPerRow = cmPA[srcIdx]->w / nUnitsPerStrip;
      smElemP    = (StripmapElem*) cmPA[srcIdx]->sdP->sm.infP->inflatedDataP;
      // For each row of this sample's atlas rectangle...
      for (int j = 0, h = atlasP->btP[i].rect.h; j < h; ++j) {
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
