#include "jb.h"
#include "x.h"
// Compressed images are already in memory in JB. 
// JB just reconstructs them from strips of staggered pixels.   // TODO illustrate this with ASCII art
// Packed bits are staggered to allow JB to unpack and (if necessary) flip 4 color-mapped pixels at a time.

// =========================================
// Clear color map and all its related data.
// =========================================
void cmClr(Colormap *cmP) {
	if (cmP != NULL && cmP->sdP != NULL) {
    stripClr(cmP->sdP);
	}
}

// =====================================================================
// Build color map by inflating, unpacking, and piecing together strips.
// =====================================================================
Error cmGen(Colormap *cmP) {
	Error e;

	if (cmP != NULL && cmP->sdP != NULL) {
		// Check if the image has already been reconstructed. If so, get out.
		if (cmP->sdP->unstrippedDataA != NULL) {   // Colormap has already been reconstructed.
			return SUCCESS;  
    }
		// If not reconstructed yet, inflate strip set if it's still compressed (inflate() checks internally).
    e = stripIni(cmP->sdP);
	}
	else {
		e = E_BAD_ARGS;
  }

	return e;
}

static inline void _atlasElemChildIni(AtlasElem *childP, U32 x, U32 y, U32 remW, U32 remH) {
  childP->x = x,
  childP->y = y,
  childP->remWidth = remW;
  childP->remHeight = remH;
  childP->used = 0;
}

static void _atlasElemIni(AtlasElem *rootP, U32 atlasIdx, SortedRect *sortedRectP) {
  printf("putting rect %d in elem %d.\n", sortedRectP->srcIdx, atlasIdx);
  // Current node
  rootP[atlasIdx].used = TRUE;
  sortedRectP->rect.x = rootP[atlasIdx].x;
  sortedRectP->rect.y = rootP[atlasIdx].y;
  // Right child
  printf("populating child %d\n", getRightAtlasChildIdx_(atlasIdx));
  _atlasElemChildIni(
      &rootP[getRightAtlasChildIdx_(atlasIdx)], 
      rootP[atlasIdx].x + sortedRectP->rect.w,
      rootP[atlasIdx].y,
      rootP[atlasIdx].remWidth - sortedRectP->rect.w,
      sortedRectP->rect.h);  // rectHeight splits node to a shelf rightward and full width downward
  // Bottom child
  printf("populating child %d\n", getLowerAtlasChildIdx_(atlasIdx));
  _atlasElemChildIni(
      &rootP[getLowerAtlasChildIdx_(atlasIdx)], 
      rootP[atlasIdx].x,
      rootP[atlasIdx].y + sortedRectP->rect.h,
      rootP[atlasIdx].remWidth,
      rootP[atlasIdx].remHeight - sortedRectP->rect.h);
}

static inline U32 _rectFits(Rect_ *rectP, AtlasElem *atlasElemP) {
  return (rectP->w <= atlasElemP->remWidth && rectP->h <= atlasElemP->remHeight);
}

// Sort colormaps by largest dimension
static Error _sortRects(SortedRect **sortedRectAP, const U32 N_SAMPLES, Colormap **cmPA) {
  if (!sortedRectAP || !cmPA) {
    return E_BAD_ARGS;
  }

  Error e = arrayNew((void**) sortedRectAP, sizeof(SortedRect), N_SAMPLES);

  if (!e) {
    SortedRect *sortedRectA = *sortedRectAP;
    // Loop through unsorted rectangles
    sortedRectA[0].srcIdx = 0;
    sortedRectA[0].maxDim = cmPA[0]->w > cmPA[0]->h ?  cmPA[0]->w : cmPA[0]->h;
    sortedRectA[0].rect.w = cmPA[0]->w;
    sortedRectA[0].rect.h = cmPA[0]->h;

    // Loop through the unsorted rectangles
    for (U32 i = 1; i < N_SAMPLES; ++i) {
      U32 currRectMaxDim = cmPA[i]->w > cmPA[i]->h ?
                           cmPA[i]->w : cmPA[i]->h;
      // Loop through sorted rectangles to see where the current unsorted one should go.
      for (U32 j = 0; j < i; ++j) {
        if (currRectMaxDim > sortedRectA[j].maxDim) {
          memcpy(&sortedRectA[j + 1], &sortedRectA[j], sizeof(SortedRect) * (i - j));
          sortedRectA[j].maxDim = currRectMaxDim;  // larger of height or width
          sortedRectA[j].srcIdx = i;  // index of sample in original array
          //sortedRectA[j].rect.x = 0;
          //sortedRectA[j].rect.y = 0;
          sortedRectA[j].rect.w = cmPA[i]->w;
          sortedRectA[j].rect.h = cmPA[i]->h;
          goto nextUnsortedRect;
        }
      }
      // If loop ended without placing rect anywhere, it belongs in last element.
      sortedRectA[i].maxDim = currRectMaxDim;  // larger of height or width
      sortedRectA[i].srcIdx = i;  // index of sample in original array
      //sortedRectA[i].rect.x = 0;
      //sortedRectA[i].rect.y = 0;
      sortedRectA[i].rect.w = cmPA[i]->w;
      sortedRectA[i].rect.h = cmPA[i]->h;
      nextUnsortedRect:
      continue;
    }
  }
  return e;
}

