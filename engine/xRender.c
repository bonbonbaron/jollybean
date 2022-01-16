#include "jb.h"

// Compressed images are already in memory in JB. 
// JB just reconstructs them from strips of staggered pixels. 
// Packed bits are staggered to allow JB to unpack and (if necessary) flip 4 color-mapped pixels at a time.

// =========================================
// Clear color map and all its related data.
// =========================================
void _cmClr(Colormap *cmP) {
	if (cmP != NULL) {
	 	if (cmP->dataP != NULL)    // But if the double pointer is null, avoid any processing.
			jbFree((void**) &cmP->dataP);
		if (cmP->stripSetP != NULL && cmP->stripSetP->stripSetInfP != NULL &&
				cmP->stripSetP->stripSetInfP->inflatedDataP != NULL)
			jbFree((void**) &cmP->stripSetP->stripSetInfP->inflatedDataP);
		if (cmP->stripMapP != NULL && cmP->stripMapP->stripMapInfP != NULL &&
				cmP->stripMapP->stripMapInfP->inflatedDataP != NULL)
			jbFree((void**) &cmP->stripSetP->stripSetInfP->inflatedDataP);
	}
}

// =====================================================================
// Build color map by inflating, unpacking, and piecing together strips.
// =====================================================================
Error _cmGen(Colormap *cmP) {
	Error e;
	register U32 dstFlip;
	printf("cmgen\n");

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
		_cmClr(cmP);

	return e;
}

static SDL_Window *windowP = NULL;
static SDL_Surface *wSurfP = NULL;
static SDL_Renderer *rendererP = NULL;

//======================================================
// Initialize xRender's system.
//======================================================
Error xRenderIniS() {
	if (!rendererP && !wSurfP && !windowP) {
		// Init SDL
		if (SDL_Init(SDL_INIT_VIDEO) != SUCCESS)
			return EXIT_FAILURE;

		// Init window
		windowP = SDL_CreateWindow("Hello world!", 100, 100, 1080, 700, SDL_WINDOW_SHOWN);
		if (!windowP)
			return EXIT_FAILURE;
		wSurfP = SDL_GetWindowSurface(windowP);

		// Init renderer
		rendererP = SDL_CreateRenderer(windowP, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (!rendererP) {
			SDL_DestroyWindow(windowP);
			SDL_Quit();
			return EXIT_FAILURE;
		}
	}
	return SUCCESS;
}

//======================================================
// Initialize xRender's components, which are Images.
//======================================================
// TODO delete this debuggery garbage
static U8 ii = 0;
SDL_Rect rect1 = {0, 0, 100, 200};
SDL_Rect rect2 = {50, 100, 200, 200};
SDL_Rect rect3 = {100, 150, 150, 150};
SDL_Rect *rect1P = &rect1;
SDL_Rect *rect2P = &rect2;
SDL_Rect *rect3P = &rect3;

Error xRenderIniC(XHeader *xhP) {
	if (!xhP)
		return E_BAD_ARGS;

	XRenderC *cP = (XRenderC*) xhP;
	if (ii == 0)
		cP->dstRectPP = &rect1P;
	else if (ii == 1)
		cP->dstRectPP = &rect2P;
	else 
		cP->dstRectPP = &rect3P;
	ii++;
	if (cP->imgP->textureP) 
		return SUCCESS;
	// Build colormap.
	Error e = _cmGen(cP->imgP->colorMapP);
	// Make surface out of colormap and color palette.
	SDL_Surface *surfaceP = NULL;

	if (!e)
		surfaceP = SDL_CreateRGBSurfaceWithFormat(0, cP->imgP->colorMapP->w, cP->imgP->colorMapP->h, cP->imgP->colorMapP->bpp, SDL_PIXELFORMAT_INDEX8);

	// Apply color palette to color map.
	if (!surfaceP) 
		e = E_BAD_ARGS;
	else {
		SDL_Palette palette = {cP->imgP->nColors, (SDL_Color*) cP->imgP->colorA, 0, 0};
		surfaceP->pixels = cP->imgP->colorMapP->dataP;
		e = SDL_SetSurfacePalette(surfaceP, &palette);
	}

	// Create texture from surface. 
	if (!e && surfaceP != NULL)
		cP->imgP->textureP = SDL_CreateTextureFromSurface(rendererP, surfaceP);

	if (!e && !cP->imgP->textureP) 
		e = E_BAD_ARGS;

	if (!e) 
		e = SDL_SetTextureBlendMode(cP->imgP->textureP, SDL_BLENDMODE_BLEND);

	if (!e)
		cP->srcRectPP = NULL;

	//SDL_FreeSurface(surfaceP);  // Program crashes when I do this. Maybe textureP needs it?

	// Clean up if anything bad happened.
	if (e) {
		_cmClr(cP->imgP->colorMapP);
		if (cP->imgP->textureP) {
			SDL_DestroyTexture(cP->imgP->textureP);
			cP->imgP->textureP = NULL;
		}
	}
	return e;
}

//======================================================
// Render activity
//======================================================
Error xRender(Activity *aP) {
	Error e = SUCCESS;
	XRenderC *cP, *cEndP;

	// Clear the screen.
	SDL_SetRenderDrawColor(rendererP, 0, 0, 0, 0xff);
	SDL_RenderClear(rendererP);


	// Send GPU instructions on what to render.
	//arrayIniPtrs((const void*) aP->ecA, (void**) &cP, (void**) &cEndP, (S32) aP->firstInactiveIdx);
	cP = (XRenderC*) aP->cA;
	cEndP = cP + aP->firstInactiveIdx;
	// TODO get rid of these placeholder rectangles.
	SDL_Rect *srcRectP = NULL;

	for (; !e && cP < cEndP; cP++) 
		e = SDL_RenderCopy(rendererP, cP->imgP->textureP, srcRectP, *cP->dstRectPP);
		///e = SDL_RenderCopy(rendererP, cP->imgP->textureP, *cP->srcRectPP, *cP->dstRectPP);
		//TODO: replace the above NULLs with pointers to rectangles from other systems

	// Tell GPU to execute instructions.
	if (!e)
		SDL_RenderPresent(rendererP);

	return e;
}

//======================================================
// System definition
//======================================================
System_(Render, RENDER, Activity_(XRENDER, xRender));
