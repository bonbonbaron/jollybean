#include "xRender.h"
#include "redColormap.h"
#include "redColorPalette.h"

// Dependencies: xRender.o, x.o, data.o, and interface.o
int main(int argc, char  **argv) {
  Surface_ *surfaceP = NULL;
  Texture_ *textureP = NULL;
  Renderer_ *rendererP = NULL;
  Window_ *windowP = NULL;

  Error e = guiNew(&windowP, &rendererP);

  if (!e) {
    e = cmGen(&redColormap);
  }
  if (!e) {
    e = surfaceNew(&surfaceP, redColormap.w, redColormap.h, redColormap.bpp);
  }
  // TODO are... teh alphas wrong?
  for (int i = 0; i < 5; ++i) {
    redColorPalette.colorA[i].r = 255;
    redColorPalette.colorA[i].a = 255;
    printf("color %d: (%d, %d, %d, %d)\n", i, 
        redColorPalette.colorA[i].r,
        redColorPalette.colorA[i].g,
        redColorPalette.colorA[i].b,
        redColorPalette.colorA[i].a
    );
  }

  if (!e) {
    e = surfaceIni(surfaceP, redColorPalette.nColors, redColorPalette.colorA, redColormap.dataP);
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

  if (!e) {
    clearScreen(rendererP);
    e = copy_(rendererP, textureP, NULL, NULL);
  }
	if (!e) {
		present_(rendererP);
    SDL_Delay(2000);
  }

  

  // Clean up if anything bad happened.
  cmClr(&redColormap);
  if (textureP) {
    textureDel(&textureP);
  }
  return e;
}
