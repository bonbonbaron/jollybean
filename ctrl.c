#include "ge.h"


void ctrl_listen() {
  extern SDL_bool game_over;
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			SDL_Quit();
			return;
		}
		else if (event.type == SDL_KEYUP) {
			switch(event.key.keysym.sym) {
			case SDLK_a:
				//bp &= ~CTRL_L;
        cancel(CTRL_L);
				break;
			case SDLK_s:
				//bp &= ~CTRL_D;
        cancel(CTRL_D);
				break;
			case SDLK_w:
				//bp &= ~CTRL_U;
        cancel(CTRL_U);
				break;
			case SDLK_d:
				//bp &= ~CTRL_R;
        cancel(CTRL_R);
				break;
			case SDLK_j:
        //bp &= ~CTRL_A;
        cancel(CTRL_A);
				break;
			case SDLK_k:
        //bp &= ~CTRL_B;
        cancel(CTRL_B);
				break;
			case SDLK_x:
        //bp &= ~CTRL_X;
        cancel(CTRL_X);
				break;
			case SDLK_SPACE:
				//bp &= ~CTRL_S;
        cancel(CTRL_S);
				break;
			case SDLK_ESCAPE:
        //bp = 0xff;
        game_over = SDL_TRUE;
				SDL_Quit();
				return;
			}
		}
		else if (event.type == SDL_KEYDOWN) {
			switch(event.key.keysym.sym) {
      case SDLK_a:
        //bp &= ~CTRL_L;
        broadcast(CTRL_L);
        break;
      case SDLK_s:
        //bp &= ~CTRL_D;
        broadcast(CTRL_D);
        break;
      case SDLK_w:
        //bp &= ~CTRL_U;
        broadcast(CTRL_U);
        break;
      case SDLK_d:
        //bp &= ~CTRL_R;
        broadcast(CTRL_R);
        break;
      case SDLK_j:
        //bp &= ~CTRL_A;
        broadcast(CTRL_A);
        break;
      case SDLK_k:
        //bp &= ~CTRL_B;
        broadcast(CTRL_B);
        break;
      case SDLK_x:
        //bp &= ~CTRL_X;
        broadcast(CTRL_X);
        break;
      case SDLK_SPACE:
        //bp &= ~CTRL_S;
        broadcast(CTRL_S);
        break;
      case SDLK_ESCAPE:
        //bp = 0xff;
        game_over = SDL_TRUE;
        SDL_Quit();
        return;
      }
		}
	}
}