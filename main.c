#include "ge.h"
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
SDL_bool signals_exist = SDL_FALSE;

#define BUTTON_REPEAT_RATE 50
Error update() {
  Error stat = 0;
  Uint8 i;
  static LinkedList active_subscribers = {NULL};
  ListNode *ListNode;
  Subscriber *subscriber;

  /* If signals exist, alert their subscribers. */
  if (signals_exist) {
    for (i = 0; i < NUM_COMM_CHANNELS; i++) {
      if (comm_channels[i].signal_on && comm_channels[i].subscribers != NULL) {
        for (ListNode = comm_channels[i].subscribers->head; ListNode != NULL; ListNode = ListNode->next) {
          subscriber = ListNode->data;
          if (!subscriber->busy) {
            stat = trigger_reaction(subscriber, i, &active_subscribers);  /* i is both the channel and the signal. */
            if (stat) {
              goto break_out_of_nested_loop;
            }
          }
        }
      }
      comm_channels[i].signal_on = SDL_FALSE;
    }
    signals_exist = SDL_FALSE;
  }

  /* If collisions exist, alert the sprites collided. */
  /* TODO */

  /* Run active_subscribers. */
  for (ListNode = active_subscribers.head; ListNode != NULL; ListNode = ListNode->next) {
    subscriber = ListNode->data;
    subscriber->s->reaction_bookmark += subscriber->activity.current_reaction_sequence->react_func_ptr[subscriber->s->reaction_bookmark](subscriber->s, subscriber->activity.tgt, scene);
    /* If subscriber's reaction sequence is finished, reset the index to zero. It's still busy if the sequence is meant to loop. */
    if (subscriber->s->reaction_bookmark >= subscriber->activity.current_reaction_sequence->num_reactions) {
      subscriber->s->reaction_bookmark = 0;
      subscriber->busy = subscriber->activity.current_reaction_sequence->repeat;
    }
  }

break_out_of_nested_loop:
  return stat;
}

Error render() {
	Error stat = 0;
	Uint16 i;
  SDL_Rect rect;
  SDL_RenderClear(renderer);

  for (i = 0; i < scene->num_sprites; i++) {
    /* Scene's sprites should always be ordered with onscreen ones first. */
    if (!(scene->sprites[i]->onscreen)) {
      break;
    }
    rect.x = scene->sprites[i]->blit_coords.x - scene->camera.rect.x;
    rect.y = scene->sprites[i]->blit_coords.y - scene->camera.rect.y;
    rect.w = scene->sprites[i]->blit_coords.w;
    rect.h = scene->sprites[i]->blit_coords.h;
    stat = SDL_BlitSurface(scene->sprites[i]->surface, &rect, canvas_surface, NULL);
    if (stat) {
      printf("render error\n");
      return 1;
    }
  }
  
	SDL_UpdateWindowSurface(window);
  return stat;
}

Error init() {
  Error stat = 0;
  extern Scene *scene_zero;

  /* Communication Channels */
  memset(comm_channels, 0, sizeof(CommChannel) * NUM_COMM_CHANNELS);
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

int main(int argc, char **argv) {
	Uint32 t0;  /* ms */
  SDL_bool game_over = SDL_FALSE;
  if (!init()) {
    while (!game_over) {
		  t0 = SDL_GetTicks();
      if (render()) {
        break;
      };
		  ctrl_listen();
      update();
		  // if ((delay = SDL_GetTicks() - t0) < FRAME_DURATION)
			  // SDL_Delay(delay);
		  SDL_Delay(50);
	  }
	  close();
  }
	return 0;
}											