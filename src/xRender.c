#include "xRender.h"

// Compressed images are already in memory in JB. 
// JB just reconstructs them from strips of staggered pixels. 
// Packed bits are staggered to allow JB to unpack 4 color-mapped pixels' bits at a time.

void cmClr(Colormap *cmP) {
	if (cmP != NULL) {
	 	if (cmP->dataP != NULL)
			jbFree((void**) &cmP->dataP);
		if (cmP->stripSetP != NULL && cmP->stripSetP->stripSetInfP != NULL &&
				cmP->stripSetP->stripSetInfP->inflatedDataP != NULL)
			jbFree((void**) &cmP->stripSetP->stripSetInfP->inflatedDataP);
		if (cmP->stripMapP != NULL && cmP->stripMapP->stripMapInfP != NULL &&
				cmP->stripMapP->stripMapInfP->inflatedDataP != NULL)
			jbFree((void**) &cmP->stripSetP->stripSetInfP->inflatedDataP);
	}
}

Error cmGen(Colormap *cmP) {
	Error e;
	register U32 dstFlip;
	register U32 srcStrip;

	if (cmP != NULL) {
		// Check if the image has already been reconstructed. If so, get out.
		if (cmP->dataP != NULL)   // Colormap has already been reconstructed.
			return SUCCESS;  
		// If not reconstructed yet, inflate strip set if it's still compressed (inflate() checks internally).
		if (cmP->stripSetP)
			e = inflate(cmP->stripSetP->stripSetInfP);
		else
			return E_NULL_VAR;
		// If CM source is strip-mapped, inflate strip map if it's still compressed (same as above).
		if (!e && cmP->stripMapP)
			e = inflate(cmP->stripMapP->stripMapInfP);
		// Allocate colormap memory.
		if (!e)
			e = jbAlloc((void**) &cmP->dataP, sizeof(U8), cmP->w * cmP->h);
	}
	else
		e = E_BAD_ARGS;

	// Expand strip set into image.
	if (!e) {
		const U32 *cmDataP = (U32*) cmP->dataP;
		U32 *dstStripP = (U32*) cmDataP;
		U32 *stripSetP = (U32*) cmP->stripSetP->stripSetInfP->inflatedDataP;
		switch (cmP->bpp) {
			//TODO: case 1:
			//TODO: case 2:
			case 4: 
				// Mapped stripsets need to be both unpacked and indexed. They may need strips to be flipped too.
				if (cmP->stripMapP) {
					U16 *mapEndP = ((U16*) cmP->stripMapP->stripMapInfP->inflatedDataP) + cmP->stripMapP->nIndices;
					// 8 *packed* pixels per U32 out of 64 pixels means there are 8 U32s.
					// First read all mapped strips into the target colormap.
					for (U16 *ssIdxP = (U16*) cmP->stripMapP->stripMapInfP->inflatedDataP; ssIdxP < mapEndP; ssIdxP++) {
						U32 *srcStripP = stripSetP + (*ssIdxP << 3);
						// Pixels 01 - 08
						*dstStripP++ =  *srcStripP   & 0x0F0F0F0F;
						*dstStripP++ = (*srcStripP++ & 0xF0F0F0F0) >> 4;
						// Pixels 09 - 16
						*dstStripP++ =  *srcStripP   & 0x0F0F0F0F;
						*dstStripP++ = (*srcStripP++ & 0xF0F0F0F0) >> 4;
						// Pixels 17 - 24
						*dstStripP++ =  *srcStripP   & 0x0F0F0F0F;
						*dstStripP++ = (*srcStripP++ & 0xF0F0F0F0) >> 4;
						// Pixels 25 - 32
						*dstStripP++ =  *srcStripP   & 0x0F0F0F0F;
						*dstStripP++ = (*srcStripP++ & 0xF0F0F0F0) >> 4;
						// Pixels 33   40
						*dstStripP++ =  *srcStripP   & 0x0F0F0F0F;
						*dstStripP++ = (*srcStripP++ & 0xF0F0F0F0) >> 4;
						// Pixels 41   48
						*dstStripP++ =  *srcStripP   & 0x0F0F0F0F;
						*dstStripP++ = (*srcStripP++ & 0xF0F0F0F0) >> 4;
						// Pixels 49   56
						*dstStripP++ =  *srcStripP   & 0x0F0F0F0F;
						*dstStripP++ = (*srcStripP++ & 0xF0F0F0F0) >> 4;
						// Pixels 57   64
						*dstStripP++ =  *srcStripP   & 0x0F0F0F0F;
						*dstStripP++ = (*srcStripP++ & 0xF0F0F0F0) >> 4;
					}
					// Then flip whatever strips need flipping. Remember data's already expanded to U8s!
					if (cmP->stripSetP->flipSetP) {
						U16 *flipEndP = cmP->stripSetP->flipSetP->flipIdxA + cmP->stripSetP->flipSetP->nFlips;
						U32 tmp;
						U32 *dstLeft4PixelsP, *dstRight4PixelsP;
						// 4 *unpacked* pixels per U32 out of 64 pixels means there are 16 U32s.
						for (U16 *flipIdxP = cmP->stripSetP->flipSetP->flipIdxA; flipIdxP < flipEndP; flipIdxP++) {
							for (U32 nPairsSwapped = 0; nPairsSwapped < 8; nPairsSwapped++) {
								// Left 4 pixels
								dstLeft4PixelsP = (U32*) cmDataP + (*flipIdxP << 3) + nPairsSwapped; 
								dstFlip = *dstLeft4PixelsP;
								dstFlip   = ((dstFlip & 0xFFFF0000) >> 16) | ((dstFlip & 0x0000FFFF) << 16);
								tmp = ((dstFlip & 0xFF00FF00) >>  8) | ((dstFlip & 0x00FF00FF) <<  8);
								// Right 4 pixels
								dstRight4PixelsP = dstLeft4PixelsP + 15 - (nPairsSwapped >> 1);
								dstFlip = *dstRight4PixelsP;
								*dstRight4PixelsP = tmp;  // Put left 4 pixels into right 4 pixels
								dstFlip   = ((dstFlip & 0xFFFF0000) >> 16) | ((dstFlip & 0x0000FFFF) << 16);
								*dstLeft4PixelsP = ((dstFlip & 0xFF00FF00) >>  8) | ((dstFlip & 0x00FF00FF) <<  8);  // Put right 4 pixels into left 4 pixels
							}
						}
					}
				} 
				// Unmapped stripsets only need to be unpacked.
				else {
					U32 *srcEndP = stripSetP + (cmP->stripSetP->nStrips << 3);
					for (U32 *srcStripP = stripSetP; srcStripP < srcEndP; srcStripP++) {
						// Pixels 01 - 08
						*dstStripP++ =  *srcStripP   & 0x0F0F0F0F;
						*dstStripP++ = (*srcStripP++ & 0xF0F0F0F0) >> 4;
						// Pixels 09 - 16
						*dstStripP++ =  *srcStripP   & 0x0F0F0F0F;
						*dstStripP++ = (*srcStripP++ & 0xF0F0F0F0) >> 4;
						// Pixels 17 - 24
						*dstStripP++ =  *srcStripP   & 0x0F0F0F0F;
						*dstStripP++ = (*srcStripP++ & 0xF0F0F0F0) >> 4;
						// Pixels 25 - 32
						*dstStripP++ =  *srcStripP   & 0x0F0F0F0F;
						*dstStripP++ = (*srcStripP++ & 0xF0F0F0F0) >> 4;
						// Pixels 33   40
						*dstStripP++ =  *srcStripP   & 0x0F0F0F0F;
						*dstStripP++ = (*srcStripP++ & 0xF0F0F0F0) >> 4;
						// Pixels 41   48
						*dstStripP++ =  *srcStripP   & 0x0F0F0F0F;
						*dstStripP++ = (*srcStripP++ & 0xF0F0F0F0) >> 4;
						// Pixels 49   56
						*dstStripP++ =  *srcStripP   & 0x0F0F0F0F;
						*dstStripP++ = (*srcStripP++ & 0xF0F0F0F0) >> 4;
						// Pixels 57   64
						*dstStripP++ =  *srcStripP   & 0x0F0F0F0F;
						*dstStripP++ = (*srcStripP++ & 0xF0F0F0F0) >> 4;
					}
				}
				break;

			default:  
				e = E_UNSUPPORTED_PIXEL_FORMAT;
				break;
		}
	}
	else 
		cmClr(cmP);

	return e;
}

