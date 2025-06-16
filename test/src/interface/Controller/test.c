#include <stdio.h>
#include "interface/interface.h"

int main(int argc, char **argv) {
  Gui *guiP = guiNew();
  U32 oldButtons = guiP->buttonsPressed;
  int counter = 0;
  while (guiP->state != QUIT) {
    SDL_Delay(20);
    guiProcessEvents(guiP);
    printf("%d: state is %d\n", counter++, guiP->state);
    if (guiP->buttonsPressed != oldButtons) {
      printf("button state: 0x%08x\n", guiP->buttonsPressed);
      oldButtons = guiP->buttonsPressed;
    }
  }
  guiDel(&guiP);
  return 0;
}
