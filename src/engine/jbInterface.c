#include "jbInterface.h"
#include "SDL.h"

Error guiNew(Window **windowPP, Renderer **rendererPP) {
	if (!rendererP && !*windowPP) {
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
			SDL_DestroyWindow(*windowPP);
			SDL_Quit();
			printf("SDL init error: %s\n", SDL_GetError());
			return EXIT_FAILURE;
		}
	}
	return SUCCESS;
}


Error surfaceNew(Surface **surfacePP, Colormap *cP) {
	if (!surfacePP || !cP)
	*surfacePP = SDL_CreateRGBSurfaceWithFormat(0, cP->imgP->colorMapP->w, cP->imgP->colorMapP->h, cP->imgP->colorMapP->bpp, SDL_PIXELFORMAT_INDEX8);
	if (!*surfacePP)
		return E_UNSUPPORTED_PIXEL_FORMAT;
	return SUCCESS;
}

Error iniSurfacePixels(Surface *surfaceP, Colormap *cP) {
	Palette palette = {cP->imgP->nColors, (Color*) cP->imgP->colorA, 0, 0};
	surfaceP->pixels = cP->imgP->colorMapP->dataP;
	return SDL_SetSurfacePalette(surfaceP, &palette);
}

Error textureNew(Texture **texturePP, Renderer *rendererP, Surface *surfaceP) {
	if (!texturePP || !rendererP || !surfaceP)
		return E_BAD_ARGS;

	*texturePP = SDL_CreateTextureFromSurface(rendererP, surfaceP);
	if (!*texturePP)
		return E_UNSUPPORTED_PIXEL_FORMAT;

	return SUCCESS;
}

void textureDel(Texture **texturePP) {
	SDL_DestroyTexture(*texturePP);
	*texturePP = NULL;
}

:rror setTextureAlpha(Texture *textureP) {
	if (!textureP)
		return E_BAD_ARGS;
	return SDL_SetTextureBlendMode(textureP, SDL_BLENDMODE_BLEND);
}

void clearScreen(Renderer *rendererP) {
	SDL_SetRenderDrawColor(rendererP, 0, 0, 0, 0xff);
	SDL_RenderClear(rendererP);
}

Error imageSet(Renderer *rendererP, XRenderComp *cP) {
	return SDL_RenderCopy(rendererP, cP->imgP->textureP, NULL, *cP->dstRectPP);
}

typedef SDL_RenderPresent present;
