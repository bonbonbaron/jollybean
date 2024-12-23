#include "x/xRender.h"
#include "jb.h"
#include "interface/interface.h"
#include "blehColormap.h"
#include "blehColorPalette.h"
#include "heckColormap.h"
#include "heckColorPalette.h"
#include "redColormap.h"
#include "redColorPalette.h"

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
        genePA[i]->u.unitary.key = 0;
        genePA[i]->u.unitary.size = sizeof(XRenderComp);
        genePA[i]->u.unitary.type = RENDER;
      }
    }
    genePA[0]->u.unitary.dataP = &blehColormap;
    genePA[1]->u.unitary.dataP = &redColormap;
    genePA[2]->u.unitary.dataP = &heckColormap;
  }

  // Extract stripdata from each media gene into an array of contiguous strip data pointers.
  StripDataS **sdPA = NULL;
  if (!e) {
    e = arrayNew((void**) &sdPA, sizeof(StripDataS*), arrayGetNElems(genePA));
  }
  if (!e) {
    for (int i = 0, iEnd = arrayGetNElems(sdPA); i < iEnd; ++i) {
      sdPA[i] = *((StripDataS**) genePA[i]->u.unitary.dataP);
    }
  }

  U32 sum = 0;
  for (StripDataS **sdPP = sdPA; sdPP < sdPA + arrayGetNElems(sdPA); ++sdPP) {
    printf("sm inf sz: %d\n", (*sdPP)->sm.infP->inflatedLen);
    printf("ss inf sz: %d\n", (*sdPP)->ss.infP->inflatedLen);
    sum += (*sdPP)->ss.infP->inflatedLen;
    sum += (*sdPP)->sm.infP->inflatedLen;
  }
  printf("sum: %d\n", sum);

  // Inflate colormap inflatables
#if 1
  if (!e) {
    e = multithread_(sdInflate, (void*) sdPA);
  }
  // Unpack stripsets
  if (!e) {
    e = multithread_(sdUnpack, (void*) sdPA);
  }
  // Assemble colormaps from strips
  if (!e) {
    e = multithread_(sdAssemble, (void*) sdPA);
  }
#endif
  // Deflate colormap inflatables
  if (sdPA) {
    //e = multithread_(stripClr,   (void*) sdPA);
    for (StripDataS **sdPP = sdPA; !e && sdPP < sdPA + arrayGetNElems(sdPA); ++sdPP) {
#if 0
      arrayDel((void**)&(*sdPP)->assembledDataA);
      arrayDel((void**)&(*sdPP)->ss.unpackedDataP);
      free((void**)(*sdPP)->ss.infP->inflatedDataP);
      free((void**)(*sdPP)->sm.infP->inflatedDataP);
#elif 0
      e = stripIni(*sdPP);
      stripClr(*sdPP);
#elif 1
      inflatableIni((*sdPP)->sm.infP);
      inflatableClr((*sdPP)->sm.infP);
#else
      jbAlloc(&(*sdPP)->sm.infP->inflatedDataP, (*sdPP)->sm.infP->inflatedLen, 1);
      jbFree(&(*sdPP)->sm.infP->inflatedDataP);
#endif
    }
  }

  arrayDel((void**) &sdPA);
  for (int i = 0; i < arrayGetNElems(genePA); ++i) {
    jbFree((void**) &genePA[i]);
  }
  arrayDel((void**) &genePA);

  return e;
}
