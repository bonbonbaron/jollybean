#include "jb.h"

//TODO Make bookmarks to indicate where images should when they switch layers.
// Compressed images are already in memory in JB. 
// JB just reconstructs them from strips of staggered pixels. 
// Packed bits are staggered to allow JB to unpack and (if necessary) flip 4 color-mapped pixels at a time.

// =========================================
// Clear color map and all its related data.
// =========================================
static void _cmClr(ColormapS *cmP) {
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

static void _unpackStrip1BPP(U32 **srcStripPP, U32 **dstStripPP) {
  U32 *srcStripP = *srcStripPP;
  U32 *dstStripP = *dstStripPP;
#ifdef __ARM_NEON__
  // 7 instructions neon VS 40-58 instructions regular
  asm volatile inline (
  "vmov.u8 q10 #1\n\t"   // q10 = mask
  );
  // 1 x (128 pixels per quad-word)
  for (int i = 0; i < 8; ++i) {
    asm volatile inline (
    "vld1.32 {d0-d1}, [%1]!\n\t"    // q0 (aka d0-d1) = packed indices
    "vand q1, q0, q10\n\t"          // q1 = unpacked indices
    "vst1.32 {d2-d3}, [%0]!\n\t"
    "vshr.u8 q0, #1\n\t"            // shift q0 over 1
    : "+r&" (dstStripP), "+r&" (srcStripP)
    );
  }
#else
  // 32 pixels per iteration, 4 iterations
  for (int i = 0; i < 2; ++i, ++srcStripP) 
    for (register int j = 0; j < 8; ++j)
      *dstStripP++ =  (*srcStripP >> j) & 0x01010101;
#endif
}


static void _flipStrips1BPP(ColormapS *cmP) {
  U16 *flipEndP = cmP->stripSetP->flipSetP->flipIdxA + cmP->stripSetP->flipSetP->nFlips;
  U32 *dstLeft4PixelsP, *dstRight4PixelsP;
  // 4 *unpacked* pixels per U32 out of 64 pixels means there are 16 U32s.
  for (U16 *flipIdxP = cmP->stripSetP->flipSetP->flipIdxA; flipIdxP < flipEndP; ++flipIdxP) {
    dstLeft4PixelsP = (U32*) cmP->dataP + (*flipIdxP << 3); 
    dstRight4PixelsP = dstLeft4PixelsP + 15;
    for (; dstLeft4PixelsP < dstRight4PixelsP; ++dstLeft4PixelsP, --dstRight4PixelsP) {
#ifdef __ARM_NEON__
      asm volatile inline (
      "vldr.64 d0, [%0]\n\t"
      "vldr.64 d1, [%1]\n\t"
      "vrev.8 q0\n\t"
      "vst1.32, d0[0], [%1]\n\t"   // Neon-based swap occurs here if I can make this work right.
      "vst1.32, d1[1], [%0]"
      : "+r" (*dstLeft4PixelsP), "+r" (*dstRight4PixelsP)
      );
#else
      // Left 4 pixels
      *dstLeft4PixelsP  = ((*dstLeft4PixelsP & 0xFFFF0000) >> 16) | ((*dstLeft4PixelsP & 0x0000FFFF) << 16);
      *dstLeft4PixelsP  = ((*dstLeft4PixelsP & 0xFF00FF00) >>  8) | ((*dstLeft4PixelsP & 0x00FF00FF) <<  8);
      // Right 4 pixels
      *dstRight4PixelsP = ((*dstRight4PixelsP & 0xFFFF0000) >> 16) | ((*dstRight4PixelsP & 0x0000FFFF) << 16);
      *dstRight4PixelsP = ((*dstRight4PixelsP & 0xFF00FF00) >>  8) | ((*dstRight4PixelsP & 0x00FF00FF) <<  8);  // Put right 4 pixels into left 4 pixels
      swap_(*dstLeft4PixelsP, *dstRight4PixelsP);
#endif
    }
  }
}

static void _unpackStrip2BPP(U32 **srcStripPP, U32 **dstStripPP) {
  U32 *srcStripP = *srcStripPP;
  U32 *dstStripP = *dstStripPP;
#ifdef __ARM_NEON__
  // 7 instructions neon VS 40-58 instructions regular
  asm volatile inline (
  "vmov.u8 q10 #3\n\t"   // q10 = mask
  );
  // 1 x (64 pixels per quad-word)
  for (int i = 0; i < 1; ++i) {  // keeping this useless loop here for when I chagne to 128-pixel strips.
    for (int j = 0; j < 4; ++j) {  // looping through the zipped half-nibbles
      asm volatile inline (
      "vld1.32 {d0-d1}, [%1]!\n\t"    // q0 (aka d0-d1) = packed indices
      "vand q1, q0, q10\n\t"          // q1 = unpacked indices
      "vst1.32 {d2-d3}, [%0]!\n\t"
      "vshr.u8 q0, #2\n\t"            // shift q0 over 1
      : "+r&" (dstStripP), "+r&" (srcStripP)
      );
    }
  }
#else
  // 32 pixels per outer loop iteration, 2 iterations
  for (int i = 0; i < 2; ++i, ++srcStripP) 
    for (register int j = 0; j < 8; j += 2)
      *dstStripP++ =  (*srcStripP >> j) & 0x03030303;
#endif
}

static void _flipStrips2BPP(ColormapS *cmP) {
  U16 *flipEndP = cmP->stripSetP->flipSetP->flipIdxA + cmP->stripSetP->flipSetP->nFlips;
  U32 *dstLeft4PixelsP, *dstRight4PixelsP;
  // 4 *unpacked* pixels per U32 out of 64 pixels means there are 16 U32s.
  for (U16 *flipIdxP = cmP->stripSetP->flipSetP->flipIdxA; flipIdxP < flipEndP; ++flipIdxP) {
    dstLeft4PixelsP = (U32*) cmP->dataP + (*flipIdxP << 3); 
    dstRight4PixelsP = dstLeft4PixelsP + 15;
    for (; dstLeft4PixelsP < dstRight4PixelsP; ++dstLeft4PixelsP, --dstRight4PixelsP) {
#ifdef __ARM_NEON__
      asm volatile inline (
      "vldr.64 d0, [%0]\n\t"
      "vldr.64 d1, [%1]\n\t"
      "vrev.8 q0\n\t"
      "vst1.32, d0[0], [%1]\n\t"   // Neon-based swap occurs here if I can make this work right.
      "vst1.32, d1[1], [%0]"
      : "+r" (*dstLeft4PixelsP), "+r" (*dstRight4PixelsP)
      );
#else
      // Left 4 pixels
      *dstLeft4PixelsP  = ((*dstLeft4PixelsP & 0xFFFF0000) >> 16) | ((*dstLeft4PixelsP & 0x0000FFFF) << 16);
      *dstLeft4PixelsP  = ((*dstLeft4PixelsP & 0xFF00FF00) >>  8) | ((*dstLeft4PixelsP & 0x00FF00FF) <<  8);
      // Right 4 pixels
      *dstRight4PixelsP = ((*dstRight4PixelsP & 0xFFFF0000) >> 16) | ((*dstRight4PixelsP & 0x0000FFFF) << 16);
      *dstRight4PixelsP = ((*dstRight4PixelsP & 0xFF00FF00) >>  8) | ((*dstRight4PixelsP & 0x00FF00FF) <<  8);  // Put right 4 pixels into left 4 pixels
      swap_(*dstLeft4PixelsP, *dstRight4PixelsP);
#endif
    }
  }
}


static void _unpackStrip4BPP(U32 **srcStripPP, U32 **dstStripPP) {
  U32 *srcStripP = *srcStripPP;
  U32 *dstStripP = *dstStripPP;
#ifdef __ARM_NEON__
  // 7 instructions neon VS 40-58 instructions regular
  asm volatile inline (
  "vmov.u8 q10 #1\n\t"   // q10 = mask
  );
  // 2 x (32 pixels per quad-word)
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 2; ++j) {
      asm volatile inline (
      "vld1.32 {d0-d1}, [%1]!\n\t"    // q0 (aka d0-d1) = packed indices
      "vand q1, q0, q10\n\t"          // q1 = unpacked indices
      "vst1.32 {d2-d3}, [%0]!\n\t"
      "vshr.u8 q0, #4\n\t"            // shift q0 over 1
      : "+r&" (dstStripP), "+r&" (srcStripP)
      );
    }
  }
