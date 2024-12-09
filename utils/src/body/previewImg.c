#include "previewImg.h"

// TODO fix this at some future date. I don't have the patience for this right now. 

void previewImg(Colormap *cmP, ColorPalette *cpP, U32 preview_ms) {
  assert (cmP && cmP->sdP && cmP->sdP->assembledDataA && cpP && preview_ms != 0);

  Gui* guiP= guiNew();
  Surface_ *surfaceP = surfaceNew(cmP->sdP->assembledDataA, cmP->w, cmP->h);
  Texture_ *textureP = textureNew(guiP->rendererP, surfaceP);
  surfaceDel(&surfaceP);
  Rect_ dstRect = {0, 0, cmP->w * 8, cmP->h * 8};
  clearScreen(guiP->rendererP);
  copy_(guiP->rendererP, textureP, NULL, &dstRect);
  present_(guiP->rendererP);
  // SDL_Delay(preview_ms);  // TODO abstract this away
  //textureDel(&textureP);
  guiDel(&guiP);  // destroying the renderer deletes the texture as well.
}
