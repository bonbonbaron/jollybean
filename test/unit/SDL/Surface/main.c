#include "SDL_pixels.h"
#include "SDL_render.h"
#include "interface.h"
#include "xRender.h"
#include "blehColormap.h"
#include "blehColorPalette.h"
#include "heckColormap.h"
#include "heckColorPalette.h"
#include "redColormap.h"
#include "redColorPalette.h"

// We leverage this knowledge to navigate backwards without re-entering already-explored nodes.
#define getParentAtlasIdx_(childIdx_) ((childIdx_ - 1 - !(cameFromRight = childIdx_ & 1)) >> 1)
#define N_SAMPLES (3)
int main(int argc, char **argv) {
  Gene **genePA = NULL;

  // ====================================================
  // Repeat things done in multithreadingg test for setup
  // ====================================================
  Error e = arrayNew((void**) &genePA, sizeof(Gene*), N_SAMPLES);

  ColorPalette *cpPA[N_SAMPLES] = {
    &blehColorPalette,
    &redColorPalette,
    &heckColorPalette
  };
  // Mock up some genes.
  if (!e) {
    for (U32 i = 0; !e && i < N_SAMPLES; ++i) {
      Gene *geneP = NULL;
      e = jbAlloc((void**) &geneP, sizeof(Gene), 1);
      if (!e) {
        genePA[i] = geneP;
        genePA[i]->geneClass = MEDIA_GENE;
        genePA[i]->u.unitary.key = 0;
        genePA[i]->u.unitary.size = sizeof(XRenderComp);
        genePA[i]->u.unitary.type = RENDER;
      }
    }
    genePA[0]->u.unitary.dataP = &blehColormap;
    genePA[1]->u.unitary.dataP = &redColormap;
    genePA[2]->u.unitary.dataP = &heckColormap;
  }


  // Extract stripdata from each media gene into an array of contiguous strip data pointers.
  StripDataS **sdPA = NULL;
  if (!e) {
    e = arrayNew((void**) &sdPA, sizeof(StripDataS*), arrayGetNElems(genePA));
  }
  if (!e) {
    for (U32 i = 0, iEnd = arrayGetNElems(sdPA); i < iEnd; ++i) {
      // dataP should point to a struct whose first member is a StripDataS pointer,
      // so treat it like a StripDataS double-pointer.
      sdPA[i] = *((StripDataS**) genePA[i]->u.unitary.dataP);  
    }
  }

#define MULTITHREADED 1
  // Inflate colormap inflatables
#if MULTITHREADED
  if (!e) {
    e = multithread_(sdInflate, (void*) sdPA);
  }
  // Unpack stripsets
  if (!e) {
    e = multithread_(sdUnpack, (void*) sdPA);
  }
  // Assemble colormaps from strips
  if (!e) {
    e = multithread_(sdAssemble, (void*) sdPA);
  }
#else 
  for (int i = 0; !e && i < N_SAMPLES; ++i) {
    e = stripIni(*((StripDataS**) genePA[i]->dataP));
  }
#endif

  // *********************************************
	// Init SDL
  // *********************************************
  for (int i = 0; i < SDL_GetNumVideoDrivers(); ++i) {
    printf("%s\n", SDL_GetVideoDriver(i));
  }
  //return 0;
  printf("before sdl init\n");
	if (SDL_Init(SDL_INIT_VIDEO) != SUCCESS) {
    //printf("[guiNew] SDL error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
  }
  printf("after sdl init\n");

  // Window and renderer
  SDL_Window *windowP = NULL;
  SDL_Renderer *rendererP = NULL;
#if 0
	windowP = SDL_CreateWindow("Hello world!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1080, 1920, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
#else
  printf("before window\n");
	windowP = SDL_CreateWindow("Hello world!", 100, 100, 500, 500, SDL_WINDOW_OPENGL );
  printf("after window\n");
#endif
	if (!windowP) {
		return EXIT_FAILURE;
  }
  printf("before renderer\n");
	rendererP = SDL_CreateRenderer(windowP, 0, 0);
  SDL_RendererInfo info;
  SDL_GetRendererInfo(rendererP, &info);
  printf("using renderer %s\n", info.name);
  printf("after renderer\n");
	if (!rendererP) {
		SDL_DestroyWindow(windowP);
		SDL_Quit();
		return EXIT_FAILURE;
	}

  // Surfaces
  Surface_ **surfPA = NULL;
  if (!e) {
    e = arrayNew((void**) &surfPA, sizeof(Surface_*), N_SAMPLES);
  }
  for (int i = 0; i < N_SAMPLES; ++i) {
    printf("surface %d\n", i);
    if (!e) {
      printf("newing\n");
      Colormap *cmP = (Colormap*) genePA[i]->u.unitary.dataP;
/*
    SDL_Surface * SDL_CreateRGBSurfaceFrom(
        void *pixels, int width, int height, int depth, int pitch,
                         Uint32 Rmask, Uint32 Gmask, Uint32 Bmask,
                         Uint32 Amask)
*/
//SDL_CreateRGBSurfaceWithFormatFrom(void *pixels,
                         //int width, int height, int depth, int pitch,
                         //Uint32 format)
      //surfPA[i] = SDL_CreateRGBSurfaceWithFormatFrom(cmP->sdP->assembledDataA, 
          //cmP->w, cmP->h, 8, cmP->w, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
      surfPA[i] = SDL_CreateRGBSurfaceWithFormatFrom(cmP->sdP->assembledDataA, 
          cmP->w, cmP->h, 8, cmP->w, SDL_PIXELFORMAT_INDEX8);
      SDL_SetPaletteColors(surfPA[i]->format->palette, cpPA[i]->colorA, 0, cpPA[i]->nColors);


      //surfPA[i] = SDL_CreateRGBSurface(0, cmP->w, cmP->h, 8, 0, 0, 0, 0);
      if (!surfPA[i]) {
        printf("surface creation failed for surface %d\n", i);
        e = E_NO_MEMORY;
      }
#if 0
      // Sanity check
      for (int y = 0; y < surfPA[i]->h; ++y) {
        for (int x = 0; x < surfPA[i]->pitch; ++x) {
          printf("%d", ((U8*) surfPA[i]->pixels)[y * surfPA[i]->pitch + x]);
        }
        printf("\n");
      }
#endif
    }
  }

  // Palette
  // Textures
  SDL_Texture **texturePA = NULL;
  if (!e) {
    e = arrayNew((void**) &texturePA, sizeof(void*), N_SAMPLES);
  }
  printf("e is %d\n", e);
  if (!e) {
    for (int i = 0; !e && i < N_SAMPLES; ++i) {
      texturePA[i] = SDL_CreateTextureFromSurface(rendererP, surfPA[i]);
      if (!texturePA[i]) {
        printf("texture creation failed for %d\n", i);
        e = E_NO_MEMORY;
      }
    }
  }
  if (!e) {
    SDL_SetRenderDrawColor(rendererP, 0xff, 0xff, 0xff, 0xff);
    for (int i = 0; i < N_SAMPLES; ++i) {
      printf("rendering %d\n", i);
      SDL_RenderClear(rendererP);
      SDL_RenderCopy(rendererP, texturePA[i], NULL, NULL);
      SDL_RenderPresent(rendererP);
      SDL_Delay(1000);
    }
  }


  // Deflate colormap inflatables
  if (!e) {
#if MULTITHREADED
    e = multithread_(stripClr,   (void*) sdPA);
#else
    for (int i = 0; !e && i < N_SAMPLES; ++i) {
      stripClr(*((StripDataS**) genePA[i]->dataP));
    }
#endif
  }

  // ====================================================
  // Clean up
  // ====================================================
  arrayDel((void**) &sdPA);
  for (U32 i = 0; i < arrayGetNElems(genePA); ++i) {
    jbFree((void**) &genePA[i]);
  }
  arrayDel((void**) &genePA);
  SDL_Quit();
  return e;
}
