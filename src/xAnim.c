#include "xAnim.h"

/* TODO
 *
 * \update src rcts in local animMPMP again
 * \prevent xAddComp with a new system flag
 * \create animatino map and make everybdoy grab from it
 * \clean up animation maps in ... TODO where's dataP coming from?
 */

// Unused X functions
XIniSysFuncDef_(Anim) {
  XAnim *xP = (XAnim*) sP;
  Error e = mapNew(&xP->animMPMP, sizeof(Map*), xGetNComps(sP));
  return e;
}

// sP, entity, subtype, dataP
XIniSubcompFuncDef_(Anim) {
  XAnim* xP = (XAnim*) sP;
  if (!sP || !entity || subtype != ANIMATION_SUBTYPE || !dataP || !xP->animMPMP) {
    return E_BAD_ARGS;
  }

  Error e = SUCCESS;

  Animation *animP = (Animation*) dataP;

  // Make a single map of animation strips for everybody who uses this particular one.
  if (!animP->animMP) {
    e = mapNew(&animP->animMP, sizeof(AnimStrip), animP->nPairs);
    if (!e) {
      KeyAnimStripPair *kasPairP = animP->kasPairA;
      KeyAnimStripPair *kasPairEndP = kasPairP + animP->nPairs;

      for (; !e && kasPairP < kasPairEndP; ++kasPairP) {
        e = mapSet(animP->animMP, kasPairP->key, kasPairP->animStripP);
      }
    }
  }

  // Map from entity to the above animation map in the system's map of animation maps
  if (!e) {
    e = mapSet(xP->animMPMP, entity, &animP->animMP);
  }

  if (!e) {
    Map *entityAnimMP;
    e = mapGetNestedMapP(xP->animMPMP, entity, &entityAnimMP);
    if (!e && entityAnimMP) {
      AnimStrip *stripP = (AnimStrip*) entityAnimMP->mapA;
      XAnimComp c = {
        .currFrameIdx = 0,
        .currStripP = stripP,  // point at first anim strip for now
        .incrDecrement = 1,
        .srcRectP = NULL,  // we'll set this in post-process function
        .timeLeft = stripP->frameA[0].duration
      };
      e = xAddComp(sP, entity, &c);
    }
  }
  return e;
}

XClrFuncDef_(Anim) {
  if (!sP) {
    return E_BAD_ARGS;
  }
  XAnim *xP = (XAnim*) sP;
  mapDel(&xP->animMPMP);
  return SUCCESS;
}

// You only need to give every component its source rect once.
XPostprocessCompsDef_(Anim) {
  XAnimComp *cP = (XAnimComp*) sP->cF;
  XAnimComp *cEndP = cP + *frayGetFirstEmptyIdxP(sP->cF);
  Entity entity;
  Error e = SUCCESS;
  XAnim *xP = (XAnim*) sP;

  // Give each component its source rectangle.
  for (; !e && cP < cEndP; ++cP) {
    entity = xGetEntityByVoidComponentPtr(sP, (void*) cP);
    if (!entity) {
      e = E_NULL_VAR;
    }
    if (!e) {
      cP->srcRectP = (Rect_*) mapGet(xP->srcRectMP, entity);
      if (!cP->srcRectP) {
        e = E_BAD_KEY;
      }
    }
  }

  // TODO get rid of this after you implement action system
  if (!e) {
    frayActivateAll(sP->cF);
  }

  return SUCCESS;
}

