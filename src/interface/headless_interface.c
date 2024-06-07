#define USE_HEADLESS_INTERFACE
#include "interface.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>


#define KEYFILE "/dev/input/event0" // Use your keyboard's event file
#define VAL_KEY_UP (0)
#define VAL_KEY_DOWN (1)
#define VAL_KEY_REPEAT (2)

// TODO abstract away all SDL-specific stuff to an SDL-based interface header, 
//      which in turn will be selected in interface.h.
/**********************
 * Rendering
 *********************/
static struct termios tty_attr;
static FILE *file = NULL;

Gui* guiNew() {
  Gui* guiP = jbAlloc(sizeof(Gui), 1);
  assert(guiP);
  memset(*guiPP, 0, sizeof(Gui));
  guiP->windowP = jbAlloc(  sizeof(Window), 1 );
  assert(guiP->windowP);

  // Init renderer
  guiP->rendererP = jbAlloc(sizeof(Window), 1 );
  assert(guiP->rendererP);
  (*guiPP)->rendererP->dstTextureP = arrayNew(sizeof(Texture_), 1 );

  // open up the keyboard file in read binary mode so we can accept key presses.
  file = fopen(KEYFILE, "rb"); 
  assert(file);

  // Set terminal settings: 
  struct termios new_tty_attr = tty_attr;
  new_tty_attr.c_lflag &= ~(ICANON | ECHO); // turn off canonic mode and echo

  // Apply new terminal settings
  tcsetattr(STDIN_FILENO, TCSANOW, &new_tty_attr);

  return guiP;
}

void guiDel(Gui **guiPP) {
  if (guiPP && *guiPP) {
    jbFree((void**) &(*guiPP)->rendererP);
    jbFree((void**) &(*guiPP)->windowP);
    jbFree((void**) guiPP);
  }
  if (file) {
    fclose(file);
  }
}

static Color_* _getPixelP( Texture_ *textureP, U32 x, U32 y ) {
  assert( textureP && textureP->pixelA &&
      ( textureP->pixelA + ( textureP->w * textureP->h ) + x ) < ( textureP->pixelA + ( arrayGetNElems( textureP->pixelA ) ) ) );
  return textureP->pixelA + ( textureP->w * textureP->h ) + x;
}

// Since the headless renderer isn't actually going to display anything, we don't care about scaling.
void copy ( Renderer* rendererP, Texture_* srcTextureP, Rect_* srcRectP, Rect_* dstRectP ) {
  assert ( rendererP && srcTextureP && srcRectP && dstRectP );

  // TODO support scaling for headless so we can test that feature headlessly in xRender.
  assert( dstRectP->w == srcRectP->w && dstRectP->h == srcRectP->h );
      

  // The destination rectangle must overlap with the texture rectangle.
  if ( dstRectP->x >= rendererP->w ||
       dstRectP->y >= rendererP->h ||
       ( dstRectP->x + dstRectP->w ) < 0 ||
       ( dstRectP->y + dstRectP->h ) < 0 ) {
    return; 
  }

  // The source rectangle must overlap with the source texture.
  if ( srcRectP->x >= srcTextureP->w ||
       srcRectP->y >= srcTextureP->h ||
       ( srcRectP->x + srcRectP->w ) < 0 ||
       ( srcRectP->y + srcRectP->h ) < 0 ) {
    return;  // just skip this copy peacefully if source rect isn't located on texture
  }

  // Determine which part of src and dst rects to copy from/to.
  // Clamp the start of the rectangles.
  U32 dstStartX, dstStartY;
  U32 srcStartX = ( ( srcRectP->x ) < 0 ) ? 0 : srcRectP->x;
  U32 srcStartY = ( ( srcRectP->y ) < 0 ) ? 0 : srcRectP->y;
  if ( dstRectP->x < 0 ) {
    dstStartX = 0;
    srcStartX = fabs( dstRectP->x );
  }
  if ( dstRectP->y < 0 ) {
    dstStartY = 0;
    srcStartY = fabs( dstRectP->y );
  }
  // Clamp the end of the rectangles.
  U32 srcEndX = ( ( srcRectP->x + srcRectP->w ) >= srcTextureP->w ) ? srcRectP->w : srcRectP->x;
  U32 srcEndY = ( ( srcRectP->y + srcRectP->h ) >= srcTextureP->h ) ? srcRectP->h : srcRectP->y;
  U32 dstEndX = ( ( dstRectP->x + dstRectP->w ) >= rendererP->dstTextureP->w ) ? rendererP->dstTextureP->w : dstRectP->x;
  U32 dstEndY = ( ( dstRectP->y + dstRectP->h ) >= rendererP->dstTextureP->h ) ? rendererP->dstTextureP->h : dstRectP->y;

  Texture_* dstTextureP = rendererP->dstTextureP;
  S32 dstW = ( dstEndX - dstStartX ) * sizeof( Color_ );  // We'll clip it if necessary.

  Color_* srcStartPixelP;
  Color_* dstStartPixelP;

  for ( U32 dstY = dstStartY, srcY = srcStartY; dstY < dstEndY && srcY < srcEndY; ++dstY, ++srcY ) {
    // may need to offset into source rectangle if dest rect is clipped.
    srcStartPixelP = _getPixelP( srcTextureP, srcStartX, srcY );
    dstStartPixelP = _getPixelP( dstTextureP, dstStartX, dstY );
    memcpy( dstStartPixelP, srcStartPixelP, dstW );
  }
}

