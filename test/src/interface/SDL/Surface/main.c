#include "SDL_pixels.h"
#include "SDL_render.h"
#include "interface.h"
#include "xRender.h"
#include "gene.h"
#include "blehColormap.h"
#include "blehColorPalette.h"
#include "heckColormap.h"
#include "heckColorPalette.h"
#include "redColormap.h"
#include "redColorPalette.h"

#define W (10)
#define H (10)
#define N (W * H)
U32 dumbSurf[100] = { 0xff0000ff };

// We leverage this knowledge to navigate backwards without re-entering already-explored nodes.
#define getParentAtlasIdx_(childIdx_) ((childIdx_ - 1 - !(cameFromRight = childIdx_ & 1)) >> 1)
#define N_SAMPLES (3)
int main(int argc, char **argv) {
  Gene **genePA = arrayNew( sizeof(Gene*), N_SAMPLES );
  // ====================================================
  // Repeat things done in multithreadingg test for setup
  // ====================================================

  ColorPalette *cpPA[N_SAMPLES] = {
    &blehColorPalette,
    &redColorPalette,
    &heckColorPalette
  };
  // Mock up some genes.
  for (U32 i = 0; i < N_SAMPLES; ++i) {
    Gene *geneP = jbAlloc(sizeof(Gene), 1);
    genePA[i] = geneP;
    genePA[i]->geneClass = MEDIA_GENE;
    genePA[i]->u.unitary.key = 0;
    genePA[i]->u.unitary.size = sizeof(XRenderComp);
    genePA[i]->u.unitary.type = RENDER;
  }
  genePA[0]->u.unitary.dataP = &blehColormap;
  genePA[1]->u.unitary.dataP = &redColormap;
  genePA[2]->u.unitary.dataP = &heckColormap;


  // Extract stripdata from each media gene into an array of contiguous strip data pointers.
  StripDataS **sdPA = arrayNew(sizeof(StripDataS*), arrayGetNElems(genePA));
  for (U32 i = 0, iEnd = arrayGetNElems(sdPA); i < iEnd; ++i) {
    // dataP should point to a struct whose first member is a StripDataS pointer,
    // so treat it like a StripDataS double-pointer.
    sdPA[i] = *((StripDataS**) genePA[i]->u.unitary.dataP);  
  }

#define MULTITHREADED 0
  // Inflate colormap inflatables
#if MULTITHREADED
  multithread_(sdInflate, (void*) sdPA);
  // Unpack stripsets
  multithread_(sdUnpack, (void*) sdPA);
  // Assemble colormaps from strips
  multithread_(sdAssemble, (void*) sdPA);
#else 
  for (int i = 0; i < N_SAMPLES; ++i) {
    stripIni(*((StripDataS**) genePA[i]->u.unitary.dataP));
  }
#endif

  // *********************************************
  // Init SDL
  // *********************************************
  printf("getting video drivers\n");
  for (int i = 0; i < SDL_GetNumVideoDrivers(); ++i) {
    printf("%s\n", SDL_GetVideoDriver(i));
  }

#define SUCCESS (0)
  if (SDL_Init(SDL_INIT_VIDEO) != SUCCESS) {
    printf("[initializing SDL] SDL error: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  // Window and renderer
  SDL_Window *windowP = NULL;
  SDL_Renderer *rendererP = NULL;
#if 0
  windowP = SDL_CreateWindow("Hello world!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1080, 1920, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
#else
  windowP = SDL_CreateWindow("Hello world!", 100, 100, 500, 500, SDL_WINDOW_OPENGL );
#endif
  if (!windowP) {
    return EXIT_FAILURE;
  }

  rendererP = SDL_CreateRenderer(windowP, 0, 0);
  SDL_RendererInfo info;
  SDL_GetRendererInfo(rendererP, &info);
  printf("using renderer %s\n", info.name);

  if (!rendererP) {
    SDL_DestroyWindow(windowP);
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // Surfaces and Palletes
  Surface_ **surfPA = arrayNew( sizeof(Surface_*), N_SAMPLES);
  for (int i = 0; i < N_SAMPLES; ++i) {
    Colormap *cmP = (Colormap*) genePA[i]->u.unitary.dataP;
#if 0
    // Surface
    surfPA[i] = SDL_CreateRGBSurfaceWithFormatFrom(cmP->sdP->assembledDataA, 
        cmP->w, cmP->h, 8, cmP->w, SDL_PIXELFORMAT_INDEX8);
    if ( !surfPA[i] ) {
      printf("[creating rgb surface] SDL error: %s\n", SDL_GetError());
      return EXIT_FAILURE;
    }
    // Pallete
    int err = SDL_SetPaletteColors(surfPA[i]->format->palette, cpPA[i]->colorA, 0, cpPA[i]->nColors);
    if (err) {
      printf("[setting palette] SDL error: %s\n", SDL_GetError());
      return EXIT_FAILURE;
    }
#else
    // Surface
    surfPA[i] = SDL_CreateRGBSurfaceWithFormatFrom(dumbSurf, W, H, 32, W * sizeof(dumbSurf[0]), SDL_PIXELFORMAT_RGBA32);
    // surfPA[i] = SDL_CreateRGBSurface(0, W, H, 32, 0, 0, 0, 0 );
    if ( !surfPA[i] ) {
      printf("[creating rgb surface] SDL error: %s\n", SDL_GetError());
      return EXIT_FAILURE;
    }
#endif
  }

  // Textures
  SDL_Texture **texturePA = arrayNew( sizeof(SDL_Texture*), N_SAMPLES);
  assert( arrayGetNElems( texturePA ) == 3 );
  assert( arrayGetElemSz( texturePA ) == sizeof( SDL_Texture* ) );
  for (int i = 0; i < N_SAMPLES; ++i) {
    texturePA[i] = SDL_CreateTextureFromSurface(rendererP, surfPA[i]);
    // SDL_CreateTextureFromSurface(rendererP, surfPA[i]);
    if ( !texturePA[i] ) {
      printf("[creating texture from surface] SDL error: %s\n", SDL_GetError());
      return EXIT_FAILURE;
    }
  }

  SDL_Rect srcRect = { 0, 0, W, H };

  // Present results
  SDL_SetRenderDrawColor(rendererP, 0xff, 0xff, 0xff, 0xff);
  for (int i = 0; i < N_SAMPLES; ++i) {
    printf("rendering %d\n", i);
    SDL_RenderClear(rendererP);
    SDL_RenderCopy(rendererP, texturePA[i], &srcRect, NULL);
    SDL_RenderPresent(rendererP);
    SDL_Delay(1000);
  }


  // Deflate colormap inflatables
#if MULTITHREADED
  multithread_(stripClr, (void*) sdPA);
#else
  for (int i = 0; i < N_SAMPLES; ++i) {
    stripClr(*((StripDataS**) genePA[i]->u.unitary.dataP));
  }
#endif

  // ====================================================
  // Clean up
  // ====================================================
  arrayDel((void**) &sdPA);
  arrayDel((void**) &texturePA);
  arrayDel((void**) &surfPA);
  for (U32 i = 0; i < arrayGetNElems(genePA); ++i) {
    jbFree((void**) &genePA[i]);
  }
  arrayDel((void**) &genePA);
  SDL_Quit();
  return 0;
}
