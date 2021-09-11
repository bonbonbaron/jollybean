#include "jb.h"


void ctrl_listen(Uint8 *bp_p) {
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
				*bp_p &= ~CTRL_L;
				break;
			case SDLK_s:
				*bp_p &= ~CTRL_D;
				break;
			case SDLK_w:
				*bp_p &= ~CTRL_U;
				break;
			case SDLK_d:
				*bp_p &= ~CTRL_R;
				break;
			case SDLK_j:
        *bp_p &= ~CTRL_A;
				break;
			case SDLK_k:
        *bp_p &= ~CTRL_B;
				break;
			case SDLK_x:
        *bp_p &= ~CTRL_X;
				break;
			case SDLK_SPACE:
				*bp_p &= ~CTRL_S;
				break;
			case SDLK_ESCAPE:
        *bp_p = 0xff;
				SDL_Quit();
				return;
			}
		}
		else if (event.type == SDL_KEYDOWN) {
			switch(event.key.keysym.sym) {
      case SDLK_a:
        *bp_p &= ~CTRL_L;
        break;
      case SDLK_s:
        *bp_p &= ~CTRL_D;
        break;
      case SDLK_w:
        *bp_p &= ~CTRL_U;
        break;
      case SDLK_d:
        *bp_p &= ~CTRL_R;
        break;
      case SDLK_j:
        *bp_p &= ~CTRL_A;
        break;
      case SDLK_k:
        *bp_p &= ~CTRL_B;
        break;
      case SDLK_x:
        *bp_p &= ~CTRL_X;
        break;
      case SDLK_SPACE:
        *bp_p &= ~CTRL_S;
        break;
      case SDLK_ESCAPE:
        *bp_p = 0xff;
        SDL_Quit();
        return;
      }
		}
	}
}
