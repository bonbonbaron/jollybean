#include "xRender.h"
#include "blehColormap.h"
#include "blehColorPalette.h"
#include "heckColormap.h"
#include "heckColorPalette.h"
#include "redColormap.h"
#include "redColorPalette.h"
#include "previewImg.h"
int main(int argc, char **argv) {
  Gene **genePA = NULL;

  Error e = arrayNew((void**) &genePA, sizeof(void*), 3);

  // Mock up some genes.
  if (!e) {
    for (int i = 0; !e && i < 3; ++i) {
      Gene *geneP = NULL;
      e = jbAlloc((void**) &geneP, sizeof(Gene), 1);
      if (!e) {
        genePA[i] = geneP;
        genePA[i]->geneClass = MEDIA_GENE;
        genePA[i]->key = 0;
        genePA[i]->size = sizeof(XRenderComp);
        genePA[i]->type = RENDER;
      }
    }
    genePA[0]->dataP = &blehColormap;
    genePA[1]->dataP = &redColormap;
    genePA[2]->dataP = &heckColormap;
  }

  // Extract stripdata from each media gene into an array of contiguous strip data pointers.
  StripDataS **sdPA = NULL;
  if (!e) {
    e = arrayNew((void**) &sdPA, sizeof(StripDataS*), arrayGetNElems(genePA));
  }
  if (!e) {
    for (int i = 0, iEnd = arrayGetNElems(sdPA); i < iEnd; ++i) {
      sdPA[i] = *((StripDataS**) genePA[i]->dataP);
    }
  }

  // Inflate colormap inflatables
  if (!e) {
    e = multiThread(sdInflate, (void*) sdPA);
  }
  // Unpack stripsets
  if (!e) {
    e = multiThread(sdUnpack, (void*) sdPA);
  }
  // Assemble colormaps from strips
  if (!e) {
    e = multiThread(sdAssemble, (void*) sdPA);
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
    e = multiThread(stripClr,   (void*) sdPA);
  }

  arrayDel((void**) &sdPA);
  for (int i = 0; i < arrayGetNElems(genePA); ++i) {
    jbFree((void**) &genePA[i]);
  }
  arrayDel((void**) &genePA);

  return e;
}
