#include "xRender.h"
#include "previewImg.h"
#include "redColormap.h"
#include "redColorPalette.h"

// Dependencies: xRender.o, x.o, data.o, and interface.o
int main(int argc, char  **argv) {
  Error e = cmGen(&redColormap);
  if (!e) {
    e = previewImg(&redColormap, &redColorPalette, 2000);
  }
  cmClr(&redColormap);
  return e;
}
