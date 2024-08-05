#include "xRender.h"

static inline void __atlasLinkNodes(
    Atlas *atlasP,
    Key parentIdx, 
    Key childIdx, 
    Child child,   // this is child *type*, not actual child data
    U32 x, 
    U32 y, 
    U32 remW, 
    U32 remH) {
  assert (childIdx <  arrayGetNElems(atlasP->btP));
  // Somehow this line must've gotten deleted... I don't see it in my git log though. Weird.
  // But why else would I be passing in x, y, remW, and remH?
  atlasP->btP[childIdx].rect.x = x;
  atlasP->btP[childIdx].rect.y = y;
  atlasP->btP[childIdx].remW = remW;
  atlasP->btP[childIdx].remH = remH;
  _btLinkNodes(&atlasP->btP[parentIdx].header, &atlasP->btP[childIdx].header, parentIdx, childIdx, child);
}

static void _atlasLinkNodes(Atlas *atlasP, const Key parentIdx, const Key childIdx, Child child) {
  // rectHeight splits node to a shelf rightward and full width downward
  if (child == RIGHT_RECT) {
    __atlasLinkNodes(
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
    __atlasLinkNodes(
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
Atlas* atlasNew( Image** imgPF) {
  assert (imgPF);

  const U32 N_ATLAS_ELEMS = *_frayGetFirstEmptyIdxP(imgPF);

  Atlas* atlasP = jbAlloc(sizeof(Atlas), 1);
  assert(atlasP);

  atlasP->btP = btNew(sizeof(AtlasElem), N_ATLAS_ELEMS);
  AtlasElem *atlasA = atlasP->btP;
  // Populate first element so the next one has something to sort against.
  _setRectData(&atlasA[0], imgPF[0]->cmP->w > imgPF[0]->cmP->h ?  imgPF[0]->cmP->w : imgPF[0]->cmP->h,
               imgPF[0]->cmP->w, imgPF[0]->cmP->h, 0);

  atlasP->extremityA[0] = 0;
  atlasP->extremityA[1] = 0;

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
  return atlasP;
}

void atlasDel(Atlas **atlasPP) {
  if (atlasPP) {
    _btDel((BtElHeader**) &(*atlasPP)->btP);
    jbFree((void**) atlasPP);
  }
}

// Texture atlas
void atlasPlanPlacements(Atlas *atlasP) {
  // We never search for rect space in orphan nodes, so we must only check the root for orphan-hood.
  assert (atlasP && atlasP->btP && !_btIsAnOrphan(&atlasP->btP[0].header));

  // For each sorted rectangle...
  AtlasElem *btP = atlasP->btP;
  AtlasElem *orphanP = btP + 1;  // 0th elem gets pre-inserted into atlas
  AtlasElem *elemEndP = btP + arrayGetNElems(btP);
  U32 parentIdx, cameFromRight, parentEndIdx;
  assert(atlasP->extremityA[RIGHT_RECT] == 0);
  // Initialize the first atlas elem manually so the elem ini magic works branchlessly.
  btP[0].remW = btP[0].rect.w;
  btP[0].remH = btP[0].rect.h;
  Key *childA;
  // Loop through orphan elements
  for (parentEndIdx = arrayGetNElems(btP); orphanP < elemEndP; ++orphanP) {
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
          _atlasLinkNodes(atlasP, parentIdx, orphanP - btP, RIGHT_RECT);
          goto nextOrphan;
        }
        // Does rect fit beneath? (right in binary tree-speak means rect to the right)
        if (_rectFitsBelow(&orphanP->rect, &btP[parentIdx])) {
          _atlasLinkNodes(atlasP, parentIdx, orphanP - btP, LOWER_RECT);
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
        btP[0].remW += orphanP->rect.w;
        _atlasLinkNodes(atlasP, atlasP->extremityA[RIGHT_RECT], orphanP - btP, RIGHT_RECT);
        atlasP->extremityA[RIGHT_RECT] = 
          btP[atlasP->extremityA[RIGHT_RECT]].header.childA[RIGHT_RECT];
        break;
      case CAN_RIGHT_SHOULD_DOWN:
      case SHOULD_DOWN:
      case CAN_DOWN:
        btP[0].remH += orphanP->rect.h;
        _atlasLinkNodes(atlasP, atlasP->extremityA[LOWER_RECT], orphanP - btP, LOWER_RECT);
        // Update the lowest extremity.
        atlasP->extremityA[LOWER_RECT] = 
          btP[atlasP->extremityA[LOWER_RECT]].header.childA[LOWER_RECT];
        break;
    }
    nextOrphan:
    continue;
  }  // for each sorted rectangle
}


//======================================================
// Initialize xRender's system.
//======================================================
void xRenderIniSys(System *sP, void *sParamsP) {
	unused_(sParamsP);
  XRender *xP = (XRender*) sP;
  // Components array should have already been allocated by this point, so it's safe to get its size.
  U32 nComponents = xGetNComps(sP);
  xP->imgPF = frayNew(sizeof(Image*), nComponents);
  xP->entityF = frayNew( sizeof(Entity), nComponents);
}

//=========================================================================
// Initialize xRender's components' elements (Colormaps and Color Palettes)
//=========================================================================
void xRenderIniSubcomp(System *sP, const Entity entity, const Key subtype, void *dataP) {
  assert (sP && entity && dataP && subtype);

  Image* imgP;
  XRender *xP = (XRender*) sP;

  // Images will prevent duplicate colormap-palette pairs.
  switch (subtype) {
    case IMAGE:
      imgP = (Image*) dataP;
      if (!(imgP->state & INITIALIZED)) {
        imgP->state |= INITIALIZED;
        frayAdd(xP->imgPF, (void*) &imgP, NULL);
      }
      break;
    case TILEMAP:   // this is for backgrounds
                    // TODO 
      break;
    default:
      break;
  }
  // Only track entity for the first subcomponent. 0x40 is the lowest of the upper two bits.
  if (subtype == 0x40) {
    frayAdd(xP->entityF, (void*) &entity, NULL);
  }
}

void xRenderProcessMessage(System *sP, Message *msgP) {
  unused_(sP);
  unused_(msgP);
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
}

static void fillRectFromStripmap(const Image *imgP, const Rect_* rectP, Color_* atlasPixelA, const U32 ATLAS_WIDTH) {
  assert(imgP && imgP->cpP && imgP->cpP->colorA && rectP);
  // Figure out how to increment the destination pixel per row.
  U32 nStripsPerRow;
  StripmapElem *smElemP, *smElemEndP;
  U32 nUnitsPerStrip = imgP->cmP->sdP->ss.nUnitsPerStrip;
  const U32 MEMCPY_SZ = nUnitsPerStrip * sizeof( Color_ );
  nStripsPerRow  = imgP->cmP->w / nUnitsPerStrip;
  smElemP        = (StripmapElem*) imgP->cmP->sdP->sm.infP->inflatedDataP;
  // Colorize the stripset first 
  // To colorize the stripset, we need to first stretch it to 4 times its length.
  Color_* colorizedStripsetP = arrayNew(sizeof(Color_), arrayGetNElems(imgP->cmP->sdP->ss.unpackedDataP));
  Color_* colorP = colorizedStripsetP;
  Color_* colorEndP = colorP + arrayGetNElems(colorizedStripsetP);
  Color_ *colorPaletteP = imgP->cpP->colorA;
  U8* stripSetElemP = imgP->cmP->sdP->ss.unpackedDataP;
  for (; colorP < colorEndP; ++stripSetElemP, ++colorP) {
    *colorP = colorPaletteP[ *stripSetElemP ];
  }
  // Now we can just copy straight from the colorized stripset into the target image.
  // For each row of this source rectangle...
  Color_* dstP    = atlasPixelA + rectP->x + ( rectP->y ) * ATLAS_WIDTH;
  Color_* dstEndP = dstP + ( rectP->h * ATLAS_WIDTH );
  const U32 INCREMENT = ATLAS_WIDTH - rectP->w;
  for ( ; dstP < dstEndP; dstP += INCREMENT ) {
    smElemEndP = smElemP + nStripsPerRow;
    // Paste the source row into the atlas's destination rectangle row. There can be multiple strips per row.
    for ( ; smElemP < smElemEndP; ++smElemP, dstP += nUnitsPerStrip ) {
      memcpy( dstP, 
          colorizedStripsetP + ( *smElemP * nUnitsPerStrip ), 
          MEMCPY_SZ );
    }
  }
  arrayDel((void**) &colorizedStripsetP);
}

#ifdef MULTITHREADED_
typedef struct {
  Color_* dstP;
  Color_* dstEndP;
  U8* cmA;
  Color_* cpA;
  U32 INCREMENT;
} FillRectParamsMT;

void fillPortionOfRect( FillRectParamsMT* fillRectParamsP ) {
  U8* cmElemP = fillRectParamsP->cmA;
  for ( ; fillRectParamsP->dstP < fillRectParamsP->dstEndP; fillRectParamsP->dstP += fillRectParamsP->INCREMENT ) {
    *fillRectParamsP->dstP = fillRectParamsP->cpA[ *(cmElemP++) ];
  }
}
#endif

static void fillRect( U8* cmA, Color_* cpA, Rect_* rectP, Color_* atlasPixelA, const U32 ATLAS_WIDTH ) {
  
  assert( cmA && cpA && rectP && atlasPixelA );

  const U32 INCREMENT = ATLAS_WIDTH - rectP->w;

#ifdef MULTITHREADED_
  FillRectParamsMT* paramsA = arrayNew( sizeof( FillRectParamsMT ), N_CORES );
  // TODO height sliver for each thread
  U32 heightSliver = rectP->h / N_CORES;
  for ( U32 i = 0; i < N_CORES; ++i ) {
    paramsA[i].dstP = atlasPixelA + rectP->x + ( rectP->y + ( i * heightSliver) ) * ATLAS_WIDTH;
    paramsA[i].dstEndP = paramsA[i].dstP + ( heightSliver * ATLAS_WIDTH );
    paramsA[i].cmA = cmA;
    paramsA[i].cpA = cpA;
    paramsA[i].INCREMENT = INCREMENT;
  }
  // Then finish off by giving the last thread a slightly more responsibility if the sections aren't divisible by N_CORES.
  paramsA[ N_CORES - 1 ].dstEndP += ( rectP->h % N_CORES ) * ATLAS_WIDTH;
  multithread_( fillPortionOfRect, (void*) paramsA );
  arrayDel( (void**) &paramsA );
#else
  Color_* dstP = atlasPixelA + rectP->x + ( rectP->y ) * ATLAS_WIDTH;
  Color_* dstEndP = dstP + ( rectP->h * ATLAS_WIDTH );
  U8* cmElemP = cmA;
  for ( ; dstP < dstEndP; dstP += INCREMENT ) {
    dstP = cpA[ *(cmElemP++) ];
  }
#endif

}

// Texture atlas array
static Color_* _assembleTextureAtlas(XRender *xP, Atlas *atlasP) {
  // Declare locals
  const U32 ATLAS_WIDTH = atlasP->btP[0].remW;
  const Image **imgPF = (const Image**) xP->imgPF;
  // Make output atlas image
  Color_* atlasPixelA = arrayNew(sizeof(Color_), atlasP->btP[0].remW * atlasP->btP[0].remH);
  // Number of rect nodes is the number of elements in the rectangle binary tree.
  U32 iEnd = arrayGetNElems(atlasP->btP);
  // For each source rectangle...
  U32 srcRectIdx;
  Rect_ *dstRectP;
  for (U32 i = 0; i < iEnd; ++i) {
    srcRectIdx = atlasP->btP[i].srcIdx;
    dstRectP = &atlasP->btP[i].rect;
    const Image* imgP = imgPF[srcRectIdx];
    switch(imgP->cmP->sdP->flags) {
      // When this was never stripmapped, it's just a raw colormap.
      // However, the colormap may be sourced differently.
      case SD_SKIP_INFLATION_ | SD_SKIP_ASSEMBLY_:
        break;
      case SD_SKIP_UNPACKING_ | SD_SKIP_ASSEMBLY_:
        break;
      case SD_SKIP_ASSEMBLY_:
        break;
      case SD_SKIP_INFLATION_:
      default:  // skipping nothing
        fillRectFromStripmap( imgP, (const Rect_*) dstRectP, atlasPixelA, ATLAS_WIDTH );
        break;
    }
  }
  return atlasPixelA;
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
static void _updateSrcRects(XRender *xP, Atlas *atlasP) {
  // Get all the animation rectangles we need to update when building our texture atlas.
  // First count all the rectangles we're going to need.
  XRenderComp c;

  SubcompOwner *scoP = xP->system.subcompOwnerMP->mapA;
  SubcompOwner *scoEndP = scoP + xP->system.subcompOwnerMP->population;

  RectOffset rectOffset = {0};
  Image *imgP;
  // If we own the src rect map, we better populate its flags before we access it.
  // Give everybody an empty rectangle for now.
  if (xP->system.flags & RENDER_SYS_OWNS_SRC_AND_OFFSET) {
    // Copy the flags from one map to another. It's a cheat code.
    mapCopyKeys(xP->srcRectMP, xP->dstRectMP);
    assert(xP->srcRectMP->population == xP->dstRectMP->population); 
    assert(arrayGetElemSz(xP->srcRectMP->mapA) == arrayGetElemSz(xP->dstRectMP->mapA)); 
  }
  // Update all source rectangles' XY coordinates to their global positions in texture atlas.
  // For each entity...
  for (; scoP < scoEndP; ++scoP) {
    imgP = (Image*) scoP->subcompA[IMG_SUBCOMP_IDX];
    assert(scoP->owner);  // Having a colormap is mandatory for xRender components.)E
                          // Source rectangle initialization (set flag first because implicit share maps don't know 
                          // which entities they should be mapped to ahead of time... would be nice if I found a way
                          // to use mapCopyKeys() for all systems prior to this function)
    if (!(xP->system.flags & RENDER_SYS_OWNS_SRC_AND_OFFSET)) {
      mapSetFlag(xP->srcRectMP, scoP->owner);
    }
    c.srcRectP = (Rect_*) mapGet(xP->srcRectMP, scoP->owner);
    assert (c.srcRectP);
    c.srcRectP->x = atlasP->btP[imgP->sortedRectIdx].rect.x;
    c.srcRectP->y = atlasP->btP[imgP->sortedRectIdx].rect.y;
    c.srcRectP->w = atlasP->btP[imgP->sortedRectIdx].rect.w;
    c.srcRectP->h = atlasP->btP[imgP->sortedRectIdx].rect.h;
    // Add component to system
    xAddComp(&xP->system, scoP->owner, &c);
    // If there's an animation system (which tells master that rect offsets are implied),
    // tell the animation system to update its frame rectangles' XY coordinates to their places
    // in the texture atlas.
    if (xP->offsetRectMP) {
      rectOffset.x = c.srcRectP->x;
      rectOffset.y = c.srcRectP->y;
      mapSet(xP->offsetRectMP, scoP->owner, &rectOffset);
      mailboxWrite(xP->system.mailboxF, ANIMATION, scoP->owner, UPDATE_RECT, 0);
    }
  }
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

  // Texture atlas
  Atlas* atlasP = atlasNew(xP->imgPF);
  atlasPlanPlacements(atlasP);
  Color_* atlasPixelA = _assembleTextureAtlas(xP, atlasP);
  // Let colormaps track where their rectangles are sorted.
  _updateCmSrcRectIndices(xP->imgPF, atlasP);
  // Texture surface
  Surface_* atlasSurfaceP = surfaceNew((void*) atlasPixelA, atlasP->btP[0].remW, atlasP->btP[0].remH);
  // NOTE: I got rid of the color palette appending loop. So this means every palette needs to be stored with its colormap in order to compose full texture atlas.
  // Texture
  xP->atlasTextureP = textureNew(xP->guiP->rendererP, atlasSurfaceP);
  /* "Pixel data is not managed automatically with SDL_CreateRGBSurfaceWithFormatFrom().
     You must free the surface before you free the pixel data." */
  surfaceDel(&atlasSurfaceP);
  arrayDel((void**) &atlasPixelA);

  // Update source rectangles. That way animation system knows where its frames are in texture atlas.
  _updateSrcRects(xP, atlasP);
  // TODO use subcomp map below
  // Dest rect pointers
  Entity *entityP = xP->entityF;
  Entity *entityEndP = entityP + arrayGetNElems(xP->entityF);
  XRenderComp *cP = NULL;
  for (; entityP < entityEndP; ++entityP) {
    cP = (XRenderComp*) xGetCompPByEntity(sP, *entityP);
    if (cP) {
      cP->dstRectP = (Rect_*) mapGet(xP->dstRectMP, *entityP);
      assert (cP->dstRectP);
      cP->dstRectP->w = cP->srcRectP->w;
      cP->dstRectP->h = cP->srcRectP->h;
    }
  }

  // TODO remove this when you're ready to try out XAction-based initialization.
  frayActivateAll(sP->cF);

  // Clean up.
  atlasDel(&atlasP);
  frayDel((void**) &xP->imgPF);
  frayDel((void**) &xP->entityF);
}

// Only get the render and window. Components' src & dst rects come from SCENE_START stimulus to XAction.
XGetShareFuncDef_(Render) {
  XRender *xP = (XRender*) sP;
  // Get renderer
  xP->guiP = (Gui*) mapGetNestedMapPElem(shareMPMP, GUI_GENE_TYPE, GUI_KEY_, NONMAP_POINTER);
  // Get window
  xP->dstRectMP = mapGetNestedMapP(shareMPMP, DST_RECT);  
  // Get source rect and rect offset maps. Give both a chance to run if we enter this block.
  xP->srcRectMP = mapGetNestedMapP(shareMPMP, SRC_RECT);  
  // If there's no animation system, there won't be a source rect shared map in master.
  xP->offsetRectMP = mapGetNestedMapP(shareMPMP, RECT_OFFSET);  
}

XPostMutateFuncDef_(Render) {
  unused_(sP);
  unused_(cP);
}

//======================================================
// Render activity
//======================================================
void xRenderRun(System *sP) {
  XRender *xP = (XRender*) sP;

  XRenderComp *cP = (XRenderComp*) sP->cF;
  XRenderComp *cEndP = cP + *_frayGetFirstInactiveIdxP(sP->cF);

  Renderer_ *rendererP = xP->guiP->rendererP;
  clearScreen(rendererP);
  for (; cP < cEndP; cP++) {
    copy_(rendererP, xP->atlasTextureP, cP->srcRectP, cP->dstRectP);
  }
  present_(rendererP);
}

//======================================================
// System definition
//======================================================
X_(Render, RENDER, srcRectP, FLG_NO_MUTATIONS_);
