#include "x/xAnim.h"

// Unused X functions
XIniSysFuncDefUnused_(Anim);
XConsumeGeneFuncDefUnused_(Anim);
XPostprocessCompsDefUnused_(Anim);
XPostActivateFuncDefUnused_(Anim);
XPostDeactivateFuncDefUnused_(Anim);

XProcMsgFuncDef_(Anim) {
  XAnim *xP = (XAnim*) sP;
  /* When the render system makes a texture atlas, the animation frames' rectangles no longer
     have the correct XY coordinates. We have to offset them here to ensure we draw from the 
     right place in the atlas. */
  // Put the most commonly used logic first.
  if (msgP->cmd == ANIMATE) {
    xMutateComponent(sP, msgP->attn, msgP->arg);
  }
  else if (msgP->cmd == UPDATE_RECT) {
    AnimStrip *animStripP, *animStripEndP;
    AnimFrame *frameP, *frameEndP;
    // Get animation subcomponent and offset to apply to all its frames.
    Map* animMP = mapGetNestedMapP(xP->system.mutationMPMP, msgP->attn);
    assert(animMP);
    RectOffset* offsetP = (RectOffset*) mapGet(xP->offsetMP, msgP->attn);
    // Update all of this entity's animation frames' XY coordinates by the offset.
    assert(animMP && offsetP);
    animStripP = (AnimStrip*) animMP->mapA;
    // Avoid offsetting any animation more than once.
    if (!(animStripP->flags & IS_OFFSET)) {
      animStripP->flags |= IS_OFFSET;
      animStripEndP = animStripP + animMP->population;
      // Offset all the frames' rectangles in this strip to reflect their texture atlas offsets.
      for (; animStripP < animStripEndP; ++animStripP) {
        frameP = animStripP->frameA;
        frameEndP = frameP + animStripP->nFrames;
        for (; frameP < frameEndP; ++frameP) {
          frameP->rect.x += offsetP->x;
          frameP->rect.y += offsetP->y;
        }
      }
    }
  }
  else {
    assert(0);  // The above should be the only two message types sent to XAnim.
  }
}

// TODO move this to postprocess()
#if 0
XGetShareFuncDef_(Anim) {
  assert(sP);
  // Get shared inner maps of resources we need (offsets and source rects)
  XAnim *xP = (XAnim*) sP;
  xP->offsetMP = mapGetNestedMapP(shareMPMP, RECT_OFFSET);
  xP->srcRectMP = mapGetNestedMapP(shareMPMP, SRC_RECT);
  xP->dstRectMP = mapGetNestedMapP(shareMPMP, DST_RECT);
  assert(xP->offsetMP);
  assert(xP->srcRectMP);
  assert(xP->dstRectMP);
  // Give each entity its source and dest rectangles.
  XAnimComp *cP = (XAnimComp*) sP->cF;
  XAnimComp *cEndP = cP + *_frayGetFirstEmptyIdxP(sP->cF);
  for (Entity entity; cP < cEndP; ++cP) {
    entity = xGetEntityByVoidComponentPtr(sP, (void*) cP);
    assert (entity);
    cP->srcRectP = (Rect_*) mapGet(xP->srcRectMP, entity);
    cP->dstRectP = (Rect_*) mapGet(xP->dstRectMP, entity);
    assert (cP->srcRectP);
    assert (cP->dstRectP);
  }
}
#endif

XPostMutateFuncDef_(Anim) {
  assert(sP && cP);
  XAnimComp *_cP = (XAnimComp*) cP;
  _cP->currFrameIdx   = 0;
  _cP->incrDecrement  = 1;  // assume we're going to start off animating forward
  // TODO take advantage of the anim's flag to decide whether to anchor the changed image to a side, corner, or center.
  assert( _cP->srcRectP != NULL );  // Make sure components can see the shared source rectangles.
  _cP->srcRectP->x    = _cP->currStrip.frameA[0].rect.x;  // mailbox should get "offset" changes to this beforehand
  _cP->srcRectP->y    = _cP->currStrip.frameA[0].rect.y;  // mailbox should get "offset" changes to this beforehand
  _cP->srcRectP->w    = _cP->dstRectP->w = _cP->currStrip.frameA[0].rect.w;
  _cP->srcRectP->h    = _cP->dstRectP->h = _cP->currStrip.frameA[0].rect.h;
  _cP->timeLeft       = _cP->currStrip.frameA[0].duration;
}

//======================================================
// Anim activity
//======================================================
void xAnimRun(System *sP) {
	XAnimComp *cP = (XAnimComp*) sP->cF;
	XAnimComp *cEndP = cP + _frayGetFirstInactiveIdx(sP->cF);

  // Animation
  for (; cP < cEndP; ++cP) {
    if ((cP->timeLeft -= 1) <= 0) {  // TODO figure out time decrement
      // If we've reached the last frame, ask if this is a pingpong or repeating animation strip.
      if (cP->currFrameIdx == (cP->currStrip.nFrames - 1)) {
        // If this is a repeating animation strip, reset index, time left, and srcRectP.
        if (cP->currStrip.pingPong) {
          cP->incrDecrement *= -1;
          cP->currFrameIdx -= 1;
        }
        else if (cP->currStrip.repeat) {
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
        if (cP->currStrip.pingPong) {
          // If we just finished going backwards in a pingpong
          if (cP->incrDecrement < 0) {
            // Start over if we're repeating
            cP->incrDecrement = 1;
            if (cP->currStrip.repeat) {
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
      cP->timeLeft = cP->currStrip.frameA[cP->currFrameIdx].duration;
      *cP->srcRectP = cP->currStrip.frameA[cP->currFrameIdx].rect;
      cP->dstRectP->w = cP->srcRectP->w;
      cP->dstRectP->h = cP->srcRectP->h;
    }  // if time's run out, animate  // TODO this should be easy to move to its own function!!! xAnimRun() ought to do nothing, ought to be a completely mail-driven system
       // TODO  but think through how you'll pause animations... in fact, component placement might mean nothing in the animation world
  }  // for each active component
}  // xAnimRun()

//======================================================
// System definition
//======================================================
X_(Anim, ANIMATION, currStrip, 0);
