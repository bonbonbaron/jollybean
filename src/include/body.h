#ifndef BODY_H
#define BODY_H

#include "interface.h"

typedef struct {
  U8     timeLeft; 
  U8     currIdx;
  Bln    repeat;
  U8    *timeA;     
  Rect_ *srcRectA;    // This is the array of animation rectangles. We draw FROM this part of the source image.
  Rect_ *srcRectP;  // Pointer to rectangle shared across multiple systems. We draw TO this part of the dest image.
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
