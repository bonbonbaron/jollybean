#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "data.h"
#include "interface.h"

U32 buttonsPressed = 0;

typedef enum {KEY_UP, KEY_DOWN} KeyEvent;

// TODO_: Assume nothing. Give dev the whole keyboard. Instead, map these to a bitfield.


//======================================================
// System definition
//======================================================
int main(int argc, char **argv) {
  Gui *guiP = NULL;
  Error e= guiNew(&guiP);
  U32 oldButtons = guiP->buttonsPressed;
  if (!e) {
    while (e != E_QUIT) {
      SDL_Delay(20);
      e = guiProcessEvents(guiP);
      if (guiP->buttonsPressed != oldButtons) {
        printf("button state: 0x%08x\n", guiP->buttonsPressed);
        oldButtons = guiP->buttonsPressed;
      }
    }
  }
  guiDel(&guiP);
  return e;
}
