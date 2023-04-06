#include "xRender.h"

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
		if (cmP->sdP->assembledDataA != NULL) {   // Colormap has already been reconstructed.
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

static inline Error __atlasLinkNodes(
    Atlas *atlasP,
    Key parentIdx, 
    Key childIdx, 
    Child child,   // this is child *type*, not actual child data
    U32 x, 
    U32 y, 
    U32 remW, 
    U32 remH) {
  if (childIdx >= arrayGetNElems(atlasP->btP)) {
    return E_SEGFAULT;
  }
  btLinkNodes_(atlasP->btP, &atlasP->btP[parentIdx], &atlasP->btP[childIdx], child);
  return SUCCESS;
}

static Error _atlasLinkNodes(Atlas *atlasP, const Key parentIdx, const Key childIdx, Child child) {
  // rectHeight splits node to a shelf rightward and full width downward
  if (child == RIGHT_RECT) {
    return __atlasLinkNodes(
      atlasP,
      parentIdx,
      childIdx,
      RIGHT_RECT, 
      atlasP->btP[parentIdx].rect.x + atlasP->btP[parentIdx].rect.w,
      atlasP->btP[parentIdx].rect.y,
      atlasP->btP[parentIdx].remW - atlasP->btP[parentIdx].rect.w - atlasP->btP[childIdx].rect.w,
      atlasP->btP[parentIdx].rect.h - atlasP->btP[childIdx].rect.h);  
  }
  else {  /* implicit "if child == LOWER_RECT */
    return __atlasLinkNodes(
      atlasP,
      parentIdx,
      childIdx,
      LOWER_RECT, 
      atlasP->btP[parentIdx].rect.x,
      atlasP->btP[parentIdx].rect.y + atlasP->btP[parentIdx].rect.h,
      atlasP->btP[parentIdx].remW - atlasP->btP[childIdx].rect.w,
      atlasP->btP[parentIdx].remH - atlasP->btP[parentIdx].rect.h - atlasP->btP[childIdx].rect.h);
  }
}

static inline U32 _rectFitsToTheRight(Rect_ *orphanRectP, AtlasElem *parentElemP) {
  return (orphanRectP->w <=  parentElemP->remW 
       && orphanRectP->h <= (parentElemP->remH + parentElemP->rect.h));
}

static inline U32 _rectFitsBelow(Rect_ *orphanRectP, AtlasElem *parentElemP) {
  return (orphanRectP->w <= (parentElemP->remW + parentElemP->rect.w)
       && orphanRectP->h <=  parentElemP->remH);
}

inline static void _setRectData(AtlasElem *elP, U32 _max, S32 w, S32 h, Key srcIdx) {
  elP->maxDim = _max;  // larger of height or width
  elP->srcIdx = srcIdx;  // index of sample in original array
  elP->rect.x = 0;
  elP->rect.y = 0;
  elP->rect.w = w;
  elP->rect.h = h;
  elP->remW = 0;
  elP->remH = 0;
}

// Sort colormaps by largest dimension
// TODO treat cmPA as a fray, not an array. There needs to be something to guard against this.
Error atlasNew(Atlas **atlasPP, Colormap **cmPF) {
  if (!atlasPP || !cmPF) {
    return E_BAD_ARGS;
  }

  U32 N_ATLAS_ELEMS = *_frayGetFirstEmptyIdxP(cmPF);

  Error e = jbAlloc((void**) atlasPP, sizeof(Atlas), 1);
  if (!e) {
    e = btNew((void**) &(*atlasPP)->btP, sizeof(AtlasElem), N_ATLAS_ELEMS);
  }

  if (!e) {
    AtlasElem *atlasA = (*atlasPP)->btP;
    // Populate first element so the next one has something to sort against.
    _setRectData(&atlasA[0], cmPF[0]->w > cmPF[0]->h ?  cmPF[0]->w : cmPF[0]->h,
                 cmPF[0]->w, cmPF[0]->h, 0);

    (*atlasPP)->extremityA[0] = 0;
    (*atlasPP)->extremityA[1] = 0;

    // Loop through the unsorted rectangles
    for (U32 i = 1; i < N_ATLAS_ELEMS; ++i) {
      U32 currRectMaxDim = cmPF[i]->w > cmPF[i]->h ?
                           cmPF[i]->w : cmPF[i]->h;
      // Loop through sorted rectangles to see where the current unsorted one should go.
      for (U32 j = 0; j < i; ++j) {
        if (currRectMaxDim > atlasA[j].maxDim) {
          memmove(&atlasA[j + 1], &atlasA[j], sizeof(AtlasElem) * (i - j));
          _setRectData(&atlasA[j], currRectMaxDim, cmPF[i]->w, cmPF[i]->h, i);
          goto nextUnsortedRect;
        }
      }
      // If loop ended without placing rect anywhere, it belongs in last element.
      _setRectData(&atlasA[i], currRectMaxDim, cmPF[i]->w, cmPF[i]->h, i);
      nextUnsortedRect:
      continue;
    }
  }
  return e;
}

void atlasDel(Atlas **atlasPP) {
  if (atlasPP) {
    btDel_((void**) &(*atlasPP)->btP);
    jbFree((void**) atlasPP);
  }
}

// Texture atlas
Error atlasPlanPlacements(Atlas *atlasP) {
  // We never search for rect space in orphan nodes, so we must only check the root for orphan-hood.
  if (!atlasP || btIsAnOrphan_(atlasP->btP, 0)) {
    return E_BAD_ARGS;
  }
  // For each sorted rectangle...
  AtlasElem *btP = atlasP->btP;;
  AtlasElem *orphanP = btP + 1;  // 0th elem gets pre-inserted into atlas
  AtlasElem *elemEndP = btP + arrayGetNElems(btP);
  U32 parentIdx, cameFromRight, parentEndIdx;
  assert(atlasP->extremityA[RIGHT_RECT] == 0);
  // Initialize the first atlas elem manually so the elem ini magic works branchlessly.
  btP[0].remW = btP[0].rect.w;
  btP[0].remH = btP[0].rect.h;
  Key *childA;
  Error e = SUCCESS;
  // Loop through orphan elements
  for (parentEndIdx = arrayGetNElems(btP); !e && orphanP < elemEndP; ++orphanP) {
    // Forward search in texture atlas
    searchForward:  // moves only right and down till a fit or a dead-end is found
    /* Since you're going to navigate rightward till you find nothing else to the right,
       you  may as well start your search there. */
    for (parentIdx = atlasP->extremityA[RIGHT_RECT]; parentIdx < parentEndIdx;) {
      // Navigate to the right as far as you can go.
      childA = btP[parentIdx].header.childA;
      if (childA[RIGHT_RECT]) {  // nonzero child is a popualted one unless both children = 0x33
        parentIdx = childA[RIGHT_RECT];
        continue;
      }
      if (childA[LOWER_RECT]) {
        parentIdx = childA[LOWER_RECT];
        continue;
      }
      if (parentIdx) {
        if (_rectFitsToTheRight(&orphanP->rect, &btP[parentIdx])) {
          e = _atlasLinkNodes(atlasP, parentIdx, orphanP - btP, RIGHT_RECT);
          goto nextOrphan;
        }
        // Does rect fit beneath? (right in binary tree-speak means rect to the right)
        if (_rectFitsBelow(&orphanP->rect, &btP[parentIdx])) {
          e = _atlasLinkNodes(atlasP, parentIdx, orphanP - btP, LOWER_RECT);
          goto nextOrphan;
        }
      }
      // Dead-end. Back out until an unexplored lower direction is found.
      goto backOut;
    }

    // Back out of dead ends
    backOut:  // moves only up and left till an unexplored downward direction or root is found
    while (parentIdx) {
      parentIdx = btP[parentIdx].header.parent;
      // Avoid re-entering a path you just exited by only going up and left.
      cameFromRight = parentIdx == btP[parentIdx].header.childA[RIGHT_RECT];
      // If you came from the right and see an unexplored path below, go down.
      if (cameFromRight && _rectFitsBelow(&orphanP->rect, &btP[parentIdx])) {
        parentIdx = btP[parentIdx].header.childA[LOWER_RECT];
        goto searchForward;
      }
    }

    // If no suitable parent was found (parentIdx == 0 now), it's time to expand the atlas.
    U32 todoDeleteThis = 
        ((orphanP->rect.h <= btP[0].remH))                              // can right
      | ((btP[0].remH >= btP[0].remW  + orphanP->rect.w)  << 1)  // should right
      | ((orphanP->rect.w <= btP[0].remW)  << 2)                         // can down
      | ((btP[0].remW  >= btP[0].remH + orphanP->rect.h)  << 3); // should down
    switch (todoDeleteThis) {
      case SHOULD_RIGHT_CAN_DOWN:
      case SHOULD_RIGHT_DOWN:
      case SHOULD_RIGHT:
      case CAN_RIGHT:
      default:
        if (!e) {
          btP[0].remW += orphanP->rect.w;
          //btP[atlasP->extremityA[RIGHT_RECT]].remW = orphanP->rect.w;
          e = _atlasLinkNodes(atlasP, atlasP->extremityA[RIGHT_RECT], orphanP - btP, RIGHT_RECT);
          if (!e) {
            atlasP->extremityA[RIGHT_RECT] = 
              btP[atlasP->extremityA[RIGHT_RECT]].header.childA[RIGHT_RECT];
          }
        }
        break;
      case CAN_RIGHT_SHOULD_DOWN:
      case SHOULD_DOWN:
      case CAN_DOWN:
        if (!e) {
          btP[0].remH += orphanP->rect.h;
          e = _atlasLinkNodes(atlasP, atlasP->extremityA[LOWER_RECT], orphanP - btP, LOWER_RECT);
          if (!e) {
            // Update the lowest extremity.
            atlasP->extremityA[LOWER_RECT] = 
              btP[atlasP->extremityA[LOWER_RECT]].header.childA[LOWER_RECT];
            //btP[atlasP->extremityA[LOWER_RECT]].remH = orphanP->rect.h;
          }
        }
        break;
    }
    nextOrphan:
    continue;
  }  // for each sorted rectangle

  return e;
}


//======================================================
// Initialize xRender's system.
//======================================================
Error xRenderIniSys(System *sP, void *sParamsP) {
	unused_(sParamsP);
  XRender *xP = (XRender*) sP;
  // Components array should have already been allocated by this point, so it's safe to get its size.
  U32 nComponents = xGetNComps(sP);
  Error e = frayNew((void**) &xP->cmPF, sizeof(Colormap*), nComponents);
  if (!e) {
    e = frayNew((void**) &xP->cpPF, sizeof(ColorPalette*), nComponents);
  }
  if (!e) {
    e = frayNew((void**) &xP->entityF, sizeof(Entity), nComponents);
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

  XRender *xP = (XRender*) sP;
  Error e = SUCCESS;

  // If it's a color palette, increment the system's atlas palette offset.
  // We use type to determine system.
  // Then we use subtype to determine what it does with it. 
  switch (subtype) {
    case COLORMAP:
      cmP = (Colormap*) dataP;
      if (!(cmP->state & INITIALIZED)) {
        cmP->state |= INITIALIZED;
        e = frayAdd(xP->cmPF, (void*) &cmP, NULL);
      }
      break;
    case COLOR_PALETTE:
      cpP = (ColorPalette*) dataP;
      // Prevent copies of sub-palettes in texture atlas palette by marking them as initialized.
      if (!(cpP->state & INITIALIZED)) {
        cpP->state |= INITIALIZED;
        cpP->atlasPaletteOffset = xP->atlasPaletteOffset;
        xP->atlasPaletteOffset += cpP->nColors;
        e = frayAdd(xP->cpPF, (void*) &cpP, NULL);
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
    e = frayAdd(xP->entityF, (void*) &entity, NULL);
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
  XRender *xP = (XRender*) sP;
  frayDel((void**) &xP->cmPF);
  frayDel((void**) &xP->cpPF);
  frayDel((void**) &xP->entityF);
  textureDel(&xP->atlasTextureP);
  if (sP->flags & RENDER_SYS_OWNS_SRC_AND_OFFSET) {
    mapDel(&xP->srcRectMP);
    mapDel(&xP->offsetRectMP);
  }
  return SUCCESS;
}

// Texture atlas array
static Error _assembleTextureAtlas(XRender *xP, Atlas *atlasP, U8 **atlasPixelAP) {
  U32 nStripsPerRow;
  StripmapElem *smElemP, *smElemEndP;
  U8 *dstP;
  U32 srcIdx;
  U32 nUnitsPerStrip;
  const U32 ATLAS_WIDTH = atlasP->btP[0].remW;
  const Colormap **cmPF = (const Colormap**) xP->cmPF;
  Error e = arrayNew((void**) atlasPixelAP, sizeof(U8), atlasP->btP[0].remW * atlasP->btP[0].remH);
  if (!e) {
    U8 *atlasPixelA = *atlasPixelAP;
    U32 iEnd = arrayGetNElems(atlasP->btP);
    // For each sample...
    for (U32 i = 0; i < iEnd; ++i) {
      srcIdx = atlasP->btP[i].srcIdx;
      nUnitsPerStrip = cmPF[srcIdx]->sdP->ss.nUnitsPerStrip;
      nStripsPerRow  = cmPF[srcIdx]->w / nUnitsPerStrip;
      smElemP        = (StripmapElem*) cmPF[srcIdx]->sdP->sm.infP->inflatedDataP;
      // For each row of this sample's atlas rectangle...
      for (U32 j = 0, h = atlasP->btP[i].rect.h; j < h; ++j) {
        smElemEndP = smElemP + nStripsPerRow;
        dstP       = atlasPixelA + atlasP->btP[i].rect.x + (j + atlasP->btP[i].rect.y) * ATLAS_WIDTH;
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
// Actionod news is, entities don't need to know where their source rectangle maps are; 
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
static Error _updateSrcRects(XRender *xP, Atlas *atlasP) {
  // Get all the animation rectangles we need to update when building our texture atlas.
  // First count all the rectangles we're going to need.
  Error e = SUCCESS;

  XRenderComp c;

  SubcompOwner *scoP = xP->system.subcompOwnerMP->mapA;
  SubcompOwner *scoEndP = scoP + xP->system.subcompOwnerMP->population;

  RectOffset rectOffset = {0};
  Colormap  *cmP;
  // If we own the src rect map, we better populate its flags before we access it.
    // Give everybody an empty rectangle for now.
  if (xP->system.flags & RENDER_SYS_OWNS_SRC_AND_OFFSET) {
    // Copy the flags from one map to another. It's a cheat code.
    e = mapCopyKeys(xP->srcRectMP, xP->dstRectMP);
    assert(xP->srcRectMP->population == xP->dstRectMP->population); 
    assert(arrayGetElemSz(xP->srcRectMP->mapA) == arrayGetElemSz(xP->dstRectMP->mapA)); 
  }
  // Update all source rectangles' XY coordinates to their global positions in texture atlas.
  // For each entity...
  for (; !e && scoP < scoEndP; ++scoP) {
    cmP = (Colormap*) scoP->subcompA[COLORMAP_SUBCOMP_IDX];
    e = scoP->owner ? SUCCESS : E_NULL_VAR;  // Having a colormap is mandatory for xRender components.
    if (!e) {
      // Source rectangle initialization (set flag first because implicit share maps don't know 
      // which entities they should be mapped to ahead of time... would be nice if I found a way
      // to use mapCopyKeys() for all systems prior to this function)
      if (!(xP->system.flags & RENDER_SYS_OWNS_SRC_AND_OFFSET)) {
        mapSetFlag(xP->srcRectMP, scoP->owner);
      }
      c.srcRectP = (Rect_*) mapGet(xP->srcRectMP, scoP->owner);
      if (!c.srcRectP) {
        return E_BAD_KEY;
      }
      c.srcRectP->x = atlasP->btP[cmP->sortedRectIdx].rect.x;
      c.srcRectP->y = atlasP->btP[cmP->sortedRectIdx].rect.y;
      c.srcRectP->w = atlasP->btP[cmP->sortedRectIdx].rect.w;
      c.srcRectP->h = atlasP->btP[cmP->sortedRectIdx].rect.h;
      // Add component to system
      e = xAddComp(&xP->system, scoP->owner, &c);
      // If there's an animation system (which tells master that rect offsets are implied),
      // tell the animation system to update its frame rectangles' XY coordinates to their places
      // in the texture atlas.
      if (!e && xP->offsetRectMP) {
        rectOffset.x = c.srcRectP->x;
        rectOffset.y = c.srcRectP->y;
        e = mapSet(xP->offsetRectMP, scoP->owner, &rectOffset);
        if (!e) {
          e = mailboxWrite(xP->system.mailboxF, ANIMATION, scoP->owner, UPDATE_RECT, 0);
        }
      }
    }
  }
  return e;
}

// Updates colormaps' indices to sorted rects so we can track their atlas XY offsets
static void _updateCmSrcRectIndices(Colormap **cmPF, Atlas *atlasP) {
  AtlasElem *aeP = atlasP->btP;
  AtlasElem *aeEndP = aeP + arrayGetNElems(atlasP->btP);
  for (; aeP < aeEndP; ++aeP) {
    cmPF[aeP->srcIdx]->sortedRectIdx = aeP - atlasP->btP;
  }
}

// Post-processing of components is done AFTER media genes are inflated and unpacked.
// Rendering media genes are flagged to skip the strip-assembling stage; that's done here.
XPostprocessCompsDef_(Render) {
  XRender *xP = (XRender*) sP;

  Atlas *atlasP = NULL;
  U8 *atlasPixelA = NULL;
  Surface_ *atlasSurfaceP;

  // Texture atlas
  Error e = atlasNew(&atlasP, xP->cmPF);
  if (!e) {
    e = atlasPlanPlacements(atlasP);
  }
  if (!e) {
    e = _assembleTextureAtlas(xP, atlasP, &atlasPixelA);
  }
  // Let colormaps track where their rectangles are sorted.
  if (!e) {
    _updateCmSrcRectIndices(xP->cmPF, atlasP);
  }
  // Texture surface
  if (!e) {
    e = surfaceNew(&atlasSurfaceP, (void*) atlasPixelA, atlasP->btP[0].remW, atlasP->btP[0].remH);
  }
  // Texture surface palette
#if 1
  if (!e) {
    ColorPalette **cpPP = xP->cpPF;
    ColorPalette **cpEndPP = cpPP + *_frayGetFirstEmptyIdxP(xP->cpPF);
    for (; cpPP < cpEndPP; ++cpPP) {
      appendAtlasPalette(atlasSurfaceP, *cpPP);
    }
  }
  // Zero the alpha out on black pixels since that's our invisible pixel.
  if (!e) {
    Color_ BLACK_PIXEL = {
      .r = 0,
      .g = 0,
      .b = 0,
      .a = 255
    };
    Color_ INVISIBLE_PIXEL = {0};
    Color_ *colorP = atlasSurfaceP->format->palette->colors;
    Color_ *colorEndP = colorP + atlasSurfaceP->format->palette->ncolors;
    for (; colorP < colorEndP; ++colorP) {
      if (!memcmp(colorP, &BLACK_PIXEL, sizeof(Color_))) {
        *colorP = INVISIBLE_PIXEL;
      }
    }
  }
#endif
  // Texture
  if (!e) {
    e = textureNew(&xP->atlasTextureP, xP->guiP->rendererP, atlasSurfaceP);
  }
  /* "Pixel data is not managed automatically with SDL_CreateRGBSurfaceWithFormatFrom().
      You must free the surface before you free the pixel data." */
  surfaceDel(&atlasSurfaceP);
  arrayDel((void**) &atlasPixelA);

  // Update source rectangles. That way animation system knows where its frames are in texture atlas.
  if (!e) {
    e = _updateSrcRects(xP, atlasP);
  }
  // TODO use subcomp map below
  // Dest rect pointers
  if (!e) {
    Entity *entityP = xP->entityF;
    Entity *entityEndP = entityP + arrayGetNElems(xP->entityF);
    XRenderComp *cP = NULL;
    for (; !e && entityP < entityEndP; ++entityP) {
      cP = (XRenderComp*) xGetCompPByEntity(sP, *entityP);
      if (cP) {
        cP->dstRectP = (Rect_*) mapGet(xP->dstRectMP, *entityP);
        if (!cP->dstRectP) {
          e = E_BAD_KEY;
        }
        if (!e) {
          cP->dstRectP->w = cP->srcRectP->w;
          cP->dstRectP->h = cP->srcRectP->h;
        }
      }
    }
  }

  // TODO remove this when you're ready to try out XAction-based initialization.
  if (!e) {
    _frayActivateAll(sP->cF);
  }

  // Clean up.
  atlasDel(&atlasP);
  frayDel((void**) &xP->cmPF);
  frayDel((void**) &xP->cpPF);
  frayDel((void**) &xP->entityF);

  return e;
}

// Only get the render and window. Components' src & dst rects come from SCENE_START stimulus to XAction.
XGetShareFuncDef_(Render) {
  XRender *xP = (XRender*) sP;
  // Get renderer
  Error e = mapGetNestedMapPElem(shareMMP, GUI_GENE_TYPE, GUI_KEY_, NONMAP_POINTER, (void**) &xP->guiP);
  // Get window
  if (!e) {
    e = mapGetNestedMapP(shareMMP, DST_RECT, &xP->dstRectMP);  
  }
  // Get source rect and rect offset maps. Give both a chance to run if we enter this block.
  if (!e) {
    e = mapGetNestedMapP(shareMMP, SRC_RECT, &xP->srcRectMP);  
  }
    // If there's no animation system, there won't be a source rect shared map in master.
  if (!e || e == E_BAD_KEY) {
    e = mapGetNestedMapP(shareMMP, RECT_OFFSET, &xP->offsetRectMP);  
  }
  // We need to tolerate an animation system not existing.
  // SRC_RECT and RECT_OFFSET share maps don't exist without an animation system.
  // Therefore, if they both don't exist, cool, just make your own src rect map. 
  // But if only one doesn't, bomb out.
  if (e && !xP->srcRectMP && !xP->offsetRectMP && 
            xP->guiP && xP->guiP->rendererP && xP->guiP->windowP) {
    xP->system.flags |= RENDER_SYS_OWNS_SRC_AND_OFFSET;
    // Make new maps
    e = mapNew(&xP->srcRectMP, RAW_DATA, sizeof(Rect_), xGetNComps(sP));
    if (!e) {
      e = mapNew(&xP->offsetRectMP, RAW_DATA, sizeof(RectOffset), xGetNComps(sP));
    }
    // Copy dst rect map's keys to them
    if (!e) {
      e = mapCopyKeys(xP->srcRectMP, xP->dstRectMP);
    }
    if (!e) {
      e = mapCopyKeys(xP->offsetRectMP, xP->dstRectMP);
    }
  }
  return e;
}

//======================================================
// Render activity
//======================================================
static U32 i = 0;
Error xRenderRun(System *sP) {
	Error e = SUCCESS;

  XRender *xP = (XRender*) sP;

	XRenderComp *cP = (XRenderComp*) sP->cF;
	XRenderComp *cEndP = cP + *_frayGetFirstInactiveIdxP(sP->cF);

	clearScreen(xP->guiP->rendererP);
  Renderer_ *rendererP = xP->guiP->rendererP;
	for (; !e && cP < cEndP; cP++) {
		e = copy_(rendererP, xP->atlasTextureP, cP->srcRectP, cP->dstRectP);
    // TODO delete this after you've got actions figured out
    if (i > 0 && (i % 400) == 0) {
      e = mailboxWrite(sP->mailboxF, ANIMATION, 1, ANIMATE, WALK_UP);
    }
    else if (i > 0 && (i % 200) == 0) {
      e = mailboxWrite(sP->mailboxF, ANIMATION, 1, ANIMATE, WALK_DOWN);
    }
    if (i > 0 && (i % 140) == 0) {
      e = mailboxWrite(sP->mailboxF, ANIMATION, 2, ANIMATE, WALK_DOWN);
    }
    else if (i > 0 && (i % 70) == 0) {
      e = mailboxWrite(sP->mailboxF, ANIMATION, 2, ANIMATE, WALK_RIGHT);
    }
    if (++i == 1000) {
      return E_BAD_ARGS;
    }
  }
	if (!e) {
		present_(xP->guiP->rendererP);
  }


	return e;
}

//======================================================
// System definition
//======================================================
X_(Render, RENDER, FLG_NO_MUTATIONS_);
