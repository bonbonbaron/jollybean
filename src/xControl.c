#include "xControl.h"

U32 buttonsPressed;

// TODO_: Assume nothing. Give dev the whole keyboard. Instead, map these to a bitfield.
Error xControlRun(System *sP) {
	Event_ event;
	while (pollEvent_(&event)) {
		if (event.type == EVENT_QUIT_) {
      return mailboxWrite(sP->outboxF, MASTER, 0, QUIT_ALL, QUIT_ALL);
		}
		else if (event.type == EVENT_KEYUP_) {
			switch(event.key.keysym.sym) {
			case KEY_a_:
				buttonsPressed &= ~CTRL_LEFT_;
				break;
			case KEY_s_:
				buttonsPressed &= ~CTRL_DOWN_;
				break;
			case KEY_w_:
				buttonsPressed &= ~CTRL_UP_;
				break;
			case KEY_d_:
				buttonsPressed &= ~CTRL_RIGHT_;
				break;
			case KEY_j_:
        buttonsPressed &= ~CTRL_A_;
				break;
			case KEY_k_:
        buttonsPressed &= ~CTRL_B_;
				break;
			case KEY_SPACE_:
				buttonsPressed &= ~CTRL_START_;
				break;
			case KEY_LSHIFT_:
        buttonsPressed &= ~CTRL_SELECT_;
				break;
			case KEY_ESCAPE_:
        return mailboxWrite(sP->outboxF, MASTER, 0, QUIT_ALL, QUIT_ALL);
			}
		}
		else if (event.type == EVENT_KEYDOWN_) {
			switch(event.key.keysym.sym) {
      case KEY_a_:
        buttonsPressed &= ~CTRL_LEFT_;
        break;
      case KEY_s_:
        buttonsPressed &= ~CTRL_DOWN_;
        break;
      case KEY_w_:
        buttonsPressed &= ~CTRL_UP_;
        break;
      case KEY_d_:
        buttonsPressed &= ~CTRL_RIGHT_;
        break;
      case KEY_j_:
        buttonsPressed &= ~CTRL_A_;
        break;
      case KEY_k_:
        buttonsPressed &= ~CTRL_B_;
        break;
      case KEY_SPACE_:
        buttonsPressed &= ~CTRL_START_;
        break;
      case KEY_LSHIFT_:
        buttonsPressed &= ~CTRL_SELECT_;
        break;
      case KEY_ESCAPE_:
        return mailboxWrite(sP->outboxF, MASTER, 0, QUIT_ALL, QUIT_ALL);
      }
		}
	}
	return SUCCESS;
}

Error xControlProcessMessage(System *sP, Message *msgP) {
	unused_(sP);
	unused_(msgP);
	return SUCCESS;
}

//TODO
Error xControlClr(System *sP) {
  unused_(sP);
  return SUCCESS;
}

Error xControlIniSys() {
	return SUCCESS;
}

XIniCompFuncDef_(Control) {
  unused_(sP);
	unused_(dataP);
	return SUCCESS;
}

XGetShareFuncDefUnused_(Control);

//======================================================
// System definition
//======================================================
X_(Control, 5, FLG_NO_SWITCHES_ | FLG_NO_CHECKS_ | FLG_NO_CF_SRC_A_);
