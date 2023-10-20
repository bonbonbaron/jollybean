#ifndef HEADLESS_INTERFACE_H
#define HEADLESS_INTERFACE_H
#include "common.h"

// ==============
// Rendering
// ==============

#define defineDumb_( x_ ) \
  typedef struct x_ { \
    void* blah; \
  } x_; 

// Define our own structures here.
typedef struct Color {
  U8 r, g, b , a;
} Color;

typedef struct Rect {
  S32 x, y, w, h;
} Rect;

typedef struct Surface {
  S32 w, h;
  Color *pixelA;
} Surface;

typedef struct Texture {
  S32 w, h;
  Color *pixelA;
} Texture;

typedef struct Renderer {
  S32 w, h;
  Texture *dstTextureP;
} Renderer;

typedef struct Window {
  S32 w, h;
} Window;

defineDumb_ ( Palette );

// newHeadlessWindow  ... what was i doing with this?

Error copy ( Renderer* rendererP, Texture* srcTextureP, Rect* srcRectP, Rect* dstRectP );
void present ( Renderer* rendererP );

#define Color_ Color
#define Rect_ Rect
#define Surface_ Surface
#define Texture_ Texture
#define Window_ Window
#define WindowTypeSz_ sizeof(Window)
#define Renderer_ struct Renderer
#define RendererTypeSz_ sizeof(Renderer)
#define Palette_ Palette
#define copy_ copy
#define present_ present
// Control
defineDumb_( Event_ );
#define QUIT_ 123
#define KEY_a_ 'a'
#define KEY_b_ 'b'
#define KEY_c_ 'c'
#define KEY_d_ 'd'
#define KEY_e_ 'e'
#define KEY_f_ 'f'
#define KEY_g_ 'g'
#define KEY_h_ 'h'
#define KEY_i_ 'i'
#define KEY_j_ 'j'
#define KEY_k_ 'k'
#define KEY_l_ 'l'
#define KEY_m_ 'm'
#define KEY_n_ 'n'
#define KEY_o_ 'o'
#define KEY_p_ 'p'
#define KEY_q_ 'q'
#define KEY_r_ 'r'
#define KEY_s_ 's'
#define KEY_t_ 't'
#define KEY_u_ 'u'
#define KEY_v_ 'v'
#define KEY_w_ 'w'
#define KEY_x_ 'x'
#define KEY_y_ 'y'
#define KEY_z_ 'z'
#define KEY_SPACE_ ' '
#define KEY_LSHIFT_  154124 // undefined for our purposes
#define KEY_ESCAPE_  27  // code for Esc

#endif  // #ifndef HEADLESS_INTERFACE_H


