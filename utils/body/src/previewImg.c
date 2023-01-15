#include "previewImg.h"

Error previewImg(Colormap *cmP, ColorPalette *cpP, U32 preview_ms) {
  if (!cmP || !cmP->sdP || !cmP->sdP->unstrippedDataA || !cpP || preview_ms == 0) {
    return E_BAD_ARGS;
  }

  Surface_ *surfaceP = NULL;
  Texture_ *textureP = NULL;
  Renderer_ *rendererP = NULL;
  Window_ *windowP = NULL;

  Error e = guiNew(&windowP, &rendererP);
  if (!e) {
    //e = surfaceNew(&surfaceP, cmP->w, cmP->h, sizeof(ColormapIdx) * 8);
    e = surfaceNew(&surfaceP, cmP);
  }
  if (!e) {
    e = surfaceIni(surfaceP, cmP, cpP);
  }
  if (!e && surfaceP) {
    e = textureNew(&textureP, rendererP, surfaceP);
  }
  if (!e && !textureP) {
    e = E_BAD_ARGS;
  }
  if (!e) {
    e = textureSetAlpha(textureP);
  }
  Rect_ dstRect = {0, 0, cmP->w, cmP->h};
  if (!e) {
    clearScreen(rendererP);
    e = copy_(rendererP, textureP, NULL, &dstRect);
  }
	if (!e) {
		present_(rendererP);
    SDL_Delay(preview_ms);
  }
  if (textureP) {
    textureDel(&textureP);
  }
  if (windowP) {
    SDL_DestroyWindow(windowP);
  }
  SDL_Quit();
  return e;
}
