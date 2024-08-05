#ifndef INTERFACE_H
#define INTERFACE_H
#include "x.h"
#include "strip.h"

// Choose your interface!!
#ifndef USE_HEADLESS_INTERFACE
#define USE_SDL_INTERFACE  // default to this when we're not specifying headless for now
#endif
#ifdef USE_SDL_INTERFACE
#include "SDL_interface.h"
#else
#ifdef USE_HEADLESS_INTERFACE // Use if you don't have any graphical capabilities-- most likely you're unit testing if you're doing this.
#include "headless_interface.h"
#endif
#endif

#define USE_PTHREAD_ 1
#if USE_PTHREAD_
#define MULTITHREADED_
#endif

// ==============
// Rendering
// ==============


typedef enum {
  BTN_PRESSED_a      = 0x00000001,
  BTN_PRESSED_b      = 0x00000002,
  BTN_PRESSED_c      = 0x00000004,
  BTN_PRESSED_d      = 0x00000008,
  BTN_PRESSED_e      = 0x00000010,
  BTN_PRESSED_f      = 0x00000020,
  BTN_PRESSED_g      = 0x00000040,
  BTN_PRESSED_h      = 0x00000080,
  BTN_PRESSED_i      = 0x00000100,
  BTN_PRESSED_j      = 0x00000200,
  BTN_PRESSED_k      = 0x00000400,
  BTN_PRESSED_l      = 0x00000800,
  BTN_PRESSED_m      = 0x00001000,
  BTN_PRESSED_n      = 0x00002000,
  BTN_PRESSED_o      = 0x00004000,
  BTN_PRESSED_p      = 0x00008000,
  BTN_PRESSED_q      = 0x00010000,
  BTN_PRESSED_r      = 0x00020000,
  BTN_PRESSED_s      = 0x00040000,
  BTN_PRESSED_t      = 0x00080000,
  BTN_PRESSED_u      = 0x00100000,
  BTN_PRESSED_v      = 0x00200000,
  BTN_PRESSED_w      = 0x00400000,
  BTN_PRESSED_x      = 0x00800000,
  BTN_PRESSED_y      = 0x01000000,
  BTN_PRESSED_z      = 0x02000000,
  BTN_PRESSED_SPACE  = 0x04000000,
  BTN_PRESSED_LSHIFT = 0x08000000,
  BTN_PRESSED_ESCAPE = 0x10000000,
  // TODO decide what the last three bits should be. Include Tab and Enter,, perhaps?
} ButtonPressed;

typedef struct {
  S32 x;
  S32 y;
} Position;  // used for placing visible entities around the level

typedef Position RectOffset;  // used for offsetting animation rectangles after making texture atlas
#define IS_OFFSET (0x01)

typedef U8 ColormapIdx;
#define N_COLORS_SUPPORTED_MAX_ (16)
typedef struct {
  StripDataS *sdP;  // this element MUST come first in a media (hence inflatable) gene for casting
  U8 state;         // prevents copies of this from being inflated
  U8 bpp;
  U16 w, h, pitch;  // in pixel units; determine actual step size by pixel format
} Colormap;     

typedef struct {
  U8 nColors;
  Color_ *colorA;
} ColorPalette;

typedef struct {
  U8 state;  // prevents copies of images from being added to texture atlas
  Key sortedRectIdx;  // Index of sorted rectangle so you can adjust src rect's XY offset in atlas
  Colormap* cmP;
  ColorPalette* cpP;
} Image;

// GUI
typedef enum {
  UNPAUSED,
  PAUSED,
  QUIT
} GuiState;
  
typedef struct {
  Entity     context;   // this is what input events affect, e.g. main character, menu, text, etc.
  Entity     cameraEntity;
  U8         buttonChanged;
  GuiState   state;
  U32        buttonsPressed;
  Renderer_ *rendererP;
  Window_   *windowP;
  Rect_     *cameraRectP;
} Gui;

Gui* guiNew();
void  guiDel(Gui **guiPP);
void guiProcessEvents(Gui *guiP);

// Surface
Surface_* surfaceNew( void *pixelDataA, U32 w, U32 h);
void surfaceDel(Surface_ **surfacePP);

// Atlas Palette
Color_* getColorPalette( Surface_ *surfaceP );
U32 getNColors( Surface_ *surfaceP );

// Texture
Texture_* textureNew( Renderer_ *rendererP, Surface_ *surfaceP);
void textureDel(Texture_ **texturePP);
void textureSetAlpha(Texture_ *textureP);

// Screen
void clearScreen(Renderer_ *rendererP);

// ==============
// Multithreading
// ==============
#if USE_PTHREAD_
#include <pthread.h>
#define N_CORES (4)  // TODO automate this via cmake later
#define threadIni_(threadP_, argP_) pthread_create(threadP_, NULL, (DummyFuncCast) _mtGenericLoop, (void*) argP_)
#define threadJoin_(threadP_) pthread_join(threadP_, NULL)

typedef pthread_t Thread;
typedef void (*CriticalFunc)(void *argP);
typedef struct {
  U32 startIdx;
  U32 nElemsToProcess;
  CriticalFunc funcP;
  void *array;  // threads need to receive a copy of the pointer to data to operate on
} ThreadFuncArg;

typedef void* (*ThreadFunc)(ThreadFuncArg*);
typedef void* (*DummyFuncCast)(void*);  // trick compiler into allowing ThreadFuncs in pthread_create

#define multithread_(funcP_, array_) multiThread((CriticalFunc) funcP_, array_)
void multiThread( CriticalFunc funcP, void *_array);

#endif // #if USE_PTHREAD_

#endif  // #ifndef INTERFACE_H