void present ( Renderer* rendererP ) {
  // Don't do anything. We're headless!
}

// Makes palette without setting its colors.
Surface* surfaceNew(void *pixelDataA, U32 w, U32 h) {
  assert (pixelDataA && w && h);
  Surface* surfaceP = jbAlloc(sizeof( Surface ), 1 );
  surfaceP->pixelA = arrayNew( sizeof( Color_ ), w * h );
  assert(surfaceP->pixelA);
  surfaceP->w = w;
  surfaceP->h = h;
  return surfaceP;
}

void surfaceDel(Surface_ **surfacePP) {
  if (surfacePP && *surfacePP) {
    arrayDel((void**) &(*surfacePP)->pixelA );
    jbFree( (void**) surfacePP );
  }
}

void appendAtlasPalette(Surface_ *atlasSurfaceP, ColorPalette *srcPaletteP) {
  // do  nothing lawl 
}

Texture* textureNew(Renderer_ *rendererP, Surface_ *surfaceP) {
  assert(  rendererP && surfaceP );
  Texture *textureP = jbAlloc(sizeof( Texture_ ), 1 );
  assert(textureP);
  textureP->pixelA = arrayNew( sizeof( Color_ ), surfaceP->w * surfaceP->h );
  assert(textureP->pixelA);
  textureP->w = surfaceP->w;
  textureP->h = surfaceP->h;
  memcpy( textureP->pixelA, surfaceP->pixelA, arrayGetElemSz( surfaceP->pixelA ) * arrayGetNElems( surfaceP->pixelA ) );
  return textureP;
}

void textureDel(Texture_ **texturePP) {
  if (texturePP && *texturePP) {
    arrayDel((void**) &(*texturePP)->pixelA );
    jbFree( (void**) texturePP );
  }
}

void clearScreen(Renderer_ *rendererP) {
  memset( rendererP->dstTextureP->pixelA, 0, arrayGetElemSz( rendererP->dstTextureP->pixelA ) * arrayGetNElems( rendererP->dstTextureP->pixelA ) );
}

#ifdef MULTITHREADED_

// =======================================
// Multithreading
// =======================================
static void _threadFuncArgArrayIni(CriticalFunc funcP, ThreadFuncArg *argA, U32 *nThreadsNeededP, void *_array) {
  if (argA && _array) {
    U32 nElemsToProcess = arrayGetNElems(_array);
    if (nElemsToProcess < *nThreadsNeededP) {
      *nThreadsNeededP = nElemsToProcess;
    }
    // Tell each thread where to start in array and how many items to process
    const U32 nElemsPerThread = ((nElemsToProcess + (*nThreadsNeededP >> 1)) / *nThreadsNeededP);
    for (U32 i = 0; i < *nThreadsNeededP; ++i) {
      argA[i].startIdx = i * nElemsPerThread;
      argA[i].nElemsToProcess = nElemsPerThread;
      argA[i].funcP = funcP;
      argA[i].array = _array;
    }
    // Last thread may have a different number of elems to process then the rest
    // (This might be faster than modulo.)
    argA[*nThreadsNeededP - 1].nElemsToProcess = nElemsToProcess - (nElemsPerThread * (*nThreadsNeededP - 1));
  }
}

