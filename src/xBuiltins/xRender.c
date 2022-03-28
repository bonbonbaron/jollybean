#include "xRender.h"
#include "x.h"

// Compressed images are already in memory in JB. 
// JB just reconstructs them from strips of staggered pixels. 
// Packed bits are staggered to allow JB to unpack and (if necessary) flip 4 color-mapped pixels at a time.

// =========================================
// Clear color map and all its related data.
// =========================================
void _cmClr(ColormapS *cmP) {
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
Error _cmGen(ColormapS *cmP) {
	Error e;
	register U32 dstFlip;

	if (cmP != NULL) {
		// Check if the image has already been reconstructed. If so, get out.
		if (cmP->dataP != NULL)   // ColormapS has already been reconstructed.
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

//======================================================
// Initialize xRender's system.
//======================================================
Error xRenderIniSys(System *sP, void *sParamsP) {
	unused_(sParamsP);
	if (!sP)
		return E_BAD_ARGS;
	XRender *xRenderSysP = (XRender*) sP;
	return guiNew(&xRenderSysP->windowP, &xRenderSysP->rendererP);
}

//======================================================
// Initialize xRender's components, which are Images.
//======================================================
Error xRenderIniComp(System *sP, void *compDataP) {
	if (!sP || !compDataP)
		return E_BAD_ARGS;

	XRender *xRenderSysP = (XRender*) sP;

	XRenderCompData *cP = (XRenderCompData*) compDataP;
	if (cP->imgP->textureP) 
		return SUCCESS;
	// Build colormap.
	Error e = _cmGen(cP->imgP->colorMapP);
	// Make surface out of colormap and color palette.
	Surface_ *surfaceP = NULL;

	if (!e)
		e = surfaceNew(&surfaceP, cP);

	// Apply color palette to color map.
	if (!e)
		e = surfaceIni(surfaceP, cP);

	// Create texture from surface. 
	if (!e && surfaceP)
		e = textureNew(&cP->imgP->textureP, xRenderSysP->rendererP, surfaceP);

	if (!e && !cP->imgP->textureP) 
		e = E_BAD_ARGS;

	if (!e) 
		e = textureSetAlpha(cP->imgP->textureP);

	if (!e)
		cP->srcRectP = NULL;

	//SDL_FreeSurface(surfaceP);  // Program crashes when I do this. Maybe textureP needs it?

	// Clean up if anything bad happened.
	if (e) {
		_cmClr(cP->imgP->colorMapP);
		if (cP->imgP->textureP) 
			textureDel(&cP->imgP->textureP);
	}
	return e;
}

Error xRenderProcessMessage(System *sP, Message *msgP) {
	unused_(sP);
	unused_(msgP);
	return SUCCESS;
}

// None of the render system's specials should be cleared as the main system owns them.
XClrFuncDef_(Render) {
  unused_(sP);
  return SUCCESS;
}

#define RECT (4)  /* TODO: move to enum */
XGetShareFuncDef_(Render) {
  Error e = SUCCESS;
  Map *rectMP = (Map*) mapGet(shareMMP, RECT);
  if (!rectMP)
    return E_BAD_KEY;
  Focus *fP = sP->focusA;
  Focus *fEndP = fP + arrayGetNElems(sP->focusA);
  for (; !e && fP < fEndP; fP++) {
    XRenderComp *cP = fP->compA;
    XRenderComp *cEndP = cP + arrayGetNElems(fP->compA);
    for (; !e && cP < cEndP; cP++) {
      cP->data.srcRectP = (Rect_*) mapGet(rectMP, cP->xHeader.owner);
      if (!cP->data.srcRectP)
        e = E_BAD_ARGS;
    }
  }
  return e;
}

//======================================================
// Render activity
//======================================================
Error render(Focus *fP) {
	Error e = SUCCESS;
	XRenderComp *cP, *cEndP;
	XRender *xRenderSysP = (XRender*) fP->ownerP;
	Renderer_ *rendererP = xRenderSysP->rendererP;

	clearScreen(xRenderSysP->rendererP);

	cP = (XRenderComp*) fP->compA;
	cEndP = cP + fP->firstInactiveIdx;

	for (; !e && cP < cEndP; cP++) 
		e = copy_(rendererP, cP->data.imgP->textureP, NULL, cP->data.dstRectP);

	// Tell GPU to execute instructions.
	if (!e)
		present_(rendererP);

	return e;
}

//======================================================
// System definition
//======================================================
X_(Render, 1, Focus_(1, render));
