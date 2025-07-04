#include "x/xRender.h"

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

  Atlas* atlasP = memAdd(sizeof(Atlas), TEMPORARY );
  atlasP->btP = btNew(sizeof(AtlasElem), N_ATLAS_ELEMS, TEMPORARY );
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
  xP->imgPF = frayNew(sizeof(Image*), nComponents, TEMPORARY);
  xP->entityF = frayNew( sizeof(Entity), nComponents, TEMPORARY);
}

//=========================================================================
// Initialize xRender's components' elements (images and tilemaps)
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

/* I like this algo best thus far:
 *
 * For every overlapping member of the same Z-height layer (A), 
 * sort rendering order by z-height. This is the beauty of doubly-
 * linked lists: You never have to worry about mixing them up with
 * with another layer. Because the head and tail tell you the bounds.
 * Since you don't necessarily know how far apart they are, you can 
 * just "move" them to the front of the list! Wow, that's beautiful.
 *
 * However, this still dictates that you sort the whole list out at
 * load time. (B)
 *
 * A> The collision system will notify xRender of colliding entities
 *    living on the same layer. Does it need to share the same z-list
 *    as xRender's? Not necessarily; not every rendered entity has a
 *    collision box. I'll flesh this answer out later after my bike ride.
 *
 * b> What're you going to do with deactivated list members?
 *    Remove them. When I reactivate them, I'll look into the
 *    map of lists with the z-height as the key. If they don't
 *    collide with anybody, then, since the front of the list is
 *    reserved for sorted overlapping members, they'll go to the
 *    back.
 *
 * C> What'll you do for members who elevate to heights without any
 *    other members? And what happens if a lot of other members join
 *    them there too? Won't that warrant a list? Maybe make an outer
 *    structure; I don't want to put too related items into too many
 *    separate boxes just in the "spirit of ECS." Things accessed
 *    together live together.
 *
 *    It's tempting to daisy-chain lists together for rapid processing,
 *    but the [de]activation plan muddies that up quickly. So don't do
 *    that. What about something like this:
 *
 *    typedef struct ZList {
 *      LNodeHeader hdr;
 *      List zList;
 *    } Zlist;
 *
 *    The index of the above will be implied by the z-height. When 
 *    any z-height loses its last member, it's removed from the list
 *    of lists. But when a list gains its first member, we have no
 *    idea who its neighboring lists should be. (D)
 *
 * D>
 *
 */

static void raiseWithinZ( System* sP, Entity entity ) {
  if ( _frayElemIsActive( sP->cF, *_getCompIdxPByEntity(sP, entity) ) ) {
    XRenderComp* firstPausedCP = sP->cF + _frayGetFirstPausedIdx(sP->cF);
    XRenderComp* cP = xGetCompPByEntity( sP, entity );
    S32 currEntityBottomYCoord = cP->dstRectP->y + cP->dstRectP->h;
    for ( XRenderComp* nextCP = cP + 1 ; nextCP < firstPausedCP && *nextCP->zHeightP == *cP->zHeightP ; ++nextCP ) {
      // if their bottom is lower than mine, swap.
      if ( ( nextCP->dstRectP->y + nextCP->dstRectP->h ) < currEntityBottomYCoord ) {
      }
      else {
        // Break if everything else is lower than this.
        break;
      }
    }
  }
}

static void lowerWithinZ( System* sP, Entity entity ) {
  if ( _frayElemIsActive( sP->cF, *_getCompIdxPByEntity(sP, entity) ) ) {
    XRenderComp* cP = xGetCompPByEntity( sP, entity );
    S32 currEntityBottomYCoord = cP->dstRectP->y + cP->dstRectP->h;
    for ( XRenderComp* prevCP = cP - 1 ; prevCP >= (XRenderComp*) sP->cF && *prevCP->zHeightP == *cP->zHeightP ; --prevCP ) {
      // if their bottom is higher than mine, swap.
      if ( ( prevCP->dstRectP->y + prevCP->dstRectP->h ) > currEntityBottomYCoord ) {
      }
      else {
        // Break if everything else is higher than this.
        break;
      }
    }
  }
}

