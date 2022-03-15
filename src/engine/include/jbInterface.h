#ifndef JB_INTERFACE
#define JB_INTERFACE
#include "botox.h"
#include "xRender.h"

#define SDL

#ifdef SDL
#include "SDL.h"
typedef SDL_Color		 Color;     // r, g, b, & a are U8s
typedef SDL_Rect		 Rect;		  // x, y, w, & h are S32s
typedef SDL_Texture  Texture;   // Arbitrary, but our system doesn't have to know that.
typedef SDL_Window   Window;    // Arbitrary, but our system doesn't have to know that.
typedef SDL_Renderer Renderer;  // Arbitrary, but our system doesn't have to know that. 
typedef SDL_Surface  Surface;
typedef SDL_Palette  Palette;
#endif  // SDL

// Implemented by environment of choice (see #defines for each category above)
Error guiNew(Window **windowPP, Renderer **rendererPP);
Error surfaceNew(Surface **surfacePP, Colormap *cP);
Error iniSurfacePixels(Surface *surfaceP, Colormap *cP);
Error textureNew(Texture **texturePP, Renderer *rendererP, Surface *surfaceP);
void textureDel(Texture **texturePP);
Error setTextureAlpha(Texture *textureP);
void clearScreen(Renderer *rendererP);
Error imageSet(Renderer *rendererP, XRenderComp *cP);
void render (Renderer* rendererP);
#endif
