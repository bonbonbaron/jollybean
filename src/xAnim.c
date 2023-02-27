#include "xAnim.h"
#include "x.h"

// Unused X functions
XIniSysFuncDefUnused_(Anim);
XPostprocessCompsDefUnused_(Anim);
XIniSubcompFuncDefUnused_(Anim);
XClrFuncDefUnused_(Anim);
// TODO get shared offset fray
// TODO process UPDATE_RECT messages with below "updateSrcRects" code block
// TODO 
// iniSubcomp
      animP = (Animation*) dataP;
      if (!animP->stripMP && animP->keyStripPairA && 
          ((nStrips = arrayGetNElems(animP->keyStripPairA)) > 0)) {
        // First, allocate animation's key-to-strip map.
        // The reason we do pointers to anim strip is preventing multi-offsetting their rects later.
        e = mapNew(&animP->stripMP, sizeof(AnimStrip*), nStrips);
        if (!e) {
          KeyStripPair *keyStripPairP = animP->keyStripPairA;
          KeyStripPair *keyStripPairEndP = keyStripPairP + arrayGetNElems(animP->keyStripPairA);
          for (; keyStripPairP < keyStripPairEndP; ++keyStripPairP) {
            mapSet(animP->stripMP, keyStripPairP->key, &keyStripPairP->animStripP);
          }
        }
      }

//clrFunc
  Animation **animPP = xRenderP->entity2Anim2AnimStripMPMP->mapA;
  Animation **animEndPP = animPP + arrayGetNElems(xRenderP->entity2Anim2AnimStripMPMP->mapA);
  // Although some holes exist in entity2Anim2AnimStripMPMP, we're safe here; 
  // those holes' stripMPs will look NULL to the loop below.
  for (; animPP < animEndPP; ++animPP) {
    if ((*animPP)->stripMP) {
      mapDel(&(*animPP)->stripMP);
    }
  }
  mapDel(&xRenderP->entity2Anim2AnimStripMPMP);


// updateSrcRects
Animation *animP;
AnimStrip *animStripP, *animStripEndP;
AnimFrame *frameP, *frameEndP;
if (!e) {
      animP = (Animation*) scoP->subcompA[TILEMAP_SUBCOMP_IDX];
    }
    // If image is animated, update all its individual frames' source rectangles.
    if (animP) {
      animStripP = animP->stripMP->mapA;
      // Since entities with identical images share animation strips, avoid multi-offsetting them.
      if (!(animStripP->flags & IS_OFFSET)) {
        animStripP->flags |= IS_OFFSET;
        animStripEndP = animStripP + arrayGetNElems(animP->stripMP->mapA);
        // Initialize an animated component's src rect to the first strip's first frame.
        c.srcRectP = &animStripP->frameA[0].rect;
        // Offset all the frames' rectangles in this strip to reflect their texture atlas offsets.
        for (; animStripP < animStripEndP; ++animStripP) {
          frameP = animStripP->frameA;
          frameEndP = frameP + arrayGetNElems(animStripP->frameA);
          for (; frameP < frameEndP; ++frameP) {
            frameP->rect.x += sortedRectA[cmP->sortedRectIdx].rect.x;
            frameP->rect.y += sortedRectA[cmP->sortedRectIdx].rect.y;
          }
        }
      }
    }

Error xAnimProcessMessage(System *sP, Message *msgP) {
  Error e = SUCCESS;
  // Get entity's map of switchable components.
  Map **mutationMPP = (Map**) mapGet(sP->mutationMPMP, msgP->attn);
  if (!mutationMPP)
    e = E_BAD_KEY;
  Map *mutationMP;
  if (!e) {
    mutationMP = *mutationMPP;
    if (!mutationMP) 
      e = E_BAD_KEY;
  }
  if (!e) {
    // Get entity's map of components to switch to.
    void *compP = xGetCompPByEntity(sP, msgP->attn);
    if (compP) {
      void *tmpP = mapGet(mutationMP, msgP->arg);
      if (tmpP)
        memcpy(compP, tmpP, sP->compSz - sizeof(Rect_*));  // don't copy over source rect pointer!
      else
        e = E_BAD_ARGS;
    }
  }
	return e;
}


// TODO: does this break if we call this BEFORE add
XGetShareFuncDef_(Anim) {
  if (!sP->cF)
    return E_BAD_ARGS;
	XAnimComp *cP = (XAnimComp*) sP->cF;
  XAnimComp *cStartP = cP;
	XAnimComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);

  for (; cP < cEndP; ++cP) {
    cP->srcRectP = (Rect_*) mapGet(shareMMP, xGetEntityByCompIdx(sP, cP - cStartP));
  }

  return SUCCESS;
}

//======================================================
// Anim activity
//======================================================
Error xAnimRun(System *sP) {
	Error e = SUCCESS;

	XAnimComp *cP = (XAnimComp*) sP->cF;
	XAnimComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);

  // loop through elements and act on them here
  for (; cP < cEndP; ++cP) {
    if (!(--cP->timeLeft)) {
      if (cP->repeat) {
        cP->timeLeft = cP->timeA[cP->currIdx = 0];
        *cP->srcRectP = cP->srcRectA[0];
      } else {
        xQueueDeactivate(sP, cP - cStartP);
      }
    }
  }

	return e;
}

//======================================================
// System definition
//======================================================
#define FLAGS_HERE (0)
X_(Anim, ANIMATION, FLAGS_HERE);
