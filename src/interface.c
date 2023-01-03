#include "botox.h"

Error guiNew(Window_ **windowPP, Renderer_ **rendererPP) {
	if (!windowPP || !rendererPP) 
		return E_BAD_ARGS;
	// Init SDL
	if (SDL_Init(SDL_INIT_VIDEO) != SUCCESS)
		return EXIT_FAILURE;
	// Init window
	*windowPP = SDL_CreateWindow("Hello world!", 100, 100, 1080, 700, SDL_WINDOW_SHOWN);
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


Error surfaceNew(Surface_ **surfacePP, U32 w, U32 h, U32 bpp) {
	if (!surfacePP)
    return E_BAD_ARGS;
  *surfacePP = SDL_CreateRGBSurfaceWithFormat(0, w, h, bpp, SDL_PIXELFORMAT_INDEX8);
	if (!*surfacePP)
		return E_UNSUPPORTED_PIXEL_FORMAT;
	return SUCCESS;
}

Error surfaceIni(Surface_ *surfaceP, U8 nColors, Color_ *colorA, U8 *cmDataP) {
	Palette_ palette = {nColors, (Color_*) colorA, 0, 0};
	surfaceP->pixels = cmDataP;
	return SDL_SetSurfacePalette(surfaceP, &palette);
}

Error textureNew(Texture_ **texturePP, Renderer_ *rendererP, Surface_ *surfaceP) {
	if (!texturePP || !rendererP || !surfaceP)
		return E_BAD_ARGS;

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
	SDL_SetRenderDrawColor(rendererP, 0, 0, 0, 0xff);
	SDL_RenderClear(rendererP);
}
