#ifndef IMAGE_H
#define IMAGE_H
#include "gene.h"
#include "data/strip.h"
#include "interface/interface.h"  // maybe you can grab color definition this way

// typedef struct Color_ Color_;  // pray to the compiler that this works LOL

//typedef U32 Color;

typedef struct Colormap {
  StripDataS *sdP;  // this element MUST come first in a media (hence inflatable) gene for casting
  U8 state;         // prevents copies of this from being inflated
  U8 bpp;
  U16 w, h, pitch;  // in pixel units; determine actual step size by pixel format
} Colormap;     

typedef struct ColorPalette{
  U8 nColors;
  Color_ *colorA;   // TODO see if we can get away without Color_ for now. That dependency to SDL sucks.
} ColorPalette;

typedef struct Image {
  GeneHdr hdr;  // this is the top-level gene for renderer system; anim and coll counterparts differentiate
  U8 state;  // prevents copies of images from being added to texture atlas
  Key sortedRectIdx;  // Index of sorted rectangle so you can adjust src rect's XY offset in atlas
  // We don't store raw image data in this struct. It actually winds up in a texture atlas in xRender.
  Colormap* cmP;
  ColorPalette* cpP;  
} Image;

#endif // IMAGE_H
