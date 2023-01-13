#include "jb.h"
// Compressed images are already in memory in JB. 
// JB just reconstructs them from strips of staggered pixels.   // TODO illustrate this with ASCII art
// Packed bits are staggered to allow JB to unpack and (if necessary) flip 4 color-mapped pixels at a time.

// =========================================
// Clear color map and all its related data.
// =========================================
void cmClr(Colormap *cmP) {
	if (cmP != NULL) {
	 	if (cmP->dataP != NULL) {    // But if the double pointer is null, avoid any processing.
			jbFree((void**) &cmP->dataP);
    }
		if (cmP->stripsetP != NULL && cmP->stripsetP->infP != NULL &&
				cmP->stripsetP->infP->inflatedDataP != NULL) {
			jbFree((void**) &cmP->stripsetP->infP->inflatedDataP);
    }
		if (cmP->stripmapP != NULL && cmP->stripmapP->infP != NULL &&
				cmP->stripmapP->infP->inflatedDataP != NULL) {
			jbFree((void**) &cmP->stripsetP->infP->inflatedDataP);
    }
	}
}

// =====================================================================
// Build color map by inflating, unpacking, and piecing together strips.
// =====================================================================
Error cmGen(Colormap *cmP) {
	Error e;

	if (cmP != NULL) {
		// Check if the image has already been reconstructed. If so, get out.
		if (cmP->dataP != NULL) {   // Colormap has already been reconstructed.
			return SUCCESS;  
    }
		// If not reconstructed yet, inflate strip set if it's still compressed (inflate() checks internally).
		if (cmP->stripsetP) {
			e = botoxInflate(cmP->stripsetP->infP);
    }
		else {
			return E_NULL_VAR;
    }

    // TODO get rid ofthis once done debugging
    printf("let us commence on spillin the strip set inflatable beans....\n");
    for (U8 *eP = (U8*) cmP->stripsetP->infP->inflatedDataP; eP < (U8*) cmP->stripsetP->infP->inflatedDataP + cmP->stripsetP->infP->inflatedLen; ++eP) {
      printf("%d ", *eP);
    }
		// If CM source is strip-mapped, inflate strip map if it's still compressed (same as above).
		if (!e && cmP->stripmapP) {
			e = botoxInflate(cmP->stripmapP->infP);
    }
		// Allocate colormap memory.
		if (!e) {
			e = jbAlloc((void**) &cmP->dataP, sizeof(U8), cmP->w * cmP->h);
    }
	}
	else {
		e = E_BAD_ARGS;
  }
	// Expand strip set into image.
	if (!e) {
    // Count remainder of pixels to process after all the whole strips.
    // Mapped stripsets need to be both unpacked and indexed. They may need strips to be flipped too.
		switch (cmP->bpp) {
			case 1:
        inflateStripsWithBpu1(cmP->stripsetP, cmP->stripmapP, (U32*) cmP->dataP);
				break;
			case 2:
        // First read all mapped strips into the target colormap.
        inflateStripsWithBpu2(cmP->stripsetP, cmP->stripmapP, (U32*) cmP->dataP);
				break;
			case 4: 
        // First read all mapped strips into the target colormap.
        inflateStripsWithBpu4(cmP->stripsetP, cmP->stripmapP, (U32*) cmP->dataP);
				break;
			default:  
				e = E_UNSUPPORTED_PIXEL_FORMAT;
				break;
		}
	}
	else {
		cmClr(cmP);
  }

	return e;
}

// Source: https://codeincomplete.com/articles/bin-packing/
// ========================================================

// Scratch area
// ============

// First, we sort the rectangles by their area (using a LUT). 
// Second, we put them in the atlas based on how much room there is.
#if 0  // let's just get the rectangle sorting to compile first, shall we?
// TA stands for "Texture Atlas". Let's spare readers' eyes.
typedef struct {
  Key srcRectIdx;  // indexes the shared source rectangle array in xMaster
  Key used;
  U16 remainingWidthRight, remainingHeightDown;  // U16 works if the max texture dimension the Pi supports is 4096.
} TaNode;  // 6 bytes

