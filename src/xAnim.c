#include "xAnim.h"
#include "x.h"

/* TODO
 * Kaizen: 
 *   I keep wondering what to do with these system functions. In my system template, 
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
XIniSysFuncDefUnused_(Anim);
XPostprocessCompsDefUnused_(Anim);
#define XIniSubcompFuncDef_(name_)  Error x##name_##IniSubcomp(System *sP, const Entity entity, const Key subtype, void *dataP)
/*
 * Render uses subcomp ini to put stuff into frays for later processing,
 * But render needs to post-process stripsets into a texture array; anim doesn't.
 * So instead, we're going to knock out the component creation in the sub comp one.
 *
 * Now, we only want to make unique animations once. 
 * Everybody who uses them points at them. 
 *
 * GOAL
 * ====
 * A) Make a system-level map of unique animations, which in turn are maps of animation strips.
 * B) (A)'s maps are keyed by entities, and the submaps are keyed by animation triggers.
 * C) Create unique animations only once.
 *
 * DESIGN
 * ======
 * A) In sys ini, allocate the anim key pair fray by ncomps.
 *    In subcomp ini, count the number of unique animations. 
 *    In postprocessing, then we'll make our map of system-level map of animations.
 * B) First look in (C) for completed animation.
 *    If no animation exists, allocate its map with nKeyStripPairs and sizeof(AnimStrip).
 *    Then set all its elements with 
 * C) We'll put a pointer in the source data to indicate status.
 *    Put a state in it to indicate added-to-system-fray status.
 */

XIniSubcompFuncDef_(Anim) {
  // updateSrcRects
  AnimStrip *animStripP, *animStripEndP;
  AnimFrame *frameP, *frameEndP;
  Animation *animP = (Animation*) scoP->subcompA[ANIMATION_SUBCOMP_IDX];
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
  // -------------------- OLD CODE ---------------------
// iniSubcomp
  Animation *animP = (Animation*) dataP;
  U32 nAnimStrips;
  Error e = SUCCESS;
  // If anim strip map isn't yet initialized, initialize it.
  if (!animP->stripMP && animP->keyStripPairA && 
      ((nAnimStrips = arrayGetNElems(animP->keyStripPairA)) > 0)) {
    // First, allocate animation's key-to-strip map.
    // The reason we do pointers to anim strip is preventing multi-offsetting their rects later.
    e = mapNew(&animP->stripMP, sizeof(AnimStrip*), nAnimStrips);
    if (!e) {
      KeyStripPair *keyStripPairP = animP->keyStripPairA;
      KeyStripPair *keyStripPairEndP = keyStripPairP + arrayGetNElems(animP->keyStripPairA);
      for (; keyStripPairP < keyStripPairEndP; ++keyStripPairP) {
        mapSet(animP->stripMP, keyStripPairP->key, &keyStripPairP->animStripP);
      }
    }
  }
  return e;
}

XClrFuncDefUnused_(Anim);
// TODO get shared offset map
// TODO get shared srcrect map
// TODO process UPDATE_RECT messages with below "updateSrcRects" code block

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
