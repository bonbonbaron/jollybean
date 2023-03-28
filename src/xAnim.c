#include "xAnim.h"
#include "data.h"

// Unused X functions
XIniSysFuncDef_(Anim) {
  XAnim *xP = (XAnim*) sP;
  Error e = mapNew(&xP->animMPMP, sizeof(Map*), xGetNComps(sP));
  if (!e) {
    e = frayNew((void**) &xP->animSingletonF, sizeof(Animation*), xGetNComps(sP));
  }
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

  // Make a SINGLETON map of animation strips for everybody who uses this particular one.
  if (!animP->animMP) {
    e = mapNew(&animP->animMP, sizeof(AnimStrip), animP->nPairs);
    if (!e) {
      KeyAnimStripPair *kasPairP = animP->kasPairA;
      KeyAnimStripPair *kasPairEndP = kasPairP + animP->nPairs;

      for (; !e && kasPairP < kasPairEndP; ++kasPairP) {
        e = mapSet(animP->animMP, kasPairP->key, kasPairP->animStripP);
      }
    }
    if (!e) {
      e = frayAdd(xP->animSingletonF, &animP, NULL);
    }
  }

  // Map from entity to the above animation map in the system's map of animation maps
  if (!e) {
    e = mapSet(xP->animMPMP, entity, &animP->animMP);
  }

  // Add entity's component to system as first available animation strip.
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
  mapDel(&xP->animMPMP);  // Don't delete inner maps; those are singletons properly deleted below.
  Animation **animPP = (Animation**) xP->animSingletonF;
  Animation **animEndPP = animPP + *frayGetFirstEmptyIdxP(xP->animSingletonF);
  for (; animPP < animEndPP; ++animPP) {
    mapDel(&(*animPP)->animMP);
  }
  frayDel((void**) &xP->animSingletonF);
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
    // Give each entity its source and destination rectangle pointers.
    if (!e) {
      cP->srcRectP = (Rect_*) mapGet(xP->srcRectMP, entity);
      if (!cP->srcRectP) {
        e = E_BAD_KEY;
      }
      if (!e) {
        cP->dstRectP = (Rect_*) mapGet(xP->dstRectMP, entity);
        if (!cP->dstRectP) {
          e = E_BAD_KEY;
        }
      }
      // Now change the W and H since originals reflect entire animation strip's dimensions.
      Map *animMP;
      if (!e) { 
        e = mapGetNestedMapP(xP->animMPMP, entity, &animMP);
      }
      if (!e && animMP) {
        AnimStrip *stripP = animMP->mapA;
        if (stripP) {
          cP->srcRectP->w = cP->dstRectP->w = stripP->frameA[0].rect.w;
          cP->srcRectP->h = cP->dstRectP->h = stripP->frameA[0].rect.h;
        }
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
    printf("animation got ANIMATE command\n");
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
      xActivateComponentByEntity(sP, msgP->attn);
    }
  }
  else if (msgP->cmd == UPDATE_RECT) {
    AnimStrip *animStripP, *animStripEndP;
    AnimFrame *frameP, *frameEndP;
    Map *animMP = NULL;
    RectOffset *offsetP = NULL;

    // Get animation subcomponent and offset to apply to all its frames.
    if (!e) {
      e = mapGetNestedMapP(xP->animMPMP, msgP->attn, &animMP);
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
        animStripEndP = animStripP + animMP->population;
        // Offset all the frames' rectangles in this strip to reflect their texture atlas offsets.
        for (; animStripP < animStripEndP; ++animStripP) {
          frameP = animStripP->frameA;
          frameEndP = frameP + animStripP->nFrames;
          printf("Entity %3d:\n", msgP->attn);
          for (; frameP < frameEndP; ++frameP) {
            printf("\tframe %d (before): {%3d, %3d}\n", frameP - animStripP->frameA, 
                frameP->rect.x, frameP->rect.y);
            frameP->rect.x += offsetP->x;
            frameP->rect.y += offsetP->y;
            printf("\tframe %d (after ): {%3d, %3d}\n", frameP - animStripP->frameA, 
                frameP->rect.x, frameP->rect.y);
          }
          // TODO remove
          animStripP->repeat = 0;
          animStripP->pingPong = 1;
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
  if (!e) {
    mapGetNestedMapP(shareMMP, DST_RECT, &xP->dstRectMP);
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
    if ((cP->timeLeft -= 9) <= 0) {  // TODO figure out time decrement
      // If we've reached the last frame, ask if this is a pingpong or repeating animation strip.
      if (cP->currFrameIdx == (cP->currStripP->nFrames - 1)) {
        // If this is a repeating animation strip, reset index, time left, and srcRectP.
        if (cP->currStripP->pingPong) {
          cP->incrDecrement *= -1;
          cP->currFrameIdx -= 1;
        }
        else if (cP->currStripP->repeat) {
          cP->currFrameIdx = 0;
        } 
        // Deactivate if not repeating or reversing order for pingpong.
        else {
          xQueueDeactivate(sP, cP);
          continue;
        }
      }
      // Otherwise, if this is a ping pong strip returning to the first frame, reverse directions.
      else if (cP->currFrameIdx == 0) {
        if (cP->currStripP->pingPong) {
          // If we just finished going backwards in a pingpong
          if (cP->incrDecrement < 0) {
            // Start over if we're repeating
            if (cP->currStripP->repeat) {
              cP->incrDecrement = 1;
              cP->currFrameIdx  = 1;
            }
            // Deactivate if not repeating pingpong.
            else {
              xQueueDeactivate(sP, cP);
              continue;
            }
          }
          else {
            cP->currFrameIdx += cP->incrDecrement;
          }
        }
        else {
          cP->currFrameIdx += cP->incrDecrement;
        }
      }
      else {
        cP->currFrameIdx += cP->incrDecrement;
      }
      // Advance the animation frame in whatever direction we're going.
      cP->timeLeft = cP->currStripP->frameA[cP->currFrameIdx].duration;
      *cP->srcRectP = cP->currStripP->frameA[cP->currFrameIdx].rect;
      cP->dstRectP->w = cP->srcRectP->w;
      cP->dstRectP->h = cP->srcRectP->h;
    }
  }

	return e;
}

//======================================================
// System definition
//======================================================
X_(Anim, ANIMATION, FLG_DONT_ADD_COMP);