#if 0
static void raiseZ( Key desiredZ ) {
  myZ = desiredZ
  currEntityBottomYCoord = our dest rect Y + our H
  for each zhIdx in zHeightIdxA (starting at the end of mapA, going backwards):
    theirZ = cF[zhIdx]
    if theirZ == myZ:
      if their dest rect Y + their dest rect H < currEntityBottomYCoord
        /* Since we're starting from the right side of the Z section,
           we have to lower instead of raise to correct spot. */
        lowerWithinZ()  // starting from the right side
    if theirZ > myZ:
      __xSwap( &cF[zhIdx++], &cF[currIdx] );
    assert( false );  // the Zs are out of order!
    // Assume their Z is always greater than or equal to our Z
}
    
static void zSortUpward( desiredZ ) {
  myZ = desiredZ
  currEntityBottomYCoord = our dest rect Y + our H
  for each zhIdx in zHeightIdxA (starting at the end of mapA, going backwards):
    theirZ = cF[zhIdx]
    if theirZ == myZ:
      if their dest rect Y + their dest rect H < currEntityBottomYCoord
        /* Since we're starting from the left side of the Z section,
           we have to raise instead of lower to correct spot. */
        raiseWithinZ()  // starting from the right side
    if theirZ > myZ:
      __xSwap( &cF[zhIdx++], &cF[currIdx] );
    assert( false );  // the Zs are out of order!
    // Assume their Z is always greater than or equal to our Z
}
#endif 


void xRenderProcessMessage(System *sP, Message *msgP) {
  switch( msgP->cmd ) {
    case MSG_MOVED_Y_UP:
      lowerWithinZ( sP, msgP->attn );
      break;
    case MSG_MOVED_Y_DOWN:
      raiseWithinZ( sP, msgP->attn );
      break;
    case MSG_MOVED_Z_UP:
    case MSG_MOVED_Z_DOWN:
    default:
      break;
  }

  unused_(sP);
  unused_(msgP);
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
  Color_* colorizedStripsetP = arrayNew(sizeof(Color_), arrayGetNElems(imgP->cmP->sdP->ss.unpackedDataA), TEMPORARY);
  Color_* colorP = colorizedStripsetP;
  Color_* colorEndP = colorP + arrayGetNElems(colorizedStripsetP);
  Color_ *colorPaletteP = imgP->cpP->colorA;
  U8* stripSetElemP = imgP->cmP->sdP->ss.unpackedDataA;
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
}

#ifdef MULTITHREADED_

typedef struct {
  Color_* dstP;
  Color_* dstEndP;
  U8* cmA;
  Color_* cpA;
  U32 INCREMENT;
  U32 rectWidth;
} FillRectParamsMT;

