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
    Image* imgP = &imgA[i];
    frayAdd(imgPF, &imgP, NULL);
  }

  // Extract stripdatas into an array so we can multithread process them all.
  StripDataS **sdPA = arrayNew(sizeof(StripDataS*), N_SAMPLES);

  for (U32 i = 0; i < N_SAMPLES; ++i) {
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
  // Texture atlas
  Atlas *atlasP =  atlasNew(imgPF);
  atlasPlanPlacements(atlasP);
  // GUI
  Gui *guiP =  guiNew();
  assert(guiP);
  // Texture
  Color_* atlasPixelA = assembleTextureAtlas(imgPF, atlasP);
  // Let colormaps track where their rectangles are sorted.
  updateCmSrcRectIndices(imgPF, atlasP);
  // Texture surface
  Surface_* atlasSurfaceP = surfaceNew((void*) atlasPixelA, atlasP->btP[0].remW, atlasP->btP[0].remH);
  // Texture
  Texture_ *textureP =  textureNew(guiP->rendererP, atlasSurfaceP);
  /* "Pixel data is not managed automatically with SDL_CreateRGBSurfaceWithFormatFrom().
     You must free the surface before you free the pixel data." */
  surfaceDel(&atlasSurfaceP);
  arrayDel((void**) &atlasPixelA);

  // Render it
  clearScreen(guiP->rendererP);
  copy_(guiP->rendererP, textureP, NULL, NULL);
  // Show it
  present_(guiP->rendererP);
  SDL_Delay(2000);

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
  SDL_FreeSurface(atlasSurfaceP);
  arrayDel((void**) &atlasPixelA);
  SDL_DestroyTexture(textureP);
  guiDel(&guiP);
  return 0;
}
