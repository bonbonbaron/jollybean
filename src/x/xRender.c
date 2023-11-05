#include "xRender.h"

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
  // Somehow this line must've gotten deleted... I don't see it in my git log though. Weird.
  // But why else would I be passing in x, y, remW, and remH?
  atlasP->btP[childIdx].rect.x = x;
  atlasP->btP[childIdx].rect.y = y;
  atlasP->btP[childIdx].remW = remW;
  atlasP->btP[childIdx].remH = remH;
  _btLinkNodes(&atlasP->btP[parentIdx].header, &atlasP->btP[childIdx].header, parentIdx, childIdx, child);
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
// Because the atlas grows in two different dimensions as we figure out the best placement of images into it, we don't allocate its texture here yet.
Error atlasNew(Atlas **atlasPP, Image** imgPF) {
  if (!atlasPP || !imgPF) {
    return E_BAD_ARGS;
  }

  const U32 N_ATLAS_ELEMS = *_frayGetFirstEmptyIdxP(imgPF);

  Error e = jbAlloc((void**) atlasPP, sizeof(Atlas), 1);
  if (!e) {
    e = btNew((void**) &(*atlasPP)->btP, sizeof(AtlasElem), N_ATLAS_ELEMS);
  }

  if (!e) {
    AtlasElem *atlasA = (*atlasPP)->btP;
    // Populate first element so the next one has something to sort against.
    _setRectData(&atlasA[0], imgPF[0]->cmP->w > imgPF[0]->cmP->h ?  imgPF[0]->cmP->w : imgPF[0]->cmP->h,
                 imgPF[0]->cmP->w, imgPF[0]->cmP->h, 0);

    (*atlasPP)->extremityA[0] = 0;
    (*atlasPP)->extremityA[1] = 0;

    // Loop through the unsorted rectangles
    for (U32 i = 1; i < N_ATLAS_ELEMS; ++i) {
      U32 currRectMaxDim = imgPF[i]->cmP->w > imgPF[i]->cmP->h ?
                           imgPF[i]->cmP->w : imgPF[i]->cmP->h;
      // Loop through sorted rectangles to see where the current unsorted one should go.
      for (U32 j = 0; j < i; ++j) {
        if (currRectMaxDim > atlasA[j].maxDim) {
          memmove(&atlasA[j + 1], &atlasA[j], sizeof(AtlasElem) * (i - j));
          _setRectData(&atlasA[j], currRectMaxDim, imgPF[i]->cmP->w, imgPF[i]->cmP->h, i);
          goto nextUnsortedRect;
        }
      }
      // If loop ended without placing rect anywhere, it belongs in last element.
      _setRectData(&atlasA[i], currRectMaxDim, imgPF[i]->cmP->w, imgPF[i]->cmP->h, i);
      nextUnsortedRect:
      continue;
    }
  }
  return e;
}

void atlasDel(Atlas **atlasPP) {
  if (atlasPP) {
    _btDel((BtElHeader**) &(*atlasPP)->btP);
    jbFree((void**) atlasPP);
  }
}

