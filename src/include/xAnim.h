#ifndef SYS_Anim
#define SYS_Anim
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

typedef struct {
  Key key;
  AnimStrip *animStripP;
} KeyAnimStripPair;

typedef struct {
  Key nPairs;
  KeyAnimStripPair *kasPairA;
  Map *animMP;  // initialized with key anim-strip pair data at system start-up
} Animation;

typedef struct {
  S32        timeLeft;      // time left for current animation frame
  S8         incrDecrement; // increments or decrements animation frame depending on pingpong
  U8         currFrameIdx;  // keeps track of where we are in animation system
  AnimStrip *currStripP;    // current animation strip
  Rect_     *srcRectP;      // shared source rectangle to update; this pointer stays constant
  Rect_     *dstRectP;      // shared source rectangle to update; this pointer stays constant
} XAnimComp;

typedef struct {
  System system;
  Map        *offsetMP;   // derived inner share map from parent system
  Map        *srcRectMP;  // derived inner source rectangle map from parent system
  Map        *dstRectMP;  // derived inner source rectangle map from parent system
  Animation  *animSingletonF;  // fray of singleton animations
  Map        *animMPMP;   // maps entity to animation map, which in turn maps strip ID to anim strip
} XAnim;

extern System *sAnimP;
#endif
