#ifndef SYS_Anim
#define SYS_Anim
#include "jb.h"

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
} KeyStripPair;

typedef struct {
  U8  state;  // Use this to tell subcomp ini when an animation has been counted.
  Key nKeyStripPairs;           // number of pairs of frame trigger-to-animation-strip mappings
  KeyStripPair *keyStripPairA;  // maps keys to animations
  Map *animMP;
} AnimKeyPairSet;

typedef struct {
  U32        timeLeft;      // time left for current animation frame
  S8         incrDecrement; // increments or decrements animation frame depending on pingpong
  U16        currFrameIdx;  // keeps track of where we are in animation system
  AnimStrip *currStripP;    // current animation strip
  Rect_     *srcRectP;      // shared source rectangle to update; this pointer stays constant
} XAnimComp;

// Images
Error xAnimIniS(System *sP, void *sParamsP);
Error xAnimProcessMessage(System *sP, Message *msgP);

typedef struct {
  System system;
  Map        *animMPMP;
  AnimKeyPairSet **animKeyPairSetPF;
  Map        *offsetMP;   // derived inner share map from parent system
  Map        *srcRectMP;  // derived inner source rectangle map from parent system
} XAnim;

extern System *sAnimP;
#endif
