#include "xRender.h"
#include "xMaster.h"
#include "jb.h"

//TODO Make bookmarks to indicate where images should when they switch layers.
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
  return SUCCESS;
	unused_(sParamsP);
  unused_(sP);
}

//======================================================
// Initialize xRender's components, which are Images.
//======================================================
Error xRenderIniComp(System *sP, void *compDataP, void *compDataSrcP) {
	if (!sP || !compDataP || !compDataSrcP)
		return E_BAD_ARGS;

	XRender *xRenderSysP = (XRender*) sP;
	XRenderComp *cP = (XRenderComp*) compDataP;
  XRenderCompSrc *imgP = (XRenderCompSrc*) compDataSrcP;

  // Skip things that're already done.
	if (cP->textureP) 
		return SUCCESS;
  else if (imgP->textureP) {
    cP->textureP = imgP->textureP;
    return SUCCESS;
  }
	// Build colormap.
	Error e = _cmGen(imgP->colorMapP);
	// Make surface out of colormap and color palette.
	Surface_ *surfaceP = NULL;

	if (!e)
		e = surfaceNew(&surfaceP, imgP->colorMapP->w, imgP->colorMapP->h, imgP->colorMapP->bpp);

	// Apply color palette to color map.
	if (!e)
		e = surfaceIni(surfaceP, imgP->nColors, imgP->colorA, imgP->colorMapP->dataP);

	// Create texture from surface. 
	if (!e && surfaceP)
		e = textureNew(&imgP->textureP, xRenderSysP->rendererP, surfaceP);

	if (!e && !imgP->textureP) 
		e = E_BAD_ARGS;

	if (!e) 
		e = textureSetAlpha(imgP->textureP);

  // Everything's ready to hand over to the component.
	if (!e) {
		cP->srcRectP = NULL;
    cP->dstRectP->x = 0;
    cP->dstRectP->y = 0;
    cP->dstRectP->w = imgP->colorMapP->w;
    cP->dstRectP->h = imgP->colorMapP->h;
    cP->textureP = imgP->textureP;
  }

	//SDL_FreeSurface(surfaceP);  // Program crashes when I do this. Maybe textureP needs it?

	// Clean up if anything bad happened.
	if (e) {
		_cmClr(imgP->colorMapP);
		if (imgP->textureP) 
			textureDel(&imgP->textureP);
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

#define CLIP_RECT (4)  /* TODO: move to enum */
#define POSSIZE_RECT (5)  /* TODO: move to enum */
XGetShareFuncDef_(Render) {
  XRender *renderSysP = (XRender*) sP;
  Error e = SUCCESS;
#if 0
  Map *srcRectMP = (Map*) mapGet(shareMMP, CLIP_RECT);
  Map *dstRectMP = (Map*) mapGet(shareMMP, POSSIZE_RECT);
  Entity entity = 0;
  // Get clip and position-size rectangles for each render component.
  if (!srcRectMP || !dstRectMP)
    return E_BAD_KEY;
  XRenderComp *cP = sP->cF;
  XRenderComp *cEndP = cP + arrayGetNElems(sP->cF);
  for (; !e && cP < cEndP; cP++) {
    cP->srcRectP = (Rect_*) mapGet(srcRectMP, entity);
    if (!cP->srcRectP)
      e = E_BAD_ARGS;
    if (!e) {
      cP->dstRectP = (Rect_*) mapGet(dstRectMP, sP->cIdx2eA[cP - (XRenderComp*) sP->cF]);
      if (!cP->dstRectP) 
        e = E_BAD_ARGS;
    }
  }
#endif
  // Get window and renderer too! Kind important lol. 
  // They're bogus because there's only one element per window/renderer map. 
  // But we gotta stick with our awesome design!!
  // Get window
  Map *windowMP = NULL; 
  Map *rendererMP = NULL;
  if (!e)
    windowMP = (Map*) mapGet(shareMMP, WINDOW_GENE_TYPE);
  if (!windowMP)
    e = E_BAD_KEY;  
  else 
    renderSysP->windowP = (Window_*) mapGet(windowMP, WINDOW_KEY_);
  // Get renderer
  if (!e)
    rendererMP = (Map*) mapGet(shareMMP, RENDERER_GENE_TYPE);
  if (!rendererMP)
    e = E_BAD_KEY;
  else 
    renderSysP->rendererP = (Renderer_*) mapGet(rendererMP, RENDERER_KEY_);

  if (!renderSysP->windowP || !renderSysP->rendererP)
    e = E_BAD_KEY;

  return e;
}

//======================================================
// Render activity
//======================================================
Error xRenderRun(System *sP) {
	Error e = SUCCESS;

	XRenderComp *cP = (XRenderComp*) sP->cF;
	XRenderComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);
	Renderer_ *rendererP = ((XRender*) sP)->rendererP;

	clearScreen(rendererP);
	for (; !e && cP < cEndP; cP++) 
		e = copy_(rendererP, cP->textureP, cP->srcRectP, cP->dstRectP);
	if (!e)
		present_(rendererP);

	return e;
}

//======================================================
// System definition
//======================================================
X_(Render, RENDER, FLG_NO_SWITCHES_);
