#include "xControl.h"

U32 buttonsPressed;

// TODO: Assume nothing. Give dev the whole keyboard. Instead, map these to a bitfield.
Error ctrlListen(Focus *fP) {
	unused_(fP);
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			SDL_Quit();
			return SUCCESS;
		}
		else if (event.type == SDL_KEYUP) {
			switch(event.key.keysym.sym) {
			case SDLK_a:
				buttonsPressed &= ~CTRL_LEFT;
				break;
			case SDLK_s:
				buttonsPressed &= ~CTRL_DOWN;
				break;
			case SDLK_w:
				buttonsPressed &= ~CTRL_UP;
				break;
			case SDLK_d:
				buttonsPressed &= ~CTRL_RIGHT;
				break;
			case SDLK_j:
        buttonsPressed &= ~CTRL_A;
				break;
			case SDLK_k:
        buttonsPressed &= ~CTRL_B;
				break;
			case SDLK_SPACE:
				buttonsPressed &= ~CTRL_START;
				break;
			case SDLK_LSHIFT:
        buttonsPressed &= ~CTRL_SELECT;
				break;
			case SDLK_ESCAPE:
        buttonsPressed = 0xffffffff;
				SDL_Quit();
				return SUCCESS;
			}
		}
		else if (event.type == SDL_KEYDOWN) {
			switch(event.key.keysym.sym) {
      case SDLK_a:
        buttonsPressed &= ~CTRL_LEFT;
        break;
      case SDLK_s:
        buttonsPressed &= ~CTRL_DOWN;
        break;
      case SDLK_w:
        buttonsPressed &= ~CTRL_UP;
        break;
      case SDLK_d:
        buttonsPressed &= ~CTRL_RIGHT;
        break;
      case SDLK_j:
        buttonsPressed &= ~CTRL_A;
        break;
      case SDLK_k:
        buttonsPressed &= ~CTRL_B;
        break;
      case SDLK_SPACE:
        buttonsPressed &= ~CTRL_START;
        break;
      case SDLK_LSHIFT:
        buttonsPressed &= ~CTRL_SELECT;
        break;
      case SDLK_ESCAPE:
        buttonsPressed = 0xff;
        SDL_Quit();
        return SUCCESS;
      }
		}
	}
	return SUCCESS;
}

Error xControlIniSys() {
	return SUCCESS;
}

Error xControlIniComp(XHeader *xhP) {
	unused_(xhP);
	return SUCCESS;
}

//======================================================
// System definition
//======================================================
System_(Control, CONTROL, Focus_(CTRL_LISTEN, ctrlListen));