#else
  // 32 pixels per iteration, 4 iterations
  for (int i = 0; i < 2; ++i, ++srcStripP) 
    for (register int j = 0; j < 8; ++j)
      *dstStripP++ =  (*srcStripP >> j) & 0x01010101;
#endif
}

static void _flipStrips4BPP(ColormapS *cmP) {
  U16 *flipEndP = cmP->stripSetP->flipSetP->flipIdxA + cmP->stripSetP->flipSetP->nFlips;
  U32 *dstLeft4PixelsP, *dstRight4PixelsP;
  // 4 *unpacked* pixels per U32 out of 64 pixels means there are 16 U32s.
  for (U16 *flipIdxP = cmP->stripSetP->flipSetP->flipIdxA; flipIdxP < flipEndP; ++flipIdxP) {
    dstLeft4PixelsP = (U32*) cmP->dataP + (*flipIdxP << 3); 
    dstRight4PixelsP = dstLeft4PixelsP + 15;
    for (; dstLeft4PixelsP < dstRight4PixelsP; ++dstLeft4PixelsP, --dstRight4PixelsP) {
#ifdef __ARM_NEON__
      asm volatile inline (
      "vldr.64 d0, [%0]\n\t"
      "vldr.64 d1, [%1]\n\t"
      "vrev.8 q0\n\t"
      "vst1.32, d0[0], [%1]\n\t"   // Neon-based swap occurs here if I can make this work right.
      "vst1.32, d1[1], [%0]"
      : "+r" (*dstLeft4PixelsP), "+r" (*dstRight4PixelsP)
      );
#else
      // Left 4 pixels
      *dstLeft4PixelsP  = ((*dstLeft4PixelsP & 0xFFFF0000) >> 16) | ((*dstLeft4PixelsP & 0x0000FFFF) << 16);
      *dstLeft4PixelsP  = ((*dstLeft4PixelsP & 0xFF00FF00) >>  8) | ((*dstLeft4PixelsP & 0x00FF00FF) <<  8);
      // Right 4 pixels
      *dstRight4PixelsP = ((*dstRight4PixelsP & 0xFFFF0000) >> 16) | ((*dstRight4PixelsP & 0x0000FFFF) << 16);
      *dstRight4PixelsP = ((*dstRight4PixelsP & 0xFF00FF00) >>  8) | ((*dstRight4PixelsP & 0x00FF00FF) <<  8);  // Put right 4 pixels into left 4 pixels
      swap_(*dstLeft4PixelsP, *dstRight4PixelsP);
#endif
    }
  }
}