// Generic multithreading function
static void* _mtGenericLoop(ThreadFuncArg *thargP) {
  const U32 ptrIncr = arrayGetElemSz(thargP->array);
  U8 *voidP = (U8*) thargP->array + ptrIncr * thargP->startIdx;
  U8 *voidEndP = voidP + thargP->nElemsToProcess;
  for (; voidP < voidEndP; voidP += ptrIncr) {
    thargP->funcP((void*) *((U32*) voidP));  
  }
  return NULL;
}

// Multithreading entry point
void multiThread( CriticalFunc funcP, void *_array) {
  if (!_array || !funcP) {
    return E_BAD_ARGS;
  }

  Thread threadA[N_CORES];
  ThreadFuncArg *thArgA = NULL;
  Error e = arrayNew((void**) &thArgA, sizeof(ThreadFuncArg), N_CORES);

  if (!e) {
    U32 nThreadsNeeded = N_CORES;
    // nThreadsNeeded gets updated to fewer than N_CORES if fewer elements than cores exist.
    _threadFuncArgArrayIni(funcP, thArgA, &nThreadsNeeded, _array);

    for (int i = 0; i < nThreadsNeeded; ++i) {
      threadIni_(&threadA[i], &thArgA[i]);
    }

    for (int i = 0; i < nThreadsNeeded; ++i) {
      threadJoin_(threadA[i]);
    }
  }

  arrayDel((void**) &thArgA);

  return e;
}

