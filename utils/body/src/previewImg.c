#include "previewImg.h"

Error previewImg(Colormap *cmP, ColorPalette *cpP, U32 preview_ms) {
  if (!cmP || !cmP->sdP || !cmP->sdP->assembledDataA || !cpP || preview_ms == 0) {
    return E_BAD_ARGS;
  }

  Surface_ *surfaceP = NULL;
  Texture_ *textureP = NULL;
  Renderer_ *rendererP = NULL;
  Window_ *windowP = NULL;

  Error e = guiNew(&windowP, &rendererP);
  if (!e) {
//Error surfaceNew(Surface_ **surfacePP, void *pixelDataA, U32 w, U32 h) {
    e = surfaceNew(&surfaceP, cmP, cmP->w, cmP->h);
  }
  if (!e) {
    appendAtlasPalette(surfaceP, cpP);
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
  Rect_ dstRect = {0, 0, cmP->w * 8, cmP->h * 8};
  if (!e) {
    clearScreen(rendererP);
    e = copy_(rendererP, textureP, NULL, &dstRect);
    //e = copy_(rendererP, textureP, NULL, NULL);
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