// Texture atlas
static Error _atlasGen(AtlasElem **atlasAP, const U32 N_SAMPLES, SortedRect *sortedRectA) {
  // Binary tree of rects

  // Allocate enough room for even the extraneous children to avoid child init branching.
  Error e = binaryTreeNew_((void**) atlasAP, sizeof(AtlasElem), N_SAMPLES * 2 + 2);

  if (!e) {
    AtlasElem *atlasA = *atlasAP;
    // For each sorted rectangle...
    SortedRect *sortedRectP = sortedRectA + 1;  // 0th elem gets pre-inserted into atlas
    SortedRect *sortedRectEndP = sortedRectA + arrayGetNElems(sortedRectA);
    U32 nAtlasElems = arrayGetNElems(atlasA), searchIdx, cameFromRight,
    nGrowthsRight = 0, nGrowthsDown  = 0;
    // Initialize the first atlas elem manually so the elem ini magic works branchlessly.
    atlasA[0].x = 0;
    atlasA[0].y = 0;
    atlasA[0].remWidth = sortedRectA[0].rect.w;
    atlasA[0].remHeight = sortedRectA[0].rect.h;
    _atlasElemIni(atlasA, 0, &sortedRectA[0]);
    for (searchIdx = 0; sortedRectP < sortedRectEndP; ++sortedRectP) {
      // Forward search in texture atlas
      searchForward:  // moves only right and down till a fit or a dead-end is found
      while (searchIdx < nAtlasElems) {
        // If current node is not used, fill it (since we're sorted).
        if (!atlasA[searchIdx].used) {
          _atlasElemIni(atlasA, searchIdx, sortedRectP);
          goto nextRect;
        }
        // Does rect fit to the right?
        if (_rectFits(&sortedRectP->rect, atlasA + getRightAtlasChildIdx_(searchIdx))) {
          searchIdx = getRightAtlasChildIdx_(searchIdx);
          continue;
        }
        // Does rect fit beneath?
        else if (_rectFits(&sortedRectP->rect, atlasA + getRightAtlasChildIdx_(searchIdx))) {
          searchIdx = getLowerAtlasChildIdx_(searchIdx);
          continue;
        }
        // If you've hit a dead-end, back out until an unexplored lower direction is found.
        goto backOut;
      }

      // TODO implement if (searchIdx >= nAtlasElems) safety check here if necessary.

      // Backing out of dead ends
      backOut:  // moves only up and left till an unexplored downward direction or root is found
      while (searchIdx) {
        searchIdx = getParentAtlasIdx_(searchIdx);
        if (cameFromRight // sneakily populated in getParentAtlasIdx_()
            && _rectFits(&sortedRectP->rect, atlasA + getLowerAtlasChildIdx_(searchIdx))) {
          searchIdx = getLowerAtlasChildIdx_(searchIdx);
          goto searchForward;
        }
      }

      // If no space was found (searchIdx == 0 now), it's time to expand the atlas.
      U32 todoDeleteThis = ((sortedRectP->rect.h <= atlasA[0].remHeight))         // can right
        | ((atlasA[0].remHeight >= atlasA[0].remWidth  + sortedRectP->rect.w)  << 1) // should right
        | ((sortedRectP->rect.w <= atlasA[0].remWidth)  << 2)  // can down
        | ((atlasA[0].remWidth  >= atlasA[0].remHeight + sortedRectP->rect.h)  << 3); // should down
      switch (todoDeleteThis
      ) {
        case SHOULD_RIGHT_CAN_DOWN:
        case SHOULD_RIGHT_DOWN:
        case SHOULD_RIGHT:
        case CAN_RIGHT:
          atlasA[0].remWidth += sortedRectP->rect.w;
          atlasA[getNthRightDescendant_(++nGrowthsRight)].remWidth = sortedRectP->rect.w;
          _atlasElemIni(atlasA, getNthRightDescendant_(nGrowthsRight), sortedRectP);
          break;
        case CAN_RIGHT_SHOULD_DOWN:
        case SHOULD_DOWN:
        case CAN_DOWN:
          atlasA[0].remHeight += sortedRectP->rect.h;
          atlasA[getNthLowerDescendant_(++nGrowthsDown)].remHeight = sortedRectP->rect.h;
          _atlasElemIni(atlasA, getNthLowerDescendant_(nGrowthsDown), sortedRectP);
          break;
        default:
          break;
      }
      nextRect:
      continue;
    }  // for each sorted rectangle
  }  // if binary tree is successfully made

  return e;
}