Error fit(Rect_ *srcRectA) {
  if (!srcRectA)
    return E_BAD_ARGS;
  Error e = SUCCESS;
  Rect_ *rectP = srcRectA;
  Rect_ *rectEndP = rectP + arrayGetNElems(srcRectA);
  for (; !e && rectP < rectEndP; ++rectP) {
    if (node = this.findNode(this.root, block.w, block.h))
      e = this.splitNode(node, block.w, block.h);
  }
  return e;
}

Key taNodeFind(TaNode *nodeA, Key idx, S32 w, S32 h) {
  // If this node's used up, continue looking in its neighboring nodes (going right first).
  // When it reaches the right border, it looks beneath the rightmost node.
  if (nodeP->used)
    return taNodeFind(nodeA, nodeP->right, w, h) || taNodeFind(nodeA, nodeP->down, w, h);
  // When it finds an empty node, it returns that node.
  else if ((w <= nodeA[idx].w) && (h <= nodeA[idx].h))  
    return idx;
  else
    return 0;  // This node won't work. Hopefully the next one will when this returns.
    // The first node is already automatically used by the first (largest) rectangle.
    // So a return value of 0 means the current rectangle won't fit in this node.
}

// This is where nodes are born: mitosis. w and h represent the remaining w and h in the direction from this block.
TaNode* taNodeSplit(TaNode *nodeA, Key idx, S32 w, S32 h) {
  nodeA[idx].down.x = nodeA[idx].x;
  nodeA[idx].down.y = nodeA[idx].y + h;
  nodeA[idx].down.w = nodeA[idx].w;
  nodeA[idx].down.h = nodeA[idx].h - h;
  nodeP->down  = { x: nodeP->x,     y: nodeP->y + h, w: nodeP->w,     h: nodeP->h - h };
  nodeP->right = { x: nodeP->x + w, y: nodeP->y,     w: nodeP->w - w, h: h          };
  return node;
}

// Prevent copies with new member in XRenderComponentSrc struct: S16 textureAtlasIdx.
// For array-based binary trees, left child is (parent*2)+1; right child is (parent*2)+2. So...
// p0, c01, c02, ... then you can't have an independent parent. They all descend from p0.
// p0, c01, c02, c01_01, c01_02, c02_01, c02_02, c01_01_01, c01_01_02... etc.
void taNodeFitNewBlock() {
  var n, node, block, len = blocks.length;
  var w = len > 0 ? blocks[0].w : 0;
  var h = len > 0 ? blocks[0].h : 0;
  this.root = { x: 0, y: 0, w: w, h: h };
  for (n = 0; n < len ; n++) {
    block = blocks[n];
    if (node = taNodeFind(this.root, block.w, block.h))
      taNodeFit = taNodeSplit(node, block.w, block.h);
    else
      taNodeFit = taNodeGrow(block.w, block.h);
  }
}

