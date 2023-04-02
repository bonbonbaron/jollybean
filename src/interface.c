#include "interface.h"
#include "SDL_events.h"

// TODO abstract away all SDL-specific stuff to an SDL-based interface header, 
//      which in turn will be selected in interface.h.
/**********************
 * Rendering
 *********************/

static int _eventFilter(void *userDataP, SDL_Event *eventP) {
  switch (eventP->type) {
    case SDL_MOUSEWHEEL:
    case SDL_MOUSEMOTION:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
    //case 0x7f00:  // IDK what this is, but it was annoying.
    case SDL_WINDOWEVENT_MOVED:
      return 0;  // Return 1 to ignore the event. (Thanks ChatGPT!)
    default:
      return 1;
  }
}

Error guiNew(Gui **guiPP) {
	if (!guiPP) {
		return E_BAD_ARGS;
  }
	// Init SDL
	if (SDL_Init(SDL_INIT_VIDEO) != SUCCESS) {
		return EXIT_FAILURE;
  }
  Error e = jbAlloc((void**) guiPP, sizeof(Gui), 1);
  if (!e) {
	// Init window
	  (*guiPP)->windowP = SDL_CreateWindow("Hello world!", 100, 100, 512, 448, 
        SDL_WINDOW_BORDERLESS | 
        SDL_WINDOW_RESIZABLE | 
        SDL_WINDOW_OPENGL
    );
  }
	if (!(*guiPP)->windowP) {
    guiDel(guiPP);
		return EXIT_FAILURE;
  }
	// Init renderer
  if (!e) {
    (*guiPP)->rendererP = SDL_CreateRenderer(
        (*guiPP)->windowP,
        -1, 
        SDL_RENDERER_ACCELERATED | 
        SDL_RENDERER_PRESENTVSYNC
    );
  }
	if (!(*guiPP)->rendererP) {
    guiDel(guiPP);
		return EXIT_FAILURE;
	}
  if (!e) {
    e = SDL_RenderSetLogicalSize((*guiPP)->rendererP, 256, 224);
  }
  // Events
  (*guiPP)->buttonsPressed = 0;
  SDL_SetEventFilter(_eventFilter, NULL);
	return e;
}

void guiDel(Gui **guiPP) {
  if (guiPP && *guiPP) {
    if ((*guiPP)->rendererP) {
      SDL_DestroyRenderer((*guiPP)->rendererP);
      (*guiPP)->rendererP = NULL;
    }
    if ((*guiPP)->windowP) {
      SDL_DestroyWindow((*guiPP)->windowP);
      (*guiPP)->windowP = NULL;
    }
    jbFree((void**) guiPP);
  }
  SDL_Quit();
}

// Makes palette without setting its colors.
Error surfaceNew(Surface_ **surfacePP, void *pixelDataA, U32 w, U32 h) {
	if (!surfacePP) {
    return E_BAD_ARGS;
  }
  SDL_ClearError();
  *surfacePP = SDL_CreateRGBSurfaceWithFormatFrom(pixelDataA, w, h, 8, w, SDL_PIXELFORMAT_INDEX8);
  // Palette
  if (!*surfacePP) {
    printf("\n\nSDL error: %s\n\n\n", SDL_GetError());
		return E_UNSUPPORTED_PIXEL_FORMAT;
  }

	return SUCCESS;
}

void surfaceDel(Surface_ **surfacePP) {
  if (surfacePP && *surfacePP) {
    SDL_FreeSurface(*surfacePP);
    *surfacePP = NULL;
  }
}

void appendAtlasPalette(Surface_ *atlasSurfaceP, ColorPalette *srcPaletteP) {
  SDL_SetPaletteColors(atlasSurfaceP->format->palette, 
      srcPaletteP->colorA, srcPaletteP->atlasPaletteOffset, srcPaletteP->nColors);
}

Error textureNew(Texture_ **texturePP, Renderer_ *rendererP, Surface_ *surfaceP) {
	if (!texturePP || !rendererP || !surfaceP) {
		return E_BAD_ARGS;
  }

  //TODO add an #if that only uses this code when interfacing SDL
	*texturePP = SDL_CreateTextureFromSurface(rendererP, surfaceP);
	if (!*texturePP) {
		return E_UNSUPPORTED_PIXEL_FORMAT;
  }
  Error e = textureSetAlpha(*texturePP);

	return e;
}

void textureDel(Texture_ **texturePP) {
  if (texturePP && *texturePP) {
    SDL_DestroyTexture(*texturePP);
    *texturePP = NULL;
  }
}

Error textureSetAlpha(Texture_ *textureP) {
	if (!textureP) {
		return E_BAD_ARGS;
  }
	return SDL_SetTextureBlendMode(textureP, SDL_BLENDMODE_BLEND);
}

void clearScreen(Renderer_ *rendererP) {
	SDL_SetRenderDrawColor(rendererP, 0xff, 0xff, 0xff, 0xff);
	SDL_RenderClear(rendererP);
}

#ifdef MULTITHREADED_

// =======================================
// Multithreaded strip data reconstruction
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
	Event_ event;
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
    // Window events
    if (event.type == SDL_WINDOWEVENT) {
      switch (event.window.event) {
      case SDL_WINDOWEVENT_RESIZED:        
      case SDL_WINDOWEVENT_HIDDEN:         
        break;
      case SDL_WINDOWEVENT_SHOWN:          
      case SDL_WINDOWEVENT_EXPOSED:        
      case SDL_WINDOWEVENT_MAXIMIZED:      
      case SDL_WINDOWEVENT_RESTORED:       
        present_(guiP->rendererP);
        break;
      case SDL_WINDOWEVENT_FOCUS_GAINED:   
        return E_UNPAUSE;
      case SDL_WINDOWEVENT_FOCUS_LOST:     
        return E_PAUSE;
      case SDL_WINDOWEVENT_CLOSE:          
        return E_QUIT;
      default: 
        break;
      }
    }
	}
  return SUCCESS;
}
#endif