//======================================================
// Initialize xRender's system.
//======================================================
Error xRenderIniSys(System *sP, void *sParamsP) {
	unused_(sParamsP);
  XRender *xRenderP = (XRender*) sP;
  // Components array should have already been allocated by this point, so it's safe to get its size.
  Error e = arrayNew((void**) xRenderP->cmPA, sizeof(Colormap*), xGetNComps(sP));
  if (!e) {
    e = arrayNew((void**) xRenderP->cpPA, sizeof(ColorPalette*), xGetNComps(sP));
  }
  return e;
}

//=========================================================================
// Initialize xRender's components' elements (Colormaps and Color Palettes)
//=========================================================================
Error xRenderIniCompElem(System *sP, const Entity entity, const Key subtype, void *dataP) {
	if (!sP || !entity || !dataP) {
		return E_BAD_ARGS;
  }

  ColorPalette *cpP;
  Colormap *cmP;

  XRender *xRenderP = (XRender*) sP;

  // If it's a color palette, increment the system's atlas palette offset.
  // We use type to determine system.
  // Then we use subtype to determine what it does with it. 
  switch (subtype) {
    case COLORMAP:
      cmP = (Colormap*) dataP;
      if (!(cmP->state & INITIALIZED)) {
        cmP->state |= INITIALIZED;
      }
      break;
    case COLOR_PALETTE:
      cpP = (ColorPalette*) dataP;
      // Prevent copies of sub-palettes in texture atlas palette by marking them as initialized.
      if (!(cpP->state & INITIALIZED)) {
        cpP->state |= INITIALIZED;
        cpP->atlasPaletteOffset = xRenderP->atlasPaletteOffset;
        xRenderP->atlasPaletteOffset += cpP->nColors;
      }
      break;
    default:
      break;
  }

  return SUCCESS;  // here for now to make the compiler happy
}

// TODO Is the source rect MPMP already shared?
Error xRenderProcessMessage(System *sP, Message *msgP) {
	unused_(sP);
	unused_(msgP);
	return SUCCESS;
}