Error xAnimProcessMessage(System *sP, Message *msgP) {
  Error e = SUCCESS;
  XAnim *xP = (XAnim*) sP;
  /* When the render system makes a texture atlas, the animation frames' rectangles no longer
     have the correct XY coordinates. We have to offset them here to ensure we draw from the 
     right place in the atlas. */
  // Put the most commonly used logic first.
  if (msgP->cmd == ANIMATE) {
    AnimStrip *animStripP = NULL;
    XAnimComp *cP = NULL;
    e = mapGetNestedMapPElem(xP->animMPMP, msgP->attn, msgP->arg, (void**) &animStripP);
    if (!e) {
      cP = (XAnimComp*) xGetCompPByEntity(sP, msgP->attn);
      e = cP ? SUCCESS : E_NULL_VAR;
    }
    if (!e) {
      cP->currStripP = animStripP;
      cP->currFrameIdx = 0;
      cP->incrDecrement = 1;
    }
  }
  else if (msgP->cmd == UPDATE_RECT) {
    AnimStrip *animStripP, *animStripEndP;
    AnimFrame *frameP, *frameEndP;
    Map *animMP = NULL;
    RectOffset *offsetP = NULL;

    // Get animation subcomponent and offset to apply to all its frames.
    if (!e) {
      animMP = mapGet(xP->animMPMP, msgP->attn);
    }
    if (!e && animMP) {
      offsetP = (RectOffset*) mapGet(xP->offsetMP, msgP->attn);
    }

    // Update all of this entity's animation frames' XY coordinates by the offset.
    if (!e && animMP && offsetP) {
      animStripP = (AnimStrip*) animMP->mapA;
      // Avoid offsetting any animation more than once.
      if (!(animStripP->flags & IS_OFFSET)) {
        animStripP->flags |= IS_OFFSET;
        animStripEndP = animStripP + arrayGetNElems(animMP->mapA);
        // Offset all the frames' rectangles in this strip to reflect their texture atlas offsets.
        for (; animStripP < animStripEndP; ++animStripP) {
          frameP = animStripP->frameA;
          frameEndP = frameP + arrayGetNElems(animStripP->frameA);
          for (; frameP < frameEndP; ++frameP) {
            frameP->rect.x += offsetP->x;
            frameP->rect.y += offsetP->y;
          }
        }
      }
    }
  }
  else {
    e = E_MAILBOX_BAD_RECIPIENT;
  }
	return e;
}

XGetShareFuncDef_(Anim) {
  if (!sP) {
    return E_BAD_ARGS;
  }
  // Get shared inner maps of resources we need (offsets and source rects)
  XAnim *xP = (XAnim*) sP;
  Error e = mapGetNestedMapP(shareMMP, RECT_OFFSET, &xP->offsetMP);
  if (!e) {
    mapGetNestedMapP(shareMMP, SRC_RECT, &xP->srcRectMP);
  }
  return e;
}

//======================================================
// Anim activity
//======================================================
Error xAnimRun(System *sP) {
	Error e = SUCCESS;

	XAnimComp *cP = (XAnimComp*) sP->cF;
	XAnimComp *cEndP = cP + frayGetFirstInactiveIdx(sP->cF);

  // Animation
  for (; cP < cEndP; ++cP) {
    if (!(--cP->timeLeft)) {
      // If we've reached the last frame, ask if this is a pingpong or repeating animation strip.
      if (cP->currFrameIdx == (cP->currStripP->nFrames - 1)) {
        // If this is a repeating animation strip, reset index, time left, and srcRectP.
        if (cP->currStripP->repeat) {
          cP->currFrameIdx = 0;
          cP->timeLeft = cP->currStripP->frameA[0].duration;
        } 
        // Otherwise, if this is a pingpong animation strip, reverse directions.
        else if (cP->currStripP->pingPong) {
          cP->incrDecrement *= -1;
        }
        else {
          xQueueDeactivate(sP, cP);
          continue;
        }
      }
      // Otherwise, if this is a ping pong strip returning to the first frame, reverse directions.
      else if (cP->currFrameIdx == 0 && cP->currStripP->pingPong && cP->currStripP->repeat) {
        cP->incrDecrement *= -1;
      }
      else {
        xQueueDeactivate(sP, cP);
        continue;
      }
    }
    // Advance the animation frame in whatever direction we're going.
    cP->currFrameIdx += cP->incrDecrement;
    *cP->srcRectP = cP->currStripP->frameA[cP->currFrameIdx].rect;
  }

	return e;
}

//======================================================
// System definition
//======================================================
X_(Anim, ANIMATION, FLG_DONT_ADD_COMP);
