#include "previewImg.h"

Error previewImg(Colormap *cmP, ColorPalette *cpP, U32 preview_ms) {
  if (!cmP || !cmP->sdP || !cmP->sdP->assembledDataA || !cpP || preview_ms == 0) {
    return E_BAD_ARGS;
  }

  Surface_ *surfaceP = NULL;
  Texture_ *textureP = NULL;
  Gui      *guiP     = NULL;

  Error e = guiNew(&guiP);
  if (!e) {
    e = surfaceNew(&surfaceP, cmP->sdP->assembledDataA, cmP->w, cmP->h);
  }
  if (!e) {
    appendAtlasPalette(surfaceP, cpP);
  }
  if (!e && surfaceP) {
    e = textureNew(&textureP, guiP->rendererP, surfaceP);
  }
  surfaceDel(&surfaceP);
  cmClr(cmP);
  if (!e && !textureP) {
    e = E_BAD_ARGS;
  }
  if (!e) {
    e = textureSetAlpha(textureP);
  }
  Rect_ dstRect = {0, 0, cmP->w * 8, cmP->h * 8};
  if (!e) {
    clearScreen(guiP->rendererP);
    e = copy_(guiP->rendererP, textureP, NULL, &dstRect);
  }
	if (!e) {
		present_(guiP->rendererP);
    SDL_Delay(preview_ms);
  }
  //textureDel(&textureP);
  guiDel(&guiP);  // destroying the renderer deletes the texture as well.
  return e;
}