// =====================================================================
// Build color map by inflating, unpacking, and piecing together strips.
// =====================================================================
static Error _cmGen(ColormapS *cmP) {
	Error e;

	if (cmP != NULL) {
		// Check if the image has already been reconstructed. If so, get out.
		if (cmP->dataP != NULL)   // ColormapS has already been reconstructed.
			return SUCCESS;  
		// If not reconstructed yet, inflate strip set if it's still compressed (inflate() checks internally).
		if (cmP->stripSetP)
			e = botoxInflate(cmP->stripSetP->stripSetInfP);
		else
			return E_NULL_VAR;
		// If CM source is strip-mapped, inflate strip map if it's still compressed (same as above).
		if (!e && cmP->stripMapP)
			e = botoxInflate(cmP->stripMapP->stripMapInfP);
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
    U32 *srcStripP;
    // Mapped stripsets need to be both unpacked and indexed. They may need strips to be flipped too.
		switch (cmP->bpp) {
			case 1:
        // First read all mapped strips into the target colormap.
				if (cmP->stripMapP) {
					U16 *mapEndP = ((U16*) cmP->stripMapP->stripMapInfP->inflatedDataP) + cmP->stripMapP->nIndices;
					for (U16 *ssIdxP = (U16*) cmP->stripMapP->stripMapInfP->inflatedDataP; ssIdxP < mapEndP; ssIdxP++) {
            srcStripP = stripSetP + (*ssIdxP << 3);
            _unpackStrip1BPP(&srcStripP, &dstStripP);
          }

					// Then flip whatever strips need flipping. Remember data's already expanded to U8s!
					if (cmP->stripSetP->flipSetP) 
            _flipStrips1BPP(cmP);
				} 
				// Unmapped stripsets are already ordered, so they only need to be unpacked.
				else {
					U32 *srcEndP = stripSetP + (cmP->stripSetP->nStrips << 3);
					for (U32 *srcStripP = stripSetP; srcStripP < srcEndP; srcStripP++) 
            _unpackStrip1BPP(&srcStripP, &dstStripP);
				}
				break;
			case 2:
        // First read all mapped strips into the target colormap.
				if (cmP->stripMapP) {
					U16 *mapEndP = ((U16*) cmP->stripMapP->stripMapInfP->inflatedDataP) + cmP->stripMapP->nIndices;
					for (U16 *ssIdxP = (U16*) cmP->stripMapP->stripMapInfP->inflatedDataP; ssIdxP < mapEndP; ssIdxP++) 
            srcStripP = stripSetP + (*ssIdxP << 3);
            _unpackStrip2BPP(&srcStripP, &dstStripP);

					// Then flip whatever strips need flipping. Remember data's already expanded to U8s!
					if (cmP->stripSetP->flipSetP) 
            _flipStrips2BPP(cmP);
				} 
				// Unmapped stripsets are already ordered, so they only need to be unpacked.
				else {
					U32 *srcEndP = stripSetP + (cmP->stripSetP->nStrips << 3);
					for (U32 *srcStripP = stripSetP; srcStripP < srcEndP; srcStripP++) 
            _unpackStrip2BPP(&srcStripP, &dstStripP);
				}
				break;
			case 4: 
        // First read all mapped strips into the target colormap.
				if (cmP->stripMapP) {
					U16 *mapEndP = ((U16*) cmP->stripMapP->stripMapInfP->inflatedDataP) + cmP->stripMapP->nIndices;
					for (U16 *ssIdxP = (U16*) cmP->stripMapP->stripMapInfP->inflatedDataP; ssIdxP < mapEndP; ssIdxP++) 
            srcStripP = stripSetP + (*ssIdxP << 3);
            _unpackStrip4BPP(&srcStripP, &dstStripP);

					// Then flip whatever strips need flipping. Remember data's already expanded to U8s!
					if (cmP->stripSetP->flipSetP) 
            _flipStrips4BPP(cmP);
				} 
				// Unmapped stripsets are already ordered, so they only need to be unpacked.
				else {
					U32 *srcEndP = stripSetP + (cmP->stripSetP->nStrips << 3);
					for (U32 *srcStripP = stripSetP; srcStripP < srcEndP; srcStripP++) 
            _unpackStrip4BPP(&srcStripP, &dstStripP);
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

#if 0
// Source: https://codeincomplete.com/articles/bin-packing/
// ========================================================

// Scratch area
// ============

// First, we sort the rectangles by their area (using a LUT). 
// Second, we put them in the atlas based on how much room there is.

// TA stands for "Texture Atlas". Spares readers.
typedef struct {
  Entity rectIdx;  // indexes the shared rectangle array in xMaster
  Key right, down;
} TaTreeNode;  // 20 bytes, oh well, best I can do

TaTreeNode* taNodeFind(TaTreeNode *rootP, TaTreeNode *nodeP, S32 w, S32 h) {
  if (root.used)
    return taNodeFind(root.right, w, h) || taNodeFind(root.down, w, h);
  else if ((w <= root.w) && (h <= root.h))
    return root;
  else
    return null;
}

TaTreeNode* taNodeSplit(TaTreeNode *nodeP, S32 w, S32 h) {
  node.down  = { x: node.x,     y: node.y + h, w: node.w,     h: node.h - h };
  node.right = { x: node.x + w, y: node.y,     w: node.w - w, h: h          };
  return node;
}

// Prevent copies with new member in XRenderComponentSrc struct: S16 textureAtlasIdx.
void taNodeFit() {
  var n, node, block, len = blocks.length;
  var w = len > 0 ? blocks[0].w : 0;
  var h = len > 0 ? blocks[0].h : 0;
  this.root = { x: 0, y: 0, w: w, h: h };
  for (n = 0; n < len ; n++) {
    block = blocks[n];
    if (node = taNodeFind(this.root, block.w, block.h))
      taNodeFit = this.taNodeSplit(node, block.w, block.h);
    else
      taNodeFit = taNodeGrow(block.w, block.h);
  }
}

TaTreeNode* taNodeGrowRight(w, h) {
  this.root = {
    used: true,
    x: 0,
    y: 0,
    w: this.root.w + w,
    h: this.root.h,
    down: this.root,
    right: { x: this.root.w, y: 0, w: w, h: this.root.h }
  };
  if (node = taNodeFind(this.root, w, h))
    return taNodeSplit(node, w, h);
  else
    return null;
}

TaTreeNode *taNodeGrowDown(w, h) {
  this.root = {
    used: true,
    x: 0,
    y: 0,
    w: this.root.w,
    h: this.root.h + h,
    down:  { x: 0, y: this.root.h, w: this.root.w, h: h },
    right: this.root
  };
  if (node = taNodeFind(this.root, w, h))
    return taNodeSplit(node, w, h);
  else
    return null;
}


TaTreeNode* taNodeGrow(S32 w, S32 h) {
  var canGrowDown  = (w <= this.root.w);
  var canGrowRight = (h <= this.root.h);

  var shouldGrowRight = canGrowRight && (this.root.h >= (this.root.w + w)); // attempt to keep square-ish by growing right when height is much greater than width
  var shouldGrowDown  = canGrowDown  && (this.root.w >= (this.root.h + h)); // attempt to keep square-ish by growing down  when width  is much greater than height

  if (shouldGrowRight)
    return taNodeGrowRight(w, h);
  else if (shouldGrowDown)
    return taNodeGrowDown(w, h);
  else if (canGrowRight)
   return taNodeGrowRight(w, h);
  else if (canGrowDown)
    return taNodeGrowDown(w, h);
  else
    return null; // need to ensure sensible root starting size to avoid this happening
}
// This is where I'll write the texture atlas algorithm.
//
// Write a insert-sort by rect w/h function that memcpys in an inner insert.
// Write some array-based binary tree functionality -- perhaps within data.c. Only what's needed.
// Sort all rectangles by 
#endif


//======================================================
// Initialize xRender's system.
//======================================================
Error xRenderIniSys(System *sP, void *sParamsP) {
	unused_(sParamsP);
  unused_(sP);
  return SUCCESS;
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
	if (!e)  
    cP->textureP = imgP->textureP;  // All other imgP attributes are set later by a b-tree.

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

// Only get the render and window. Components' src & dst rects come from SCENE_START stimulus to XGo.
XGetShareFuncDef_(Render) {
  XRender *renderSysP = (XRender*) sP;
  // Get renderer
  Error e = mapGetNestedMapPElem(shareMMP, RENDERER_GENE_TYPE, RENDERER_KEY_, (void**) &renderSysP->rendererP);
  // Get window
  if (!e)
    e = mapGetNestedMapPElem(shareMMP, WINDOW_GENE_TYPE, WINDOW_KEY_, (void**) &renderSysP->windowP);
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
