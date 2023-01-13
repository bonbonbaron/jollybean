#include "interface.h"

Error guiNew(Window_ **windowPP, Renderer_ **rendererPP) {
  //TODO add an #if that only uses this code when interfacing SDL
	if (!windowPP || !rendererPP) 
		return E_BAD_ARGS;
	// Init SDL
	if (SDL_Init(SDL_INIT_VIDEO) != SUCCESS)
		return EXIT_FAILURE;
	// Init window
	//*windowPP = SDL_CreateWindow("Hello world!", 0, 0, 0, 0, SDL_WINDOW_FULLSCREEN);
	*windowPP = SDL_CreateWindow("Hello world!", 100, 100, 400, 300, 0);
	if (!*windowPP)
		return EXIT_FAILURE;
	// Init renderer
	*rendererPP = SDL_CreateRenderer(*windowPP, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!*rendererPP) {
    printf("%s\n", SDL_GetError());
		SDL_DestroyWindow(*windowPP);
		SDL_Quit();
		return EXIT_FAILURE;
	}
	return SUCCESS;
}


Error surfaceNew(Surface_ **surfacePP, Colormap *cmP) {
	if (!surfacePP)
    return E_BAD_ARGS;
  SDL_ClearError();
#if 0
  switch (bpp) {
    case 1:
      *surfacePP = SDL_CreateRGBSurfaceWithFormat(0, w, h, bpp, SDL_PIXELFORMAT_RGBA8888);
      break;
    case 2:
      *surfacePP = SDL_CreateRGBSurfaceWithFormat(0, w, h, bpp, SDL_PIXELFORMAT_RGBA8888);
      break;
    case 4:
      *surfacePP = SDL_CreateRGBSurfaceWithFormat(0, w, h, bpp, SDL_PIXELFORMAT_RGBA8888);
      break;
    default:
      return E_UNSUPPORTED_PIXEL_FORMAT;
  }
#else
  //*surfacePP = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA8888);
  //*surfacePP = SDL_CreateRGBSurfaceWithFormat(0, cmP->w, cmP->h, 8, SDL_PIXELFORMAT_INDEX8);
  *surfacePP = SDL_CreateRGBSurface(0, cmP->w, cmP->h, 8, 0, 0, 0, 0);
  if (*surfacePP) {
    printf("succefully made surface with %d colors\n", (*surfacePP)->format->palette->ncolors);
  }
  else {
    printf("\n\nSDL error: %s\n\n\n", SDL_GetError());
		return E_UNSUPPORTED_PIXEL_FORMAT;
  }
#endif

	return SUCCESS;
}

Error surfaceIni(Surface_ *surfaceP, Colormap *cmP, ColorPalette *cpP) {
  //Palette_ palette = {nColors, colorA, 0, 0};
  printf("starting to make palette\n");
#if 0
  Palette_ *palP = NULL;
  Error e = jbAlloc((void**) &palP, sizeof(Palette_), 1);
  if (!e) {
    palP->ncolors = cpP->nColors;
    printf("copying colors into palette\n");
    memcpy(palP->colors, cpP->colorA, arrayGetElemSz(cpP->colorA) * cpP->nColors);
    //surfaceP->pixels = cmDataP;
    printf("copying pixels into surface pixels");
    memcpy(surfaceP->pixels, cmP->dataP, arrayGetElemSz(cmP->dataP) * arrayGetNElems(cmP->dataP));
    SDL_SetSurfacePalette(surfaceP, palP);
  }
  if (e) {
    jbFree((void**) &palP);
  }
	return e;
#else
  //TODO add an #if that only uses this code when interfacing SDL
  //
  //SDL_SetColorKey(surfaceP, SDL_TRUE, 0);
  memset(surfaceP->format->palette->colors, 0, sizeof(Color_) * surfaceP->format->palette->ncolors);
  memcpy(surfaceP->format->palette->colors, cpP->colorA, arrayGetElemSz(cpP->colorA) * cpP->nColors);
  //surfaceP->pixels = cmDataP;
  printf("copying pixels into surface pixels");
  memcpy(surfaceP->pixels, cmP->dataP, arrayGetElemSz(cmP->dataP) * arrayGetNElems(cmP->dataP));
  return SUCCESS;
#endif
}

Error textureNew(Texture_ **texturePP, Renderer_ *rendererP, Surface_ *surfaceP) {
	if (!texturePP || !rendererP || !surfaceP)
		return E_BAD_ARGS;

  //TODO add an #if that only uses this code when interfacing SDL
	*texturePP = SDL_CreateTextureFromSurface(rendererP, surfaceP);
	if (!*texturePP)
		return E_UNSUPPORTED_PIXEL_FORMAT;

	return SUCCESS;
}

void textureDel(Texture_ **texturePP) {
	SDL_DestroyTexture(*texturePP);
	*texturePP = NULL;
}

Error textureSetAlpha(Texture_ *textureP) {
	if (!textureP)
		return E_BAD_ARGS;
	return SDL_SetTextureBlendMode(textureP, SDL_BLENDMODE_BLEND);
}

void clearScreen(Renderer_ *rendererP) {
	SDL_SetRenderDrawColor(rendererP, 0xff, 0xff, 0xff, 0xff);
	SDL_RenderClear(rendererP);
}
