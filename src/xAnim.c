#include "xAnim.h"
#include "data.h"

/* TODO
 * Kaizen: 
 *   I keep wondering what to do with these system functions. In my system template, 
 *
 *   At least populate functions with argument error-checking.
 * 
 *   I should describe what each function does, provide a small example, and even give a comment-
 *     able, finished line for an unused function if necessary.
 *  
 *   Finally, I should outline a general workflow for component- vs. subcomponent-based systems.
 *
 *   Standardize your sprint flow and figure out what works best. Measure it for science.
 *     For now, this is what I'm thinking:
 *     Day 1: Goals and Design. They intermingle enough to coincide.
 *            Make design concrete enough for rapid development.
 *     Day 2: Dev, build, and test in a tight loop, one function at a time.
 *
 */
// Unused X functions
XIniSysFuncDef_(Anim) {
  unused_(sParamsP);
  XAnim *xAnimP = (XAnim*) sP;
  Error e = frayNew((void**) &xAnimP->animKeyPairSetPF, sizeof(KeyStripPair), xGetNComps(sP));
  if (!e) {
    e = mapNew(&xAnimP->animMPMP, sizeof(Map*), xGetNComps(sP));
  }
  return e;
}

#define ANIMATION_SUBCOMP_IDX getSubcompIdx_(0x40)
XPostprocessCompsDef_(Anim) {
  if (!sP) {
    return E_BAD_ARGS;
  }

  Error e = SUCCESS;

  XAnim *xP = (XAnim*) sP;

  AnimKeyPairSet **setPP = xP->animKeyPairSetPF;
  AnimKeyPairSet **setEndPP = setPP + *frayGetFirstEmptyIdxP(xP->animKeyPairSetPF);
  KeyStripPair   *kvPairP, *kvPairEndP;

  // Initialize all submaps before placing them into (outer) entity-to-animation map.
  for (; !e && setPP < setEndPP; ++setPP) {
    e = mapNew(&(*setPP)->animMP, sizeof(AnimStrip), (*setPP)->nKeyStripPairs);
    if (!e) {
      // Set all the keys in the key-pair set to their values in the inner animation map.
      kvPairP = (*setPP)->keyStripPairA;
      kvPairEndP = kvPairP + (*setPP)->nKeyStripPairs;
      for (; !e && kvPairP < kvPairEndP; ++kvPairP) {
        e = mapSet((*setPP)->animMP, kvPairP->key, kvPairP->animStripP);
      }
    }
  }

  // Put all the animation maps into the entity-to-animation map of maps.
  if (!e) {
    SubcompOwner *scoP = sP->subcompOwnerMP->mapA;
    SubcompOwner *scoEndP = scoP + arrayGetNElems(sP->subcompOwnerMP->mapA);
    for (; !e && scoP < scoEndP; ++scoP) {
      setPP = scoP->subcompA[ANIMATION_SUBCOMP_IDX];
      e = mapSet(xP->animMPMP, scoP->owner, &(*setPP)->animMP);
    }
  }

  frayDel((void**) &xP->animKeyPairSetPF);

  return e;
}

XIniSubcompFuncDef_(Anim) {
	if (!sP || !entity || !dataP || !subtype) {
		return E_BAD_ARGS;
  }

  AnimKeyPairSet *animKeyPairSetP;

  XAnim *xP = (XAnim*) sP;
  Error e = SUCCESS;

  // We use type to determine system.
  // Then we use subtype to determine what it does with it. 
  if (subtype == ANIMATION) {
    animKeyPairSetP = (AnimKeyPairSet*) dataP;
    if (!(animKeyPairSetP->state & INITIALIZED)) {
      animKeyPairSetP->state |= INITIALIZED;
      e = frayAdd(xP->animKeyPairSetPF, (void*) &animKeyPairSetP, NULL);
    }
  }

  return e;

}

XClrFuncDef_(Anim) {
  if (!sP) {
    return E_BAD_ARGS;
  }

  XAnim *xP = (XAnim*) sP;

  frayDel((void**) &xP->animKeyPairSetPF);

  // Delete submaps of animations
  AnimKeyPairSet **setPP = xP->animKeyPairSetPF;
  AnimKeyPairSet **setEndPP = setPP + *frayGetFirstEmptyIdxP(xP->animKeyPairSetPF);
  for (; setPP < setEndPP; ++setPP) {
    mapDel(&(*setPP)->animMP);
  }

  mapDel(&xP->animMPMP);

  return SUCCESS;
}

Error xAnimProcessMessage(System *sP, Message *msgP) {
  Error e = SUCCESS;
  // Handle request to offset animation frames' coordinates (e.g. for new positions in texture atlas)
  if (msgP->cmd == ANIMATE) {
    AnimStrip *stripP = NULL;
    XAnimComp *cP = NULL;
    e = mapGetNestedMapPElem(sP->mutationMPMP, msgP->attn, msgP->arg, (void**) &stripP);
    if (!e) {
      cP = xGetCompPByEntity(sP, msgP->attn);
    }
    if (!e && cP) {
      cP->currFrameIdx = 0;
      cP->currStripP = stripP;
      cP->incrDecrement = 1;
      cP->timeLeft = stripP->frameA[0].duration;
      *cP->srcRectP = stripP->frameA[0].rect;
    }
  }
  /* When the render system makes a texture atlas, the animation frames' rectangles no longer
     have the correct XY coordinates. We have to offset them here to ensure we draw from the 
     right place in the atlas. */
  else if (msgP->cmd == UPDATE_RECT) {
    XAnim *xP = (XAnim*) sP;
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

  XAnim *xP = (XAnim*) sP;

  // Get shared inner maps of resources we need (offsets and source rects)
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
#define FLAGS_HERE (0)
X_(Anim, ANIMATION, FLAGS_HERE);
