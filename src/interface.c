#include "interface.h"

/**********************
 * Rendering
 *********************/

Error guiNew(Window_ **windowPP, Renderer_ **rendererPP) {
  //TODO add an #if that only uses this code when interfacing SDL
	if (!windowPP || !rendererPP) {
		return E_BAD_ARGS;
  }
	// Init SDL
	if (SDL_Init(SDL_INIT_VIDEO) != SUCCESS) {
    //printf("[guiNew] SDL error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
  }
	// Init window
	//*windowPP = SDL_CreateWindow("Hello world!", 0, 0, 0, 0, SDL_WINDOW_FULLSCREEN);
	*windowPP = SDL_CreateWindow("Hello world!", 100, 100, 400, 300, 0);
	if (!*windowPP) {
		return EXIT_FAILURE;
  }
	// Init renderer
	*rendererPP = SDL_CreateRenderer(*windowPP, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!*rendererPP) {
    //printf("%s\n", SDL_GetError());
		SDL_DestroyWindow(*windowPP);
		SDL_Quit();
		return EXIT_FAILURE;
	}
	return SUCCESS;
}

void guiDel(Window_ **windowPP, Renderer_ **rendererPP) {
  if (rendererPP && *rendererPP) {
    SDL_DestroyRenderer(*rendererPP);
    *rendererPP = NULL;
  }
  if (windowPP && *windowPP) {
    SDL_DestroyWindow(*windowPP);
    *windowPP = NULL;
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
	SDL_DestroyTexture(*texturePP);
	*texturePP = NULL;
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
#endif
