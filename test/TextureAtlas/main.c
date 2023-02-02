#include "xRender.h"
#include "blehColormap.h"
#include "blehColorPalette.h"
#include "heckColormap.h"
#include "heckColorPalette.h"
#include "redColormap.h"
#include "redColorPalette.h"
#include "previewImg.h"

static Error _sdInflate(void *cmP) {
  Error e = inflatableIni(((Colormap*) cmP)->sdP->ss.infP);
  if (!e) {
    e = inflatableIni(((Colormap*) cmP)->sdP->sm.infP);
  }
  return e;
}

static Error _sdClr(void *cmP) {
  stripClr(((Colormap*) cmP)->sdP);
  return SUCCESS;
}

static Error _ssUnpack(void *cmP) {
  Error e = stripsetUnpack(&((Colormap*) cmP)->sdP->ss);
  return e;
}
  
static Error _sdAssemble(void *cmP) {
  Error e = arrayNew((void**) &((Colormap*) cmP)->sdP->unstrippedDataA, sizeof(U8), 
               ((Colormap*) cmP)->sdP->sm.nIndices * ((Colormap*) cmP)->sdP->ss.nUnitsPerStrip);
  if (!e) {
    e = stripAssemble(((Colormap*) cmP)->sdP);
  }
  return e;
}

int main(int argc, char **argv) {
  Colormap **colormapPA = NULL;

  Error e = arrayNew((void**) &colormapPA, sizeof(void*), 9);

  if (!e) {
    colormapPA[0] = &blehColormap;
    colormapPA[1] = &heckColormap;
    colormapPA[2] = &redColormap;
    colormapPA[3] = &redColormap;
    colormapPA[4] = &redColormap;
    colormapPA[5] = &redColormap;
    colormapPA[6] = &redColormap;
    colormapPA[7] = &redColormap;
    colormapPA[8] = &redColormap;
  }

  // Inflate colormap inflatables
  if (!e) {
    e = multiThread(_sdInflate, 0, colormapPA);
  }
  // Unpack stripsets
  if (!e) {
    e = multiThread(_ssUnpack, 1, colormapPA);
  }
  // Assemble colormaps from strips
  if (!e) {
    e = multiThread(_sdAssemble, 2, colormapPA);
  }
  if (!e) {
    e = previewImg(&blehColormap, &blehColorPalette, 1000);
  }
  if (!e) {
    e = previewImg(&redColormap, &redColorPalette, 1000);
  }
  if (!e) {
    e = previewImg(&heckColormap, &heckColorPalette, 1000);
  }
  // Deflate colormap inflatables
  if (!e) {
    e = multiThread(_sdClr,   3,   colormapPA);
  }

  arrayDel((void**) &colormapPA);

  return e;
}