/* We need these types of initializations:
 * iniC: inflate (if necessary), initialize (" "), 
 * iniS: prepare the system to handle all its components.
 */
static SDL_Window *windowP;
static SDL_Surface *wSurfP;
static SDL_Rect wRect = {0};
static SDL_Renderer *rendererP;

Error xRenderIniS() {
	// Init SDL
	if (SDL_Init(SDL_INIT_VIDEO) != SUCCESS)
		return EXIT_FAILURE;

	// Init window
	windowP = SDL_CreateWindow("Hello world!", 100, 100, 1080, 700, SDL_WINDOW_SHOWN);
	if (!windowP)
		return EXIT_FAILURE;
	wSurfP = SDL_GetWindowSurface(windowP);
	wRect.w =  = {0, 0, 620, 387};

	// Init renderer
	rendererP = SDL_CreateRenderer(windowP, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!rendererP) {
		SDL_DestroyWindow(windowP);
		SDL_Quit();
		return EXIT_FAILURE;
	}
	return SUCCESS;
}

// Decouple SDL from JB as much as possible! 
// I need a plan:
// 0) design XRenderC... and how do seeds interact with entities?
// 1) initialize a system's special attributes along with its standard ECS attributes (repeatable)
// 2) initialize one component
// 3) initialize an array of components
// 4) 
//
Error xRenderIniC() {
	Error e = cmGen(&biggerCM);
	if (!e)
		SDL_Surface *surfaceP = SDL_CreateRGBSurfaceWithFormat(0, biggerCM.w, biggerCM.h, biggerCM.bpp, SDL_PIXELFORMAT_INDEX8);

	if (!surfaceP) 
		e = E_BAD_ARGS;
	else {
		SDL_Palette palette1 = {16, colorSet1, 0, 0};
		surfaceP->pixels = biggerCM.dataP;
		e = SDL_SetSurfacePalette(surfaceP, &palette1);
	}

	SDL_Texture *tP = SDL_CreateTextureFromSurface(rendererP, surfaceP);

	if (!tP) 
		e = E_BAD_ARGS;
	if (!e) 
		e = SDL_SetTextureBlendMode(tP, SDL_BLENDMODE_BLEND);

	if (!e) {
		SDL_Rect srcRect = {0, 0, 128, 256};
		SDL_Rect dstRect = {100, 100, 64, 256};
	}

	cmClr(&biggerCM);
}

Error xRenderExe() {
	Error e = SUCCESS;

	for (U32 i = 0; i < 500; i++) {
		SDL_SetRenderDrawColor(rendererP, 0, 0, 0, 0xff);
		SDL_RenderClear(rendererP);
		e = SDL_RenderCopy(rendererP, tP, &srcRect, &dstRect);
		dstRect.x++;
		SDL_RenderPresent(rendererP);
	}

	SDL_DestroyRenderer(rendererP);
	SDL_DestroyWindow(windowP);
	SDL_Quit();

	return e;
}
