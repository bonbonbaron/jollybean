#include "jb.h"
/* TODO:
    implement events; do they register in Sprite's functions' printf() statements?
    encapsulate sprite(s) in a scene; does it still draw?
    blit just one sprite frame to window surface
    animate the sprite; code the actual animation now
    move the sprite; code the actual motion now
		extern init_scene_zero (develop first)
*/

static SDL_Window   *window;
static SDL_Renderer *renderer;
static SDL_Surface  *canvas_surface;
static Scene        *scene;

#define BUTTON_REPEAT_RATE 50

Error render() {
	Error stat = 0;
	Uint16 i;
  SDL_Rect rect;

  SDL_RenderClear(renderer);

  /* TODO: Consider pointer arithmetic instead for this. */


	SDL_UpdateWindowSurface(window);
  return stat;
}

Error init(Uint8 *bp_p) {
  Error stat = 0;
  extern Scene *scene_zero;

  /* Communication Channels */
	renderer = NULL;

  /* SDL Video */
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		return 1;
	}

  /* Window surface */
  window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
  if (window == NULL) {
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			return 1;
  }

  canvas_surface = SDL_GetWindowSurface(window);
  if (canvas_surface == NULL) {
    return 1;
  }

  /* Renderer w/ RLE acceleration and color key */
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL ) {
		printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
		return 1;
	}
  stat = SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);
#if SDL_BIG_ENDIAN
  if (!stat) {
    stat = SDL_SetColorKey(canvas_surface, SDL_TRUE, 0x000000FF);
#else
    stat = SDL_SetColorKey(canvas_surface, SDL_TRUE, 0xFF000000);
#endif
  }
  if (!stat) {
    stat = SDL_SetSurfaceRLE(canvas_surface, SDL_TRUE);
  }
  if (!stat) {
    stat = SDL_RenderSetLogicalSize(renderer, 256, 224);
  }
  if (!stat) {
    stat = SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
  }

  /* Scene Zero */
  if (!stat) {
    scene = scene_zero;
  }

	if (!stat) {
		*bp_p = 0;
	}

	return stat;
}

void close() {
  SDL_FreeSurface(canvas_surface);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	window = NULL;
	renderer = NULL;
	SDL_Quit();
}

int loop(int argc, char **argv) {
	Uint32 t0;  /* ms */
	Uint8  bp;  /* buttons pressed */
  if (!init(&bp)) {
    while (!game_over) {
		  t0 = SDL_GetTicks();
      if (render()) {
        break;
      };
		  ctrl_listen(&bp);
		  SDL_Delay(50);
	  }
	  close();
  }
	return 0;
}
