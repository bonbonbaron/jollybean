#include "xControl.h"

U32 buttonsPressed;

typedef enum {KEY_UP, KEY_DOWN} KeyEvent;

inline static Error _alertFocusToKeyEvent(XControl *xControlSysP, KeyEvent keyEvent) {
  return mailboxWrite(xControlSysP->system.outboxF, 0, xControlSysP->focus, SWITCH_AND_ACTIVATE, keyEvent);
}

// TODO_: Assume nothing. Give dev the whole keyboard. Instead, map these to a bitfield.
Error xControlRun(System *sP) {
  XControl *xControlSysP = (XControl*) sP;
	Event_ event;
  Error e = SUCCESS;
	while (pollEvent_(&event)) {
		if (event.type == EVENT_QUIT_) {
      return mailboxWrite(sP->outboxF, MASTER, 0, QUIT_ALL, QUIT_ALL);
		}
		else if (event.type == EVENT_KEYUP_) {
			switch(event.key.keysym.sym) {
			case KEY_a_:
				buttonsPressed &= ~CTRL_LEFT_;
        e = _alertFocusToKeyEvent(xControlSysP, KD_(LEFT_));
				break;
			case KEY_s_:
				buttonsPressed &= ~CTRL_DOWN_;
        e = _alertFocusToKeyEvent(xControlSysP, KD_(DOWN_));
				break;
			case KEY_w_:
				buttonsPressed &= ~CTRL_UP_;
        e = _alertFocusToKeyEvent(xControlSysP, KD_(UP_));
				break;
			case KEY_d_:
				buttonsPressed &= ~CTRL_RIGHT_;
        e = _alertFocusToKeyEvent(xControlSysP, KD_(RIGHT_));
				break;
			case KEY_j_:
        buttonsPressed &= ~CTRL_A_;
        e = _alertFocusToKeyEvent(xControlSysP, KD_(A_));
				break;
			case KEY_k_:
        buttonsPressed &= ~CTRL_B_;
        e = _alertFocusToKeyEvent(xControlSysP, KD_(B_));
				break;
			case KEY_SPACE_:
				buttonsPressed &= ~CTRL_START_;
        e = _alertFocusToKeyEvent(xControlSysP, KD_(START_));
				break;
			case KEY_LSHIFT_:
        buttonsPressed &= ~CTRL_SELECT_;
        e = _alertFocusToKeyEvent(xControlSysP, KD_(SELECT_));
				break;
			case KEY_ESCAPE_:
        return mailboxWrite(sP->outboxF, MASTER, 0, QUIT_ALL, QUIT_ALL);
			}
		}
		else if (event.type == EVENT_KEYDOWN_) {
			switch(event.key.keysym.sym) {
      case KEY_a_:
        buttonsPressed &= ~CTRL_LEFT_;
        e = _alertFocusToKeyEvent(xControlSysP, KD_(LEFT_));
        break;
      case KEY_s_:
        buttonsPressed &= ~CTRL_DOWN_;
        e = _alertFocusToKeyEvent(xControlSysP, KD_(DOWN_));
        break;
      case KEY_w_:
        buttonsPressed &= ~CTRL_UP_;
        e = _alertFocusToKeyEvent(xControlSysP, KD_(UP_));
        break;
      case KEY_d_:
        buttonsPressed &= ~CTRL_RIGHT_;
        e = _alertFocusToKeyEvent(xControlSysP, KD_(RIGHT_));
        break;
      case KEY_j_:
        buttonsPressed &= ~CTRL_A_;
        e = _alertFocusToKeyEvent(xControlSysP, KD_(A_));
        break;
      case KEY_k_:
        buttonsPressed &= ~CTRL_B_;
        e = _alertFocusToKeyEvent(xControlSysP, KD_(B_));
        break;
      case KEY_SPACE_:
        buttonsPressed &= ~CTRL_START_;
        e = _alertFocusToKeyEvent(xControlSysP, KD_(START_));
        break;
      case KEY_LSHIFT_:
        buttonsPressed &= ~CTRL_SELECT_;
        e = _alertFocusToKeyEvent(xControlSysP, KD_(SELECT_));
        break;
      case KEY_ESCAPE_:
        return mailboxWrite(sP->outboxF, MASTER, 0, QUIT_ALL, QUIT_ALL);
      }
		}
	}
	return e;
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