// None of the render system's specials should be cleared as the main system owns them.
XClrFuncDef_(Render) {
  XRender *xRenderP = (XRender*) sP;
  arrayDel((void**) &xRenderP->cmPA);
  arrayDel((void**) &xRenderP->cpPA);
  textureDel(&xRenderP->atlasTextureP);
  // Surface_      *atlasSurfaceP;
  // Texture_      *atlasTextureP;
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

// Texture atlas array
static Error _assembleTextureAtlas(XRender *xRenderP, AtlasElem *atlasA, SortedRect *sortedRectA, U8 **atlasPixelAP) {
  U32 nStripsPerRow;
  StripmapElem *smElemP, *smElemEndP;
  U8 *dstP;
  U32 srcIdx;
  U32 nUnitsPerStrip;
  const U32 ATLAS_WIDTH = atlasA[0].remWidth;
  const U32 nEntities = xGetNComps(&xRenderP->system);
  const Colormap **cmPA = (const Colormap**) xRenderP->cmPA;
  Error e = arrayNew((void**) atlasPixelAP, sizeof(U8), atlasA[0].remWidth * atlasA[0].remHeight);
  // For each sample...
  if (!e) {
    U8 *atlasPixelA = *atlasPixelAP;
    for (int i = 0; i < nEntities; ++i) {
      srcIdx = sortedRectA[i].srcIdx;
      nUnitsPerStrip = cmPA[srcIdx]->sdP->ss.nUnitsPerStrip;
      nStripsPerRow = cmPA[srcIdx]->w / nUnitsPerStrip;
      smElemP    = (StripmapElem*) cmPA[srcIdx]->sdP->sm.infP->inflatedDataP;
      // For each row of this sample's atlas rectangle...
      for (int j = 0; j < sortedRectA[i].rect.h; ++j) {
        smElemEndP = smElemP + nStripsPerRow;
        dstP       = atlasPixelA + sortedRectA[i].rect.x + (j + sortedRectA[i].rect.y) * ATLAS_WIDTH;
        // Paste rectangle row
        for (; smElemP < smElemEndP; ++smElemP, dstP += nUnitsPerStrip) {
          memcpy(dstP, 
                 cmPA[srcIdx]->sdP->ss.unpackedDataP + (*smElemP * nUnitsPerStrip), 
                 nUnitsPerStrip);
        }
      }
    }
  }
  return e;
}

// Update src (usually animation) rectangles to reflect their global positions in the texture atlas.
// Prior to updating, they only reflect their local offsets.
static Error _updateSrcRects(XRender *xRenderP, SortedRect *sortedRectA) {
  // Get all the animation rectangles we need to update when building our texture atlas.
  // First count all the rectangles we're going to need.
  Entity entity = 0;
  Rect_ *srcRectA = NULL;
  Rect_ **srcRectAP = NULL;
  Rect_ **srcRectAEndP = NULL;
  Rect_ **srcRectAA = NULL;
  Rect_ ***srcRectAAP = NULL;
  System *sP = &xRenderP->system;
  Rect_ *rectP = NULL;
  Rect_ *rectEndP = NULL;

  Error e = SUCCESS;
  
  SortedRect *sortedRectP = sortedRectA;
  SortedRect *sortedRectEndP = sortedRectP + arrayGetNElems(sortedRectA);

  // Update all source rectangles' XY coordinates to their global positions in texture atlas.
  // For each entity...
  for (; sortedRectP < sortedRectEndP; ++sortedRectP) {
    entity = xGetEntityByCompIdx(sP, sortedRectP->srcIdx);
    // Get entity's array of arrays of source rectangles.
    srcRectAAP = (Rect_***) mapGet(xRenderP->srcRectMAMP, entity);
    if (srcRectAAP) {
      srcRectAA = *srcRectAAP;
    }
    else {
      e = E_NULL_VAR;
    }
    if (!e) {
      srcRectAP = srcRectAA;
      srcRectAEndP = srcRectAP + arrayGetNElems(srcRectAA);
      // For each animation strip...
      for (; srcRectAP < srcRectAEndP; ++srcRectAP) {
        srcRectA = *srcRectAP;  // don't check for null arrays as we already did before
        rectP = srcRectA;
        rectEndP = rectP + arrayGetNElems(srcRectA);
        // For each rectangle in this strip...
        for (; rectP < rectEndP; ++rectP) {
          rectP->x = sortedRectP->rect.x;
          rectP->y = sortedRectP->rect.y;
        }
      }
    }
  }
  return e;
}

// Post-processing of components is done AFTER media genes are inflated and unpacked.
// Rendering media genes are flagged to skip the strip-assembling stage; that's done here.
XPostprocessCompsDef_(Render) {
  Error e = SUCCESS;
  XRender *xRenderP = (XRender*) sP;
  Entity entity;

  SortedRect *sortedRectA = NULL;
  AtlasElem *atlasA = NULL;
  U8 *atlasPixelA = NULL;

  // Set each component's src and dest rectangle indices.
  XRenderComp *cP = sP->cF;
  XRenderComp *cEndP = cP + arrayGetNElems(sP->cF);
  cP = sP->cF;
  cEndP = cP + arrayGetNElems(sP->cF);
  U32 nEntities = xGetNComps(sP);

  for (; !e && cP < cEndP; ++cP) {
    entity = xGetEntityByCompIdx(sP, cP - (XRenderComp*) sP->cF);
    e = mapGetIndex(xRenderP->srcRectMP, entity, &cP->srcRectIdx);
    if (!e) {
      e = mapGetIndex(xRenderP->dstRectMP, entity, &cP->dstRectIdx);
    }
  }
  // TODO have xRenderIniSys() produce its colormap and color palette arrays. Called in xIniSys().

  // Sort rectangles
  if (!e) {
    e = _sortRects(&sortedRectA, nEntities, xRenderP->cmPA);
  }
  // Texture atlas
  if (!e) {
    e = _atlasGen(&atlasA, nEntities, sortedRectA);
  }
  if (!e) {
    e = _assembleTextureAtlas(xRenderP, atlasA, sortedRectA, &atlasPixelA);
  }
  // Texture surface
  if (!e) {
    // TODO abstract away anythihng SDL-related to interface.c
    xRenderP->atlasSurfaceP = SDL_CreateRGBSurfaceWithFormatFrom((void*) atlasPixelA, 
        atlasA[0].remWidth, atlasA[0].remHeight, 8, atlasA[0].remWidth, SDL_PIXELFORMAT_INDEX8);
  }
  if (!xRenderP->atlasSurfaceP) {
    e = E_NO_MEMORY;
  }
  // Texture surface palette
  if (!e) {
    for (int i = 0 ; i < nEntities; ++i) {
      // TODO abstract away anythihng SDL-related to interface.c
      SDL_SetPaletteColors(xRenderP->atlasSurfaceP->format->palette, 
          xRenderP->cpPA[i]->colorA, xRenderP->cpPA[i]->atlasPaletteOffset, xRenderP->cpPA[i]->nColors);
    }
  }
  // Texture
  if (!e) {
    e = textureNew(&xRenderP->atlasTextureP, xRenderP->rendererP, xRenderP->atlasSurfaceP);
  }
  // Update source rectangles. That way animation system knows where its frames are in texture atlas.
  if (!e) {
    e = _updateSrcRects(xRenderP, sortedRectA);
  }
  // Clean up.
  arrayDel((void**) &sortedRectA);
  arrayDel((void**) &atlasA);
  arrayDel((void**) &atlasPixelA);

  return SUCCESS;
}

// Only get the render and window. Components' src & dst rects come from SCENE_START stimulus to XGo.
XGetShareFuncDef_(Render) {
  XRender *xRenderP = (XRender*) sP;
  // Get renderer
  Error e = mapGetNestedMapPElem(shareMMP, RENDERER_GENE_TYPE, RENDERER_KEY_, (void**) &xRenderP->rendererP);
  // Get window
  if (!e) {
    e = mapGetNestedMapPElem(shareMMP, WINDOW_GENE_TYPE, WINDOW_KEY_, (void**) &xRenderP->windowP);
  }

  // Extract all source and dest rectangles we need to keep track of (they get updated by other systems).
  if (!e) {
    e = _getSharedMap(shareMMP, SRC_RECT, &xRenderP->srcRectMP); 
  }
  if (!e) {
    e = _getSharedMap(shareMMP, DST_RECT, &xRenderP->dstRectMP);  // represents current destination rectangle for, say, a mobile, scalable entity
  }
  if (!e) {
    // Animation system nicely makes a map of pointers to its components' srcRect arrays (2D: first dim is anim # (not important), second is array of src rects).
    e = _getSharedMap(shareMMP, SRC_RECT_ARRAY, &xRenderP->srcRectMAMP);  // represents all the possible source rectangles to keep track of in texture atlas
  }
  return e;
}

//======================================================
// Render activity
//======================================================
Error xRenderRun(System *sP) {
	Error e = SUCCESS;

  XRender *xRenderP = (XRender*) sP;

	XRenderComp *cP = (XRenderComp*) sP->cF;
	XRenderComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);
	Renderer_ *rendererP = xRenderP->rendererP;
  Texture_ *sysTextureP = xRenderP->atlasTextureP;
  Rect_ *srcRectA = (Rect_*) xRenderP->srcRectMP->mapA;
  Rect_ *dstRectA = (Rect_*) xRenderP->dstRectMP->mapA;

	clearScreen(rendererP);
	for (; !e && cP < cEndP; cP++) {
		e = copy_(rendererP, sysTextureP, &srcRectA[cP->srcRectIdx], &dstRectA[cP->dstRectIdx]);
  }
	if (!e) {
		present_(rendererP);
  }

	return e;
}

//======================================================
// System definition
//======================================================
X_(Render, RENDER, FLG_NO_SWITCHES_);