Error guiProcessEvents(Gui *guiP) {

  // ******************************************  Move things above this comment to guiNew
  // ******************************************  Move things above this comment to guiNew
  // ******************************************  Move things above this comment to guiNew
  // ******************************************  Move things above this comment to guiNew
  // Restore old terminal settings before exiting
  tcsetattr(STDIN_FILENO, TCSANOW, &tty_attr);
  // Move things above this comment to guiDel
  // ******************************************  Move things above this comment to guiNew
  // ******************************************  Move things above this comment to guiNew
  // ******************************************  Move things above this comment to guiNew
  // ******************************************  Move things above this comment to guiNew

  // Keyboard button-release events (only process this if engine is unpaused)
  // NOTE: You can implement a partial pause with any one of the other keys.
  // Grab each key press event
  struct input_event ev;
  while ( fread((void *)&ev, sizeof(ev), 1, file) != 0 ) {

    if ((ev.type == EV_KEY) && (ev.value != VAL_KEY_REPEAT))
    {
      printf("type: %d, val: %d, code: %d\n", ev.type, ev.value, ev.code); // EV_KEY corresponds to key press events
      if ( ev.value == VAL_KEY_UP ) {
        switch( ev.code ) {
          case KEY_a_:      guiP->buttonsPressed &= ~BTN_PRESSED_a; break;
          case KEY_b_:      guiP->buttonsPressed &= ~BTN_PRESSED_b; break;
          case KEY_c_:      guiP->buttonsPressed &= ~BTN_PRESSED_c; break;
          case KEY_d_:      guiP->buttonsPressed &= ~BTN_PRESSED_d; break;
          case KEY_e_:      guiP->buttonsPressed &= ~BTN_PRESSED_e; break;
          case KEY_f_:      guiP->buttonsPressed &= ~BTN_PRESSED_f; break;
          case KEY_g_:      guiP->buttonsPressed &= ~BTN_PRESSED_g; break;
          case KEY_h_:      guiP->buttonsPressed &= ~BTN_PRESSED_h; break;
          case KEY_i_:      guiP->buttonsPressed &= ~BTN_PRESSED_i; break;
          case KEY_j_:      guiP->buttonsPressed &= ~BTN_PRESSED_j; break;
          case KEY_k_:      guiP->buttonsPressed &= ~BTN_PRESSED_k; break;
          case KEY_l_:      guiP->buttonsPressed &= ~BTN_PRESSED_l; break;
          case KEY_m_:      guiP->buttonsPressed &= ~BTN_PRESSED_m; break;
          case KEY_n_:      guiP->buttonsPressed &= ~BTN_PRESSED_n; break;
          case KEY_o_:      guiP->buttonsPressed &= ~BTN_PRESSED_o; break;
          case KEY_p_:      guiP->buttonsPressed &= ~BTN_PRESSED_p; break;
          case KEY_q_:      guiP->buttonsPressed &= ~BTN_PRESSED_q; break;
          case KEY_r_:      guiP->buttonsPressed &= ~BTN_PRESSED_r; break;
          case KEY_s_:      guiP->buttonsPressed &= ~BTN_PRESSED_s; break;
          case KEY_t_:      guiP->buttonsPressed &= ~BTN_PRESSED_t; break;
          case KEY_u_:      guiP->buttonsPressed &= ~BTN_PRESSED_u; break;
          case KEY_v_:      guiP->buttonsPressed &= ~BTN_PRESSED_v; break;
          case KEY_w_:      guiP->buttonsPressed &= ~BTN_PRESSED_w; break;
          case KEY_x_:      guiP->buttonsPressed &= ~BTN_PRESSED_x; break;
          case KEY_y_:      guiP->buttonsPressed &= ~BTN_PRESSED_y; break;
          case KEY_z_:      guiP->buttonsPressed &= ~BTN_PRESSED_z; break;
          case KEY_SPACE_:  guiP->buttonsPressed &= ~BTN_PRESSED_SPACE; break;
          case KEY_LSHIFT_: guiP->buttonsPressed &= ~BTN_PRESSED_LSHIFT; break;
          default: break;
        }  // switch-case for keys released
      } // if key-up event
        // Keyboard button-press events
      else if ( ev.value == VAL_KEY_DOWN ) {
        switch( ev.code ) {
          case KEY_a_:      guiP->buttonsPressed |=  BTN_PRESSED_a; break;
          case KEY_b_:      guiP->buttonsPressed |=  BTN_PRESSED_b; break;
          case KEY_c_:      guiP->buttonsPressed |=  BTN_PRESSED_c; break;
          case KEY_d_:      guiP->buttonsPressed |=  BTN_PRESSED_d; break;
          case KEY_e_:      guiP->buttonsPressed |=  BTN_PRESSED_e; break;
          case KEY_f_:      guiP->buttonsPressed |=  BTN_PRESSED_f; break;
          case KEY_g_:      guiP->buttonsPressed |=  BTN_PRESSED_g; break;
          case KEY_h_:      guiP->buttonsPressed |=  BTN_PRESSED_h; break;
          case KEY_i_:      guiP->buttonsPressed |=  BTN_PRESSED_i; break;
          case KEY_j_:      guiP->buttonsPressed |=  BTN_PRESSED_j; break;
          case KEY_k_:      guiP->buttonsPressed |=  BTN_PRESSED_k; break;
          case KEY_l_:      guiP->buttonsPressed |=  BTN_PRESSED_l; break;
          case KEY_m_:      guiP->buttonsPressed |=  BTN_PRESSED_m; break;
          case KEY_n_:      guiP->buttonsPressed |=  BTN_PRESSED_n; break;
          case KEY_o_:      guiP->buttonsPressed |=  BTN_PRESSED_o; break;
          case KEY_p_:      guiP->buttonsPressed |=  BTN_PRESSED_p; break;
          case KEY_q_:      guiP->buttonsPressed |=  BTN_PRESSED_q; break;
          case KEY_r_:      guiP->buttonsPressed |=  BTN_PRESSED_r; break;
          case KEY_s_:      guiP->buttonsPressed |=  BTN_PRESSED_s; break;
          case KEY_t_:      guiP->buttonsPressed |=  BTN_PRESSED_t; break;
          case KEY_u_:      guiP->buttonsPressed |=  BTN_PRESSED_u; break;
          case KEY_v_:      guiP->buttonsPressed |=  BTN_PRESSED_v; break;
          case KEY_w_:      guiP->buttonsPressed |=  BTN_PRESSED_w; break;
          case KEY_x_:      guiP->buttonsPressed |=  BTN_PRESSED_x; break;
          case KEY_y_:      guiP->buttonsPressed |=  BTN_PRESSED_y; break;
          case KEY_z_:      guiP->buttonsPressed |=  BTN_PRESSED_z; break;
          case KEY_SPACE_:  guiP->buttonsPressed |=  BTN_PRESSED_SPACE; break;
          case KEY_LSHIFT_: guiP->buttonsPressed |=  BTN_PRESSED_LSHIFT; break;
        }  // switch-case for keys pressed
      }  // if key-down event
    }
    if (ev.code ==1 ) {
      return E_QUIT;
    }
  }
  return SUCCESS;
}

// TODO get rid of these after you fix xRender to not have an atlas palette.
Color_* getColorPalette( Surface *surfaceP ) {
  return NULL;
}
U32 getNColors( Surface *surfaceP ) {
  return 0;
}
#endif
