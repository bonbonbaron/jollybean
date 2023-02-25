#ifndef BODY_H
#define BODY_H

#include "interface.h"

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
  KeyStripPair *keyStripPairA;  // maps keys to animations
  Map          *stripMP;  // animStripMappingA gets converted to a map on startup
} Animation;

typedef struct {
  Entity entity;   // Collider
  Key    type;     // Colliding elements react according to each other's type.
  Key    layer;
  Key    rectIdx;  // Rect index of Collidee (i swear i don't talk like this)
} Collision;

typedef struct {
  U8 nCollisionTypes;
  U8 *colissionA;
} CollisionPalette;

typedef struct {
  Animation animation;
  Collision collision;
  Colormap colormap;
} Graybody;

typedef struct {
  ColorPalette color;
  CollisionPalette collision;
} Palette;

typedef struct {
  Graybody graybody;
  Palette palette;
} Body;

#endif