static void fillPortionOfRect( FillRectParamsMT* fillRectParamsP ) {
  U8* cmElemP = fillRectParamsP->cmA;
  Color_* dstP = fillRectParamsP->dstP;
  Color_* dstEndP = fillRectParamsP->dstEndP;
  Color_* cpA = fillRectParamsP->cpA;
  size_t INCREMENT = fillRectParamsP->INCREMENT;
  size_t rectWidth = fillRectParamsP->rectWidth;

  assert( dstEndP > dstP );
  for ( ; dstP < dstEndP; dstP += INCREMENT ) {
    const Color_* dstRowEndP = dstP + rectWidth;
    for ( ; dstP < dstRowEndP; ++dstP ) {
      assert( ( cmElemP - fillRectParams
      *dstP = cpA[ *(cmElemP++) ];  
    }
  }
}

#endif


static void fillRect( U8* cmA, Color_* cpA, const Rect_* rectP, Color_* atlasPixelA, const U32 ATLAS_WIDTH ) {
  assert( cmA && cpA && rectP && atlasPixelA );

  const U32 INCREMENT = ATLAS_WIDTH - rectP->w;

#ifdef MULTITHREADED_
  const U32 N_THREADS = ( rectP->h < N_CORES ) ? rectP->h : N_CORES;

  FillRectParamsMT* paramsA = arrayNew( sizeof( FillRectParamsMT ), N_THREADS, TEMPORARY );
  U32 heightSliver = rectP->h / N_THREADS;
  for ( U32 i = 0; i < N_THREADS; ++i ) {
    paramsA[i].dstP      = atlasPixelA + rectP->x + ( rectP->y + ( i * heightSliver) ) * ATLAS_WIDTH;
    paramsA[i].dstEndP   = paramsA[i].dstP + ( heightSliver * ATLAS_WIDTH );
    paramsA[i].cmA       = cmA + ( i * heightSliver ) * rectP->w;  // rect is as wide as source colormap
    paramsA[i].cpA       = cpA;
    paramsA[i].INCREMENT = INCREMENT;
    paramsA[i].rectWidth = rectP->w;
    assert( paramsA[i].dstEndP > paramsA[i].dstP );
  }
  FillRectParamsMT** ptrA = arrayNew( sizeof( FillRectParamsMT* ), N_THREADS, TEMPORARY );
  for ( U32 i = 0; i < N_THREADS; ++i ) {
    ptrA[i] = &paramsA[i];
  }
  // Then finish off by giving the last thread a slightly more responsibility if the sections aren't divisible by N_THREADS.
  paramsA[ N_THREADS - 1 ].dstEndP += ( rectP->h % N_THREADS ) * ATLAS_WIDTH;
  multithread_( fillPortionOfRect, (void*) ptrA );
#else
  Color_* dstP = atlasPixelA + rectP->x + ( rectP->y ) * ATLAS_WIDTH;
  Color_* dstEndP = dstP + ( rectP->h * ATLAS_WIDTH );
  U8* cmElemP = cmA;

  assert( arrayGetNElems( cmA ) == ( rectP->w * rectP->h ) );
  // assert( ( dstEndP - dstP ) /  == rectP->h );
  // assert( ( dstEndP - dstP ) / INCREMENT == rectP->h );

  for ( ; dstP < dstEndP; dstP += INCREMENT ) {
    const Color_* dstRowEndP = dstP + rectP->w;
    for ( ; dstP < dstRowEndP; ++dstP ) {
      assert( cmElemP < ( cmA + arrayGetNElems( cmA ) ) );
      *dstP = cpA[ *(cmElemP++) ];
    }
  }
#endif
}

// Texture atlas array
Color_* assembleTextureAtlas(Image** imgPF, Atlas *atlasP) {
  // Declare locals
  const U32 ATLAS_WIDTH = atlasP->btP[0].remW;
  // Make output atlas image
  Color_* atlasPixelA = arrayNew(sizeof(Color_), atlasP->btP[0].remW * atlasP->btP[0].remH, TEMPORARY);
  // Not even sure if memsetting the above array matters. Looks white to me either way.
  // memset( atlasPixelA, 0, arrayGetNElems( atlasPixelA ) * arrayGetElemSz( atlasPixelA ) );

  // Number of rect nodes is the number of elements in the rectangle binary tree.
  U32 iEnd = arrayGetNElems(atlasP->btP);
  // For each source rectangle...
  for (U32 i = 0; i < iEnd; ++i) {
    U32 srcRectIdx = atlasP->btP[i].srcIdx;
    const Rect_* dstRectP = &atlasP->btP[i].rect;  // not to be confused with the component's dstRectP
    const Image* imgP = imgPF[srcRectIdx];
    // If it needs to be assembled, you have to assemble strips 
    // into a full grayscale image first.
    if ( ! ( imgP->cmP->sdP->flags & SD_SKIP_ASSEMBLY_ ) ) {
      fillRectFromStripmap( imgP, dstRectP, atlasPixelA, ATLAS_WIDTH );
    }
    // Otherwise, it's a grayscale image that's ready to go.
    else {
      U8* cmP = ssGetOutput( imgP->cmP->sdP );
      fillRect( cmP, imgP->cpP->colorA, dstRectP, atlasPixelA, ATLAS_WIDTH );
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
  // At this point, empty components will already have been added by xAddComp().
  XRenderComp* cP;

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
    assert(scoP->owner);  
    cP = xGetCompPByEntity( &xP->system, scoP->owner );
    // Having a colormap is mandatory for xRender components.
    // Source rectangle initialization (set flag first because implicit share maps don't know 
    // which entities they should be mapped to ahead of time... would be nice if I found a way
    // to use mapCopyKeys() for all systems prior to this function)
    if (!(xP->system.flags & RENDER_SYS_OWNS_SRC_AND_OFFSET)) {
      mapSetFlag(xP->srcRectMP, scoP->owner);
    }
    cP->srcRectP = (Rect_*) mapGet(xP->srcRectMP, scoP->owner);
    assert (cP->srcRectP);
    cP->srcRectP->x = atlasP->btP[imgP->sortedRectIdx].rect.x;
    cP->srcRectP->y = atlasP->btP[imgP->sortedRectIdx].rect.y;
    cP->srcRectP->w = atlasP->btP[imgP->sortedRectIdx].rect.w;
    cP->srcRectP->h = atlasP->btP[imgP->sortedRectIdx].rect.h;
    // If there's an animation system (which tells master that rect offsets are implied),
    // tell the animation system to update its frame rectangles' XY coordinates to their places
    // in the texture atlas.
    if (xP->offsetRectMP) {
      rectOffset.x = cP->srcRectP->x;
      rectOffset.y = cP->srcRectP->y;
      mapSet(xP->offsetRectMP, scoP->owner, &rectOffset);
      mailboxWrite(xP->system.mailboxF, ANIMATION, scoP->owner, UPDATE_RECT, 0, NULL);
    }
  }
}

// Updates colormaps' indices to sorted rects so we can track their atlas XY offsets
void updateCmSrcRectIndices(Image **imgPF, Atlas *atlasP) {
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

  // Ensure we've grabbed our shared maps by this point.
  assert( xP->srcRectMP );
  assert( xP->dstRectMP );
  assert( xP->zHeightMP );

  // Texture atlas
  Atlas* atlasP = atlasNew(xP->imgPF);
  atlasPlanPlacements(atlasP);
  Color_* atlasPixelA = assembleTextureAtlas(xP->imgPF, atlasP);
  // Let colormaps track where their rectangles are sorted.
  updateCmSrcRectIndices(xP->imgPF, atlasP);
  // Texture surface
  Surface_* atlasSurfaceP = surfaceNew((void*) atlasPixelA, atlasP->btP[0].remW, atlasP->btP[0].remH);
  // NOTE: I got rid of the color palette appending loop. So this means every palette needs to be stored with its colormap in order to compose full texture atlas.
  // Texture
  xP->atlasTextureP = textureNew(xP->guiP->rendererP, atlasSurfaceP);
  /* "Pixel data is not managed automatically with SDL_CreateRGBSurfaceWithFormatFrom().
     You must free the surface before you free the pixel data." */
  surfaceDel(&atlasSurfaceP);

  // Update source rectangles. That way animation system knows where its frames are in texture atlas.
  _updateSrcRects(xP, atlasP);
  // TODO use subcomp map below
  // Dest rect pointers
  Entity *entityP = xP->entityF;
  Entity *entityEndP = entityP + arrayGetNElems(xP->entityF);
  XRenderComp *cP = NULL;
  for (; entityP < entityEndP; ++entityP) {
    cP = (XRenderComp*) xGetCompPByEntity(sP, *entityP);
    assert (cP);
    cP->dstRectP = (Rect_*) mapGet(xP->dstRectMP, *entityP);
    assert (cP->dstRectP);
    cP->dstRectP->w = cP->srcRectP->w;
    cP->dstRectP->h = cP->srcRectP->h;
    cP->zHeightP    = mapGet( xP->zHeightMP, *entityP );
    assert( cP->zHeightP );
  }

  // Figure out what the highest Z-height is and create an array that can hold it.
  Key* zHeightP    = xP->zHeightMP->mapA;
  Key* zHeightEndP = zHeightP + xP->zHeightMP->population;
  Key maxZ = 0;
  for ( ; zHeightP < zHeightEndP; ++zHeightP ) {
    if ( *zHeightP > maxZ ) {
      maxZ = *zHeightP;
    }
  }
}

XPostActivateFuncDef_(Render) {
  // XRender* xP = (XRender*) sP;
  /* The problem with this algo is that everybody between the newcomer and its destination 
   * has to shuffle with it, along with their metadata. That's gonna be a problem with 
   * bigger games. */
  // TODO re-sort the z-height fray here
  // First, get the Z-height of the newly activated component.
  // XRenderComp* newlyActivatedCompP = &( (XRenderComp*) sP->cF )[ changesP->newIdx ];
  
}

XPostDeactivateFuncDef_(Render) {
  // XRender* xP = (XRender*) sP;
  // TODO re-sort the z-height fray here
}

// Only get the render and window. Components' src & dst rects come from SCENE_START stimulus to XAction.
XGetShareFuncDef_(Render) {
  XRender *xP = (XRender*) sP;
  // Get renderer
  xP->guiP = (Gui*) mapGetNestedMapPElem(shareMPMP, GUI_GENE_TYPE, GUI_KEY_, NONMAP_POINTER);

  // Get source rect and rect offset maps. Give both a chance to run if we enter this block.
  xP->dstRectMP = mapGetNestedMapP(shareMPMP, DST_RECT);  
  xP->zHeightMP = mapGetNestedMapP(shareMPMP, Z_HEIGHT);  // Z-heights are shared since collision also needs them.
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
