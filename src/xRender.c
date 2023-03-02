#include "xRender.h"
#include "data.h"
#include "jb.h"
#include "x.h"
#include "xMaster.h"

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
  // Current node
  rootP[atlasIdx].used = TRUE;
  sortedRectP->rect.x = rootP[atlasIdx].x;
  sortedRectP->rect.y = rootP[atlasIdx].y;
  // Right child
  _atlasElemChildIni(
      &rootP[getRightAtlasChildIdx_(atlasIdx)], 
      rootP[atlasIdx].x + sortedRectP->rect.w,
      rootP[atlasIdx].y,
      rootP[atlasIdx].remWidth - sortedRectP->rect.w,
      sortedRectP->rect.h);  // rectHeight splits node to a shelf rightward and full width downward
  // Bottom child
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
static Error _sortRects(SortedRect **sortedRectAP, Colormap **cmPF) {
  if (!sortedRectAP || !cmPF) {
    return E_BAD_ARGS;
  }

  Error e = arrayNew((void**) sortedRectAP, sizeof(SortedRect), *frayGetFirstEmptyIdxP(cmPF));

  if (!e) {
    SortedRect *sortedRectA = *sortedRectAP;
    // Loop through unsorted rectangles
    sortedRectA[0].srcIdx = 0;
    sortedRectA[0].maxDim = cmPF[0]->w > cmPF[0]->h ?  cmPF[0]->w : cmPF[0]->h;
    sortedRectA[0].rect.w = cmPF[0]->w;
    sortedRectA[0].rect.h = cmPF[0]->h;

    // Loop through the unsorted rectangles
    for (U32 i = 1, iEnd = frayGetFirstInactiveIdx(cmPF); i < iEnd; ++i) {
      U32 currRectMaxDim = cmPF[i]->w > cmPF[i]->h ?
                           cmPF[i]->w : cmPF[i]->h;
      // Loop through sorted rectangles to see where the current unsorted one should go.
      for (U32 j = 0; j < i; ++j) {
        if (currRectMaxDim > sortedRectA[j].maxDim) {
          memcpy(&sortedRectA[j + 1], &sortedRectA[j], sizeof(SortedRect) * (i - j));
          sortedRectA[j].maxDim = currRectMaxDim;  // larger of height or width
          sortedRectA[j].srcIdx = i;  // index of sample in original array
          //sortedRectA[j].rect.x = 0;
          //sortedRectA[j].rect.y = 0;
          sortedRectA[j].rect.w = cmPF[i]->w;
          sortedRectA[j].rect.h = cmPF[i]->h;
          goto nextUnsortedRect;
        }
      }
      // If loop ended without placing rect anywhere, it belongs in last element.
      sortedRectA[i].maxDim = currRectMaxDim;  // larger of height or width
      sortedRectA[i].srcIdx = i;  // index of sample in original array
      //sortedRectA[i].rect.x = 0;
      //sortedRectA[i].rect.y = 0;
      sortedRectA[i].rect.w = cmPF[i]->w;
      sortedRectA[i].rect.h = cmPF[i]->h;
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
  U32 nComponents = xGetNComps(sP);
  Error e = frayNew((void**) &xRenderP->cmPF, sizeof(Colormap*), nComponents);
  if (!e) {
    e = frayNew((void**) &xRenderP->cpPF, sizeof(ColorPalette*), nComponents);
  }
  if (!e) {
    e = frayNew((void**) &xRenderP->entityF, sizeof(Entity), nComponents);
  }
  return e;
}

//=========================================================================
// Initialize xRender's components' elements (Colormaps and Color Palettes)
//=========================================================================
Error xRenderIniSubcomp(System *sP, const Entity entity, const Key subtype, void *dataP) {
	if (!sP || !entity || !dataP || !subtype) {
		return E_BAD_ARGS;
  }

  ColorPalette *cpP;
  Colormap *cmP;

  XRender *xRenderP = (XRender*) sP;
  Error e = SUCCESS;

  // If it's a color palette, increment the system's atlas palette offset.
  // We use type to determine system.
  // Then we use subtype to determine what it does with it. 
  switch (subtype) {
    case COLORMAP:
      cmP = (Colormap*) dataP;
      if (!(cmP->state & INITIALIZED)) {
        cmP->state |= INITIALIZED;
        e = frayAdd(xRenderP->cmPF, (void*) &cmP, NULL);
      }
      break;
    case COLOR_PALETTE:
      cpP = (ColorPalette*) dataP;
      // Prevent copies of sub-palettes in texture atlas palette by marking them as initialized.
      if (!(cpP->state & INITIALIZED)) {
        cpP->state |= INITIALIZED;
        cpP->atlasPaletteOffset = xRenderP->atlasPaletteOffset;
        xRenderP->atlasPaletteOffset += cpP->nColors;
        e = frayAdd(xRenderP->cpPF, (void*) &cpP, NULL);
      }
      break;
    case TILEMAP:   // this is for backgrounds
      // TODO 
      break;
    default:
      break;
  }
  // Only track entity for the first subcomponent. 0x40 is the lowest of the upper two bits.
  if (!e && subtype == 0x40) {
    e = frayAdd(xRenderP->entityF, (void*) &entity, NULL);
  }

  return e;
}

Error xRenderProcessMessage(System *sP, Message *msgP) {
  Error e = SUCCESS;
  unused_(sP);
  unused_(msgP);
	return e;
}

// None of the render system's specials should be cleared as the main system owns them.
XClrFuncDef_(Render) {
  XRender *xRenderP = (XRender*) sP;
  frayDel((void**) &xRenderP->cmPF);
  frayDel((void**) &xRenderP->cpPF);
  frayDel((void**) &xRenderP->entityF);
  textureDel(&xRenderP->atlasTextureP);
  if (sP->flags & RENDER_SYS_OWNS_SRC_AND_OFFSET) {
    mapDel(&xRenderP->srcRectMP);
    mapDel(&xRenderP->offsetRectMP);
  }
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
  const Colormap **cmPF = (const Colormap**) xRenderP->cmPF;
  Error e = arrayNew((void**) atlasPixelAP, sizeof(U8), atlasA[0].remWidth * atlasA[0].remHeight);
  // For each sample...
  if (!e) {
    U8 *atlasPixelA = *atlasPixelAP;
    U32 iEnd = arrayGetNElems(sortedRectA);
    for (int i = 0; i < iEnd; ++i) {
      srcIdx = sortedRectA[i].srcIdx;
      nUnitsPerStrip = cmPF[srcIdx]->sdP->ss.nUnitsPerStrip;
      nStripsPerRow = cmPF[srcIdx]->w / nUnitsPerStrip;
      smElemP    = (StripmapElem*) cmPF[srcIdx]->sdP->sm.infP->inflatedDataP;
      // For each row of this sample's atlas rectangle...
      for (int j = 0; j < sortedRectA[i].rect.h; ++j) {
        smElemEndP = smElemP + nStripsPerRow;
        dstP       = atlasPixelA + sortedRectA[i].rect.x + (j + sortedRectA[i].rect.y) * ATLAS_WIDTH;
        // Paste rectangle row
        for (; smElemP < smElemEndP; ++smElemP, dstP += nUnitsPerStrip) {
          memcpy(dstP, 
                 cmPF[srcIdx]->sdP->ss.unpackedDataP + (*smElemP * nUnitsPerStrip), 
                 nUnitsPerStrip);
        }
      }
    }
  }
  return e;
}

// Update src (usually animation) rectangles to reflect their global positions in the texture atlas.
// Prior to updating, they only reflect their local offsets.
// Good news is, entities don't need to know where their source rectangle maps are; 
// entities with common maps share a common pointer, so only one needs to update it.
#define COLORMAP_SUBCOMP_IDX getSubcompIdx_(COLORMAP)
#define TILEMAP_SUBCOMP_IDX getSubcompIdx_(TILEMAP)
#define COLOR_PALETTE_SUBCOMP_IDX getSubcompIdx_(COLOR_PALETTE)
// Here's the way this is going to work:
//
// xRender gets shared offset FRAY
// xRender updates offset for entity and stores it in that fray, getting index
// xRender writes message to animation system for each update with index and entity
// xAnimaiton gets messages and updates all its animation rectangles
// (?) How does it know if a deall is anim'd?
// Then it gets shared map of source rectangles and updates the rect there.
static Error _updateSrcRects(XRender *xRenderP, SortedRect *sortedRectA) {
  // Get all the animation rectangles we need to update when building our texture atlas.
  // First count all the rectangles we're going to need.
  Error e = SUCCESS;

  XRenderComp c;

  SubcompOwner *scoP = xRenderP->system.subcompOwnerMP->mapA;
  SubcompOwner *scoEndP = scoP + arrayGetNElems(xRenderP->system.subcompOwnerMP->mapA);

  RectOffset rectOffset = {0};
  U32  offsetIdx  = 0;
  Colormap  *cmP;
  // If we own the src rect map, we better populate its flags before we access it.
    // Give everybody an empty rectangle for now.
  if (xRenderP->system.flags & RENDER_SYS_OWNS_SRC_AND_OFFSET) {
    // Copy the flags from one map to another. It's a cheat code.
    memcpy(xRenderP->srcRectMP->flagA, xRenderP->dstRectMP->flagA, N_FLAG_BYTES * sizeof(FlagInfo));
  }
  // Update all source rectangles' XY coordinates to their global positions in texture atlas.
  // For each entity...
  for (; !e && scoP < scoEndP; ++scoP) {
    cmP = (Colormap*) scoP->subcompA[COLORMAP_SUBCOMP_IDX];
    e = scoP->owner ? SUCCESS : E_NULL_VAR;  // Having a colormap is mandatory for xRender components.
    if (!e) {
      // Source rectangle initialization
      c.srcRectP = (Rect_*) mapGet(xRenderP->srcRectMP, scoP->owner);
      if (!c.srcRectP) {
        return E_BAD_KEY;
      }
      c.srcRectP->x = sortedRectA[cmP->sortedRectIdx].rect.x;
      c.srcRectP->y = sortedRectA[cmP->sortedRectIdx].rect.y;
      c.srcRectP->w = sortedRectA[cmP->sortedRectIdx].rect.w;
      c.srcRectP->h = sortedRectA[cmP->sortedRectIdx].rect.h;
      // Add component to system
      e = xAddComp(&xRenderP->system, scoP->owner, &c);
      // If there's an animation system (which tells master that rect offsets are implied),
      // tell the animation system to update its frame rectangles' XY coordinates to their places
      // in the texture atlas.
      if (!e && xRenderP->offsetRectMP) {
        rectOffset.x = c.srcRectP->x;
        rectOffset.y = c.srcRectP->y;
        e = mapSet(xRenderP->offsetRectMP, scoP->owner, &rectOffset);
        if (!e) {
          e = mailboxWrite(xRenderP->system.mailboxF, ANIMATION, scoP->owner, UPDATE_RECT, offsetIdx);
        }
      }
    }
  }
  return e;
}

// Updates colormaps' indices to sorted rects so we can track their atlas XY offsets
static void _updateCmSrcRectIndices(Colormap **cmPF, SortedRect *sortedRectA) {
  SortedRect *srP = sortedRectA;
  SortedRect *srEndP = srP + arrayGetNElems(sortedRectA);
  for (; srP < srEndP; ++srP) {
    cmPF[srP->srcIdx]->sortedRectIdx = srP - sortedRectA;
  }
}
// Post-processing of components is done AFTER media genes are inflated and unpacked.
// Rendering media genes are flagged to skip the strip-assembling stage; that's done here.
XPostprocessCompsDef_(Render) {
  XRender *xRenderP = (XRender*) sP;

  SortedRect *sortedRectA = NULL;
  AtlasElem *atlasA = NULL;
  U8 *atlasPixelA = NULL;

  Key nEntities = xGetNComps(sP);

  // Sort rectangles
  Error e = _sortRects(&sortedRectA, xRenderP->cmPF); // Rectangle offsets
  // Let colormaps track where their rectangles are sorted.
  if (!e) {
    _updateCmSrcRectIndices(xRenderP->cmPF, sortedRectA);
  }
  // Texture atlas
  if (!e) {
    e = _atlasGen(&atlasA, nEntities, sortedRectA);
  }
  if (!e) {
    e = _assembleTextureAtlas(xRenderP, atlasA, sortedRectA, &atlasPixelA);
  }
  // TODO purge color-keyed pixels' alphas
  // Texture surface
  if (!e) {
    e = surfaceNew(&xRenderP->atlasSurfaceP, (void*) atlasPixelA, atlasA[0].remWidth, atlasA[0].remHeight);
  }
  // Texture surface palette
  if (!e) {
    ColorPalette **cpPP = xRenderP->cpPF;
    ColorPalette **cpEndPP = cpPP + *frayGetFirstEmptyIdxP(xRenderP->cpPF);
    for (; cpPP < cpEndPP; ++cpPP) {
      appendAtlasPalette(xRenderP->atlasSurfaceP, *cpPP);
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
  // TODO use subcomp map below
  // Dest rect pointers
  if (!e) {
    Entity *entityP = xRenderP->entityF;
    Entity *entityEndP = entityP + arrayGetNElems(xRenderP->entityF);
    XRenderComp *cP = NULL;
    for (; !e && entityP < entityEndP; ++entityP) {
      cP = (XRenderComp*) xGetCompPByEntity(sP, *entityP);
      if (cP) {
        cP->dstRectP = (Rect_*) mapGet(xRenderP->dstRectMP, *entityP);
        cP->dstRectP->w = cP->srcRectP->w;
        cP->dstRectP->h = cP->srcRectP->h;
      }
      if (!cP->dstRectP) {
        e = E_BAD_KEY;
      }
    }
  }

  // TODO remove this when you're ready to try out XGo-based initialization.
  if (!e) {
    frayActivateAll(sP->cF);
  }

  // Clean up.
  arrayDel((void**) &sortedRectA);
  arrayDel((void**) &atlasA);
  arrayDel((void**) &atlasPixelA);
  frayDel((void**) &xRenderP->cmPF);
  frayDel((void**) &xRenderP->cpPF);
  frayDel((void**) &xRenderP->entityF);

  return e;
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
  // Get dest rect map (implied by render component)
  if (!e) {
    e = mapGetNestedMapP(shareMMP, DST_RECT, &xRenderP->dstRectMP);  
  }
  // Get source rect and rect offset maps. Give both a chance to run if we enter this block.
  if (!e) {
    e = mapGetNestedMapP(shareMMP, SRC_RECT, &xRenderP->srcRectMP);  
    // If there's no animation system, there won't be a source rect shared map in master.
    e = mapGetNestedMapP(shareMMP, RECT_OFFSET, &xRenderP->offsetRectMP);  
  }
  // We need to tolerate an animation system not existing.
  // SRC_RECT and RECT_OFFSET share maps don't exist without an animation system.
  // Therefore, if they both don't exist, cool, just make your own src rect map. 
  // But if only one doesn't, bomb out.
  if (e && !xRenderP->srcRectMP && !xRenderP->offsetRectMP && 
            xRenderP->rendererP && xRenderP->windowP) {
    xRenderP->system.flags |= RENDER_SYS_OWNS_SRC_AND_OFFSET;
    e = mapNew(&xRenderP->srcRectMP, sizeof(Rect_), xGetNComps(sP));
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

	clearScreen(xRenderP->rendererP);
	for (; !e && cP < cEndP; cP++) {
		e = copy_(xRenderP->rendererP, xRenderP->atlasTextureP, cP->srcRectP, cP->dstRectP);
  }
	if (!e) {
		present_(xRenderP->rendererP);
  }

	return e;
}

//======================================================
// System definition
//======================================================
X_(Render, RENDER, FLG_NO_SWITCHES_);
