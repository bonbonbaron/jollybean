#include "interface.h"
#include <assert.h>
#include <math.h>

// TODO abstract away all SDL-specific stuff to an SDL-based interface header, 
//      which in turn will be selected in interface.h.
/**********************
 * Rendering
 *********************/

Error guiNew(Gui **guiPP) {
	if (!guiPP) {
		return E_BAD_ARGS;
  }

	// Init the GUI
  Error e = jbAlloc((void**) guiPP, sizeof(Gui), 1);

  if (!e) {
    memset(*guiPP, 0, sizeof(Gui));
	// Init window
    e = jbAlloc( (void**) &(*guiPP)->windowP, sizeof(Window), 1 );
  }

	if ( e ) {
    guiDel(guiPP);
		return EXIT_FAILURE;
  }

	// Init renderer
  if (!e) {
    e = jbAlloc( (void**) &(*guiPP)->rendererP, sizeof(Window), 1 );
    if ( !e ) {
      e = arrayNew( (void**) &(*guiPP)->rendererP->dstTextureP, sizeof(Texture), 1 );
    }

  }

	if ( e ) {
    guiDel(guiPP);
		return EXIT_FAILURE;
	}

	return e;
}

void guiDel(Gui **guiPP) {
  if (guiPP && *guiPP) {
    jbFree((void**) &(*guiPP)->rendererP);
    jbFree((void**) &(*guiPP)->windowP);
    jbFree((void**) guiPP);
  }
}

static Color_* _getPixelP( Texture *textureP, U32 x, U32 y ) {
  assert( textureP && textureP->pixelA &&
      ( textureP->pixelA + ( textureP->w * textureP->h ) + x ) < ( textureP->pixelA + ( arrayGetNElems( textureP->pixelA ) ) ) );
  return textureP->pixelA + ( textureP->w * textureP->h ) + x;
}

// Since the headless renderer isn't actually going to display anything, we don't care about scaling.
Error copy ( Renderer* rendererP, Texture* srcTextureP, Rect* srcRectP, Rect* dstRectP ) {
  if ( !rendererP || !srcTextureP || !srcRectP || !dstRectP ) {
    return E_BAD_ARGS;
  }

  // TODO support scaling for headless so we can test that feature headlessly in xRender.
  assert( dstRectP->w == srcRectP->w && dstRectP->h == srcRectP->h );
      

  // The destination rectangle must overlap with the texture rectangle.
  if ( dstRectP->x >= rendererP->w ||
       dstRectP->y >= rendererP->h ||
       ( dstRectP->x + dstRectP->w ) < 0 ||
       ( dstRectP->y + dstRectP->h ) < 0 ) {
    return SUCCESS; 
  }

  // The source rectangle must overlap with the source texture.
  if ( srcRectP->x >= srcTextureP->w ||
       srcRectP->y >= srcTextureP->h ||
       ( srcRectP->x + srcRectP->w ) < 0 ||
       ( srcRectP->y + srcRectP->h ) < 0 ) {
    return SUCCESS;  // just skip this copy peacefully if source rect isn't located on texture
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

  Texture* dstTextureP = rendererP->dstTextureP;
  S32 dstW = ( dstEndX - dstStartX ) * sizeof( Color_ );  // We'll clip it if necessary.

  Color_* srcStartPixelP;
  Color_* dstStartPixelP;

  for ( U32 dstY = dstStartY, srcY = srcStartY; dstY < dstEndY && srcY < srcEndY; ++dstY, ++srcY ) {
    // may need to offset into source rectangle if dest rect is clipped.
    srcStartPixelP = _getPixelP( srcTextureP, srcStartX, srcY );
    dstStartPixelP = _getPixelP( dstTextureP, dstStartX, dstY );
    memcpy( dstStartPixelP, srcStartPixelP, dstW );
  }
  
  return SUCCESS;
}

void present ( Renderer* rendererP ) {
  // Don't do anything. We're headless!
}

// Makes palette without setting its colors.
Error surfaceNew(Surface_ **surfacePP, void *pixelDataA, U32 w, U32 h) {
	if (!surfacePP || !pixelDataA) {
    return E_BAD_ARGS;
  }
  Error e = jbAlloc( (void**) surfacePP, sizeof( Surface ), 1 );
  if (!e ) {
    e = arrayNew( (void**) &(*surfacePP)->pixelA, sizeof( Color_ ), w * h );
    if ( e ) {
      jbFree( (void**) surfacePP );
    }
    (*surfacePP)->w = w;
    (*surfacePP)->h = h;
  }
	return e;
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

Error textureNew(Texture_ **texturePP, Renderer_ *rendererP, Surface_ *surfaceP) {
	if ( !texturePP || !rendererP || !surfaceP ) {
    return E_BAD_ARGS;
  }
  Error e = jbAlloc( (void**) texturePP, sizeof( Texture ), 1 );
  if (!e ) {
    e = arrayNew( (void**) &(*texturePP)->pixelA, sizeof( Color_ ), surfaceP->w * surfaceP->h );
    if ( e ) {
      jbFree( (void**) texturePP );
    }
    (*texturePP)->w = surfaceP->w;
    (*texturePP)->h = surfaceP->h;
    memcpy( (*texturePP)->pixelA, surfaceP->pixelA, arrayGetElemSz( surfaceP->pixelA ) * arrayGetNElems( surfaceP->pixelA ) );
  }
	return e;
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
  for (Error e = SUCCESS; !e && voidP < voidEndP; voidP += ptrIncr) {
    e = thargP->funcP((void*) *((U32*) voidP));  // ugly but... how else to generalize?
  }
  return NULL;
}

// Multithreading entry point
Error multiThread( CriticalFunc funcP, void *_array) {
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
	while (pollEvent_(&event)) {
		if (event.type == EVENT_QUIT_) {
      return E_QUIT;
		}
    // Keyboard button-release events (only process this if engine is unpaused)
    // NOTE: You can implement a partial pause with any one of the other keys.
    if (guiP->state == UNPAUSED) {
      if (event.type == EVENT_KEYUP_ && event.key.repeat == 0) {
        switch(event.key.keysym.sym) {
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
        case KEY_ESCAPE_: return E_QUIT;
        default:
          break;
        }  // switch-case for keys released
      } // if key-up event
      // Keyboard button-press events
      else if (event.type == EVENT_KEYDOWN_ && event.key.repeat == 0) {
        switch(event.key.keysym.sym) {
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
        case KEY_ESCAPE_: return E_QUIT;
        }  // switch-case for keys pressed
      }  // if key-down event
		}  // if unpaused
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
