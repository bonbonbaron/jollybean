#include "xControl.h"
#include "x.h"
#include "xMaster.h"

U32 buttonsPressed = 0;

typedef enum {KEY_UP, KEY_DOWN} KeyEvent;

// TODO_: Assume nothing. Give dev the whole keyboard. Instead, map these to a bitfield.
Error xControlRun(System *sP) {
  XControl *xControlSysP = (XControl*) sP;
	Event_ event;
  Error e = SUCCESS;
	while (pollEvent_(&event)) {
    switch (event.type) {
    case EVENT_QUIT_:
      return E_QUIT;
		case EVENT_KEYUP_:
			switch(event.key.keysym.sym) {
			case KEY_a_:
				buttonsPressed &= ~CTRL_LEFT;
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
        return mailboxWrite(sP->mailboxF, MASTER_, 0, QUIT_ALL, QUIT_ALL);
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
        return mailboxWrite(sP->mailboxF, MASTER_, 0, QUIT_ALL, QUIT_ALL);
      }
		}
	}
	return e;
}

XProcMsgFuncDefUnused_(Control);
XClrFuncDefUnused_(Control);
XIniSysFuncDefUnused_(Control);
XIniSubcompFuncDefUnused_(Control);
XGetShareFuncDefUnused_(Control);

//======================================================
// System definition
//======================================================
X_(Control, CONTROL, FLG_NO_SWITCHES_ | FLG_NO_CHECKS_ | FLG_NO_CF_SRC_A_);