// Texture atlas
Error atlasPlanPlacements(Atlas *atlasP) {
  // We never search for rect space in orphan nodes, so we must only check the root for orphan-hood.
  if (!atlasP || !atlasP->btP || _btIsAnOrphan(&atlasP->btP[0].header)) {
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
      // Avoid re-entering a path you just exited by only going up and left.
      cameFromRight = parentIdx == btP[parentIdx].header.childA[RIGHT_RECT];
      // Go to parent node.
      parentIdx = btP[parentIdx].header.parent;
      // If you moved left to get to parent node and see an unexplored path below, go down.
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
  Error e = frayNew((void**) &xP->imgPF, sizeof(Image*), nComponents);
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

  Image* imgP;

  XRender *xP = (XRender*) sP;
  Error e = SUCCESS;

  // Images will prevent duplicate colormap-palette pairs.
  switch (subtype) {
    case IMAGE:
      imgP = (Image*) dataP;
      if (!(imgP->state & INITIALIZED)) {
        imgP->state |= INITIALIZED;
        e = frayAdd(xP->imgPF, (void*) &imgP, NULL);
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
  frayDel((void**) &xP->imgPF);
  frayDel((void**) &xP->entityF);
  textureDel(&xP->atlasTextureP);
  if (sP->flags & RENDER_SYS_OWNS_SRC_AND_OFFSET) {
    mapDel(&xP->srcRectMP);
    mapDel(&xP->offsetRectMP);
  }
  return SUCCESS;
}

// Texture atlas array
// TODO this wasn't designed to take color palettes into consideration at all, trusting runtime to do that every frame. Screw that. 
//      We need to ensure it copies them into the final picture now.
//      1) Stretch allocated texture atlas memory out by a factor of 4.
//      2) Figure out a way to include color palette in args.
//      3) index every individual colormap's color palette. Multiple CMs can have different CPs. 
static Error _assembleTextureAtlas(XRender *xP, Atlas *atlasP, U8 **atlasPixelAP) {
  // Declare locals
  U32 nStripsPerRow;
  StripmapElem *smElemP, *smElemEndP;
  U8 *dstP;
  U32 srcIdx;
  U32 nUnitsPerStrip;
  const U32 ATLAS_WIDTH = atlasP->btP[0].remW;
  const Image **imgPF = (const Image**) xP->imgPF;
  // Make output atlas image
  Error e = arrayNew((void**) atlasPixelAP, sizeof(U8), atlasP->btP[0].remW * atlasP->btP[0].remH);
  if (!e) {
    // This'll refer to the atlas' array of pixels 
    U8 *atlasPixelA = *atlasPixelAP;
    // Number of rect nodes is the  number of elements in the rectangle binary tree.
    U32 iEnd = arrayGetNElems(atlasP->btP);
    // TODO why are you using stripmap elements here? Isn't your data already assembled by this point?
    // For each source rectangle...
    for (U32 i = 0; i < iEnd; ++i) {
      srcIdx = atlasP->btP[i].srcIdx;
      nUnitsPerStrip = imgPF[srcIdx]->cmP->sdP->ss.nUnitsPerStrip;
      nStripsPerRow  = imgPF[srcIdx]->cmP->w / nUnitsPerStrip;
      smElemP        = (StripmapElem*) imgPF[srcIdx]->cmP->sdP->sm.infP->inflatedDataP;
      // For each row of this source rectangle...
      for (U32 j = 0, h = atlasP->btP[i].rect.h; j < h; ++j) {
        smElemEndP = smElemP + nStripsPerRow;
        dstP       = atlasPixelA + atlasP->btP[i].rect.x + (j + atlasP->btP[i].rect.y) * ATLAS_WIDTH;
        // Paste the source row into the atlas's destination rectangle row.
        // TODO change this to copy colors, not indices, into texture atlas.
        //      You know, it seems like the fastest way to go about this is to only copy colors into a stripmap
        //      Oh shit... I'm assuming stripmaps here when that won't always be the case.
        //      That means I'm going to have to restructure at least this portion of the code.
        //      How many places use stripmaps in this file?
        //      GOOD! only this part!
        //      So I need an if-statement to take care of two different copying mechanisms for me.
        //      TODO first, focus as you were, top-down. Check onn the atals allocation
        for (; smElemP < smElemEndP; ++smElemP, dstP += nUnitsPerStrip) {
          memcpy(dstP, 
                 imgPF[srcIdx]->cmP->sdP->ss.unpackedDataP + (*smElemP * nUnitsPerStrip), 
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
#define IMG_SUBCOMP_IDX getSubcompIdx_(IMAGE)
#define TILEMAP_SUBCOMP_IDX getSubcompIdx_(TILEMAP)
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
  Image *imgP;
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
    imgP = (Image*) scoP->subcompA[IMG_SUBCOMP_IDX];
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
      c.srcRectP->x = atlasP->btP[imgP->sortedRectIdx].rect.x;
      c.srcRectP->y = atlasP->btP[imgP->sortedRectIdx].rect.y;
      c.srcRectP->w = atlasP->btP[imgP->sortedRectIdx].rect.w;
      c.srcRectP->h = atlasP->btP[imgP->sortedRectIdx].rect.h;
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
static void _updateCmSrcRectIndices(Image **imgPF, Atlas *atlasP) {
  AtlasElem *aeP = atlasP->btP;
  AtlasElem *aeEndP = aeP + arrayGetNElems(atlasP->btP);
  for (; aeP < aeEndP; ++aeP) {
    imgPF[aeP->srcIdx]->sortedRectIdx = aeP - atlasP->btP;
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
  Error e = atlasNew(&atlasP, xP->imgPF);
  if (!e) {
    e = atlasPlanPlacements(atlasP);
  }
  if (!e) {
    e = _assembleTextureAtlas(xP, atlasP, &atlasPixelA);
  }
  // Let colormaps track where their rectangles are sorted.
  if (!e) {
    _updateCmSrcRectIndices(xP->imgPF, atlasP);
  }
  // Texture surface
  if (!e) {
    e = surfaceNew(&atlasSurfaceP, (void*) atlasPixelA, atlasP->btP[0].remW, atlasP->btP[0].remH);
  }
  // NOTE: I got rid of the color palette appending loop. So this means every palette needs to be stored with its colormap in order to compose full texture atlas.
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
    frayActivateAll(sP->cF);
  }

  // Clean up.
  atlasDel(&atlasP);
  frayDel((void**) &xP->imgPF);
  frayDel((void**) &xP->entityF);

  return e;
}

// Only get the render and window. Components' src & dst rects come from SCENE_START stimulus to XAction.
XGetShareFuncDef_(Render) {
  XRender *xP = (XRender*) sP;
  // Get renderer
  Error e = mapGetNestedMapPElem(shareMPMP, GUI_GENE_TYPE, GUI_KEY_, NONMAP_POINTER, (void**) &xP->guiP);
  // Get window
  if (!e) {
    e = mapGetNestedMapP(shareMPMP, DST_RECT, &xP->dstRectMP);  
  }
  // Get source rect and rect offset maps. Give both a chance to run if we enter this block.
  if (!e) {
    e = mapGetNestedMapP(shareMPMP, SRC_RECT, &xP->srcRectMP);  
  }
    // If there's no animation system, there won't be a source rect shared map in master.
  if (!e || e == E_BAD_KEY) {
    e = mapGetNestedMapP(shareMPMP, RECT_OFFSET, &xP->offsetRectMP);  
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

XPostMutateFuncDef_(Render) {
  unused_(sP);
  unused_(cP);
  return SUCCESS;
}

//======================================================
// Render activity
//======================================================
Error xRenderRun(System *sP) {
	Error e = SUCCESS;

  XRender *xP = (XRender*) sP;

	XRenderComp *cP = (XRenderComp*) sP->cF;
	XRenderComp *cEndP = cP + *_frayGetFirstInactiveIdxP(sP->cF);

  Renderer_ *rendererP = xP->guiP->rendererP;
	clearScreen(rendererP);
	for (; !e && cP < cEndP; cP++) {
		e = copy_(rendererP, xP->atlasTextureP, cP->srcRectP, cP->dstRectP);
  }
	if (!e) {
		present_(rendererP);
  }


	return e;
}

//======================================================
// System definition
//======================================================
X_(Render, RENDER, srcRectP, FLG_NO_MUTATIONS_);
