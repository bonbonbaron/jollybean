#ifndef SYS_Anim
#define SYS_Anim
#include "jb.h"

typedef struct {
  Rect_ rect;
  U32 duration;
} AnimFrame;

typedef struct {
  U8  nFrames;
  Bln flags;
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
  Key nKeyStripPairs;
  KeyStripPair *keyStripPairA;  // maps keys to animations
} AnimKeyPairSet;

typedef struct {
  Map          *stripMP;  // animStripMappingA gets converted to a map on startup
} Animation;

typedef Animation XAnimComp;

// Images
Error xAnimIniS(System *sP, void *sParamsP);
Error xAnimProcessMessage(System *sP, Message *msgP);

typedef struct {
  System system;
  AnimKeyPairSet *animKeyPairSetF;
} XAnim;

extern System *sAnimP;
#endif