TaNode* taNodeGrowRight(w, h) {
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

TaNode *taNodeGrowDown(w, h) {
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


TaNode* taNodeGrow(S32 w, S32 h) {
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
#define DO_IT_THE_OLD_DUMB_WAY 0
Error xRenderIniComp(System *sP, void *compDataP, void *compDataSrcP) {
	if (!sP || !compDataP || !compDataSrcP)
		return E_BAD_ARGS;
	// Build colormap.
	//XRender *xRenderSysP = (XRender*) sP;
	//XRenderComp *cP = (XRenderComp*) compDataP;
  XRenderCompSrc *imgP = (XRenderCompSrc*) compDataSrcP;
	Error e = cmGen(imgP->colorMapP);  // I think I'm keeping this to vectorize better.

  return e;  // here for now to make the compiler happy

#if DO_IT_THE_OLD_DUMB_WAY
  // Skip things that're already done.
	if (cP->textureP) 
		return SUCCESS;
  else if (imgP->textureP) {
    cP->textureP = imgP->textureP;
    return SUCCESS;
  }
#endif
  
#if DO_IT_THE_OLD_DUMB_WAY  // I think the texture atlas obsoletes all this.
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
#endif
}

// TODO Is the source rect MPMP already shared?
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

static Error _getSharedMap(Map *shareMPMP, Key key, Map **outputMPP) {
  if (!shareMPMP || !outputMPP)
    return E_BAD_ARGS;
  Map **shareMPP = (Map**) mapGet(shareMPMP, key);
  if (!shareMPP)
    return E_BAD_KEY;
  *outputMPP = *shareMPP;
  if (!*outputMPP)
    return E_BAD_ARGS;
  return SUCCESS;
}

// Only get the render and window. Components' src & dst rects come from SCENE_START stimulus to XGo.
XGetShareFuncDef_(Render) {
  XRenderComp *cP = NULL;
  XRenderComp *cEndP = NULL;
  Entity entity = 0;
  XRender *renderSysP = (XRender*) sP;
  // Get renderer
  Error e = mapGetNestedMapPElem(shareMMP, RENDERER_GENE_TYPE, RENDERER_KEY_, (void**) &renderSysP->rendererP);
  // Get window
  if (!e)
    e = mapGetNestedMapPElem(shareMMP, WINDOW_GENE_TYPE, WINDOW_KEY_, (void**) &renderSysP->windowP);

  // Extract all source and dest rectangles we need to keep track of (they get updated by other systems).
  if (!e) 
    e = _getSharedMap(shareMMP, SRC_RECT, &renderSysP->srcRectMP);  // represents current source rectangle for, say, an animated entity
  if (!e)
    e = _getSharedMap(shareMMP, DST_RECT, &renderSysP->dstRectMP);  // represents current destination rectangle for, say, a mobile, scalable entity
  if (!e)
    // Animation system nicely makes a map of pointers to its components' srcRect arrays (2D: first dim is anim # (not important), second is array of src rects).
    e = _getSharedMap(shareMMP, SRC_RECT_ARRAY, &renderSysP->srcRectMAMP);  // represents all the possible source rectangles to keep track of in texture atlas

  // Get each component's src and dest rectangle indices.
  if (!e) {
    cP = sP->cF;
    cEndP = cP + arrayGetNElems(sP->cF);
    for (; !e && cP < cEndP; ++cP) {
      entity = xGetEntityByCompIdx(sP, cP - (XRenderComp*) sP->cF);
      e = mapGetIndex(renderSysP->srcRectMP, entity, &cP->srcRectIdx);
      if (!e) 
        e = mapGetIndex(renderSysP->dstRectMP, entity, &cP->dstRectIdx);
    }
  }

  // Get all the animation rectangles we need to update when building our texture atlas.
  // First count all the rectangles we're going to need.
  U32 nRectangles = 0;
  Rect_ *srcRectA = NULL;
  Rect_ **srcRectAP = NULL;
  Rect_ **srcRectAEndP = NULL;
  Rect_ **srcRectAA = NULL;
  Rect_ ***srcRectAAP = NULL;
  
  cEndP = cP + arrayGetNElems(sP->cF);
  if (!e) {
    for (cP = sP->cF; !e && cP < cEndP; ++cP) {
      entity = xGetEntityByCompIdx(sP, cP - (XRenderComp*) sP->cF);
      srcRectAAP = (Rect_***) mapGet(renderSysP->srcRectMAMP, entity);  // TRIPLE POINTER!!! holllly crap
      if (srcRectAAP)
        srcRectAA = *srcRectAAP;
      if (srcRectAA) {
        srcRectAP = srcRectAA;
        srcRectAEndP = srcRectAP + arrayGetNElems(srcRectAA);
        for (; srcRectAP < srcRectAEndP; ++srcRectAP) {
          srcRectA = *srcRectAP;
          if (!srcRectA) {
            e = E_NULL_VAR;
            goto ranIntoNullArray;
          }
          nRectangles += arrayGetNElems(srcRectA);
        }
      }
    }
  }
  
  // Second, insert-sort those rectangles into our own, temporary array of rectangles by their largest dimension.
  Rect_ *sortedRectA;
  Rect_ *rectP = NULL;
  Rect_ *rectEndP = NULL;
  Rect_ *sortedRectP = NULL;
  Rect_ *sortedRectEndP = NULL;
  U32 nRectsSorted = 0;

  if (!e)
    e = arrayNew((void**) sortedRectA, sizeof(Rect_), nRectangles);

  // DON'T PANIC! I know four nested for-loops looks horrifying, but it's an O(n) algorithm.
  if (!e) {
    // For each entity...
    for (cP = sP->cF; !e && cP < cEndP; ++cP) {
      entity = xGetEntityByCompIdx(sP, cP - (XRenderComp*) sP->cF);
      srcRectAAP = (Rect_***) mapGet(renderSysP->srcRectMAMP, entity);  // TRIPLE POINTER!!! holllly crap
      if (srcRectAAP)
        srcRectAA = *srcRectAAP;
      if (srcRectAA) {
        srcRectAP = srcRectAA;
        srcRectAEndP = srcRectAP + arrayGetNElems(srcRectAA);
        // For each animation strip...
        for (; srcRectAP < srcRectAEndP; ++srcRectAP) {
          srcRectA = *srcRectAP;  // we won't check for null arrays this time as we already did last time.
          rectP = srcRectA;
          rectEndP = rectP + arrayGetNElems(srcRectA);
          // For each rectangle in this strip...
          for (; rectP < rectEndP; ++rectP) {
            sortedRectP = sortedRectA;
            sortedRectEndP = sortedRectP + arrayGetNElems(sortedRectA);
            // For each sorted rectangle in the array of rectangles sorted by there largest dimensions...
            // (While iterating to nRectsSorted would be quicker, it ruins the first rect's break condition.)
            for (; sortedRectP < sortedRectEndP; ++sortedRectP) {  
              if (       ((rectP->w > rectP->h)       ? rectP->w       : rectP->h)          // new rect's max dim 
                  > (sortedRectP->w > sortedRectP->h) ? sortedRectP->w : sortedRectP->h) {  // sorted rect's max dim
                memcpy((void*) (sortedRectP + 1), (void*) sortedRectP, sizeof(Rect_) * (nRectsSorted - (sortedRectP - sortedRectA)));
                *sortedRectP = *rectP;
                break;
              }
            }
          }
        }
      }
    }
  }

  // TODO: 
  // 1) if you're updating the source rectangle on the fly, then you don't need to store it or its entity.
  //    The rectangle's purpose then is just to show how much space there is left.
  //    Once you finish, you can then just fly through the images themselves and place them where their
  //    source rectangles dictate they go.

  // Make an array-based binary tree of the sorted rectangles' placements in the texture atlas.
  if (!e) {
    //TODO
  }

  // Inflate all the components' images straight into the texture atlas.
  if (!e) {
    //TODO
  }

ranIntoNullArray:
  arrayDel((void**) &sortedRectA);
  return e;
}

//======================================================
// Render activity
//======================================================
Error xRenderRun(System *sP) {
	Error e = SUCCESS;

  XRender *renderSysP = (XRender*) sP;

	XRenderComp *cP = (XRenderComp*) sP->cF;
	XRenderComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);
	Renderer_ *rendererP = renderSysP->rendererP;
  Texture_ *sysTextureP = renderSysP->textureP;
  Rect_ *srcRectA = (Rect_*) renderSysP->srcRectMP->mapA;
  Rect_ *dstRectA = (Rect_*) renderSysP->dstRectMP->mapA;

	clearScreen(rendererP);
	for (; !e && cP < cEndP; cP++) 
		e = copy_(rendererP, sysTextureP, &srcRectA[cP->srcRectIdx], &dstRectA[cP->dstRectIdx]);
	if (!e)
		present_(rendererP);

	return e;
}

//======================================================
// System definition
//======================================================
X_(Render, RENDER, FLG_NO_SWITCHES_);
