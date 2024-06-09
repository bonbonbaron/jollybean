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

void copy ( Renderer* rendererP, Texture* srcTextureP, Rect* srcRectP, Rect* dstRectP );
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
#define KEY_a_ KEY_A
#define KEY_b_ KEY_B
#define KEY_c_ KEY_C
#define KEY_d_ KEY_D
#define KEY_e_ KEY_E
#define KEY_f_ KEY_F
#define KEY_g_ KEY_G
#define KEY_h_ KEY_H
#define KEY_i_ KEY_I
#define KEY_j_ KEY_J
#define KEY_k_ KEY_K
#define KEY_l_ KEY_L
#define KEY_m_ KEY_M
#define KEY_n_ KEY_N
#define KEY_o_ KEY_O
#define KEY_p_ KEY_P
#define KEY_q_ KEY_Q
#define KEY_r_ KEY_R
#define KEY_s_ KEY_S
#define KEY_t_ KEY_T
#define KEY_u_ KEY_U
#define KEY_v_ KEY_V
#define KEY_w_ KEY_W
#define KEY_x_ KEY_X
#define KEY_y_ KEY_Y
#define KEY_z_ KEY_Z
#define KEY_SPACE_ KEY_SPACE
#define KEY_LSHIFT_  KEY_LEFTSHIFT // undefined for our purposes
#define KEY_ESCAPE_  KEY_ESC  // code for Esc

#endif  // #ifndef HEADLESS_INTERFACE_H


