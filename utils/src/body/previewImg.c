#include "previewImg.h"

// TODO fix this at some future date. I don't have the patience for this right now. 

void previewImg(Colormap *cmP, ColorPalette *cpP, U32 preview_ms) {
  assert (cmP && cmP->sdP && cmP->sdP->assembledDataA && cpP && preview_ms != 0);

  Gui* guiP= guiNew();
  // Colorize image first since surfaceNew() expects RGBA32 now instead of COLOR_IDX8.
  U8* cmElemP = cmP->sdP->assembledDataA;
  const U32 N_PIXELS = arrayGetNElems( cmP->sdP->assembledDataA );
  const U8* cmElemEndP = cmElemP + N_PIXELS;
  const U8 MAX_COLORPAL_IDX = cpP->nColors - 1;
  Color_* colorizedImg = arrayNew( sizeof(Color_), N_PIXELS );
  Color_* colorPixelP = colorizedImg;
  for ( ; cmElemP < cmElemEndP; ++cmElemP, ++colorPixelP ) {
    assert( *cmElemP <= MAX_COLORPAL_IDX );
    *colorPixelP = cpP->colorA[*cmElemP];
  }
  Surface_ *surfaceP = surfaceNew(colorizedImg, cmP->w, cmP->h);
  Texture_ *textureP = textureNew(guiP->rendererP, surfaceP);
  surfaceDel(&surfaceP);
  Rect_ dstRect = {0, 0, cmP->w, cmP->h};
  clearScreen(guiP->rendererP);
  copy_(guiP->rendererP, textureP, NULL, &dstRect);
  present_(guiP->rendererP);
  SDL_Delay(preview_ms);  
  textureDel(&textureP);   // TODO why was this commented out?
  guiDel(&guiP);  // destroying the renderer deletes the texture as well.
  arrayDel( (void**) &colorizedImg );
}
