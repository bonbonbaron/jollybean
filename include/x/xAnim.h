#ifndef X_ANIM_H
#define X_ANIM_H
#include "data/strip.h"
#include "data/fray.h"
#include "x/x.h"
#include "interface/interface.h"
#include "jb.h"

#define ANIMATION_SUBTYPE (0x40)

typedef struct {
  Rect_ rect;
  U32 duration;
} AnimFrame;

typedef struct {
  U8  nFrames;
  U8  flags;
  Bln repeat;       
  Bln pingPong;       
  AnimFrame *frameA;
} AnimStrip;

typedef AnimStrip XAnimMutation;

typedef struct {
  S8         incrDecrement; // increments or decrements animation frame depending on pingpong
  U8         currFrameIdx;  // keeps track of where we are in animation system
  Rect_     *srcRectP;      // shared source rectangle to update; this pointer stays constant
  Rect_     *dstRectP;      // shared source rectangle to update; this pointer stays constant
  S32        timeLeft;      // time left for current animation frame
  AnimStrip  currStrip;     // current animation strip
} XAnimComp;

typedef struct {
  System system;
  Map        *offsetMP;   // derived inner share map from parent system
  Map        *srcRectMP;  // derived inner source rectangle map from parent system
  Map        *dstRectMP;  // derived inner source rectangle map from parent system
} XAnim;

extern System *sAnimP;
#endif
