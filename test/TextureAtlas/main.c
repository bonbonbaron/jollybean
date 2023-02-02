#include "data.h"
#include "xRender.h"
#include "blehColormap.h"
#include "blehColorPalette.h"
#include "heckColormap.h"
#include "heckColorPalette.h"
#include "redColormap.h"
#include "redColorPalette.h"
#include "previewImg.h"

/* Coding Kaizen:
 *  1) Code only a little bit at a time then build to make sure it compiles.
 *  2) Test every individual function you write.
 */
static void* mtColormapInflate(ThreadFuncArg *thargP) {
  Colormap **cmPP = ((Colormap**) thargP->array) + thargP->startIdx;
  Colormap **cmEndPP = cmPP + thargP->nElemsToProcess;
  for (Error e = SUCCESS; !e && cmPP < cmEndPP; ++cmPP) {
    pthread_mutex_lock(&(*cmPP)->sdP->lock);
    e = inflatableIni((*cmPP)->sdP->ss.infP);
    if (!e) {
      e = inflatableIni((*cmPP)->sdP->sm.infP);
    }
    pthread_mutex_unlock(&(*cmPP)->sdP->lock);
  }
}

static void* mtColormapStripClr(ThreadFuncArg *thargP) {
  Colormap **cmPP = ((Colormap**) thargP->array) + thargP->startIdx;
  Colormap **cmEndPP = cmPP + thargP->nElemsToProcess;
  for (; cmPP < cmEndPP; ++cmPP) {
    pthread_mutex_lock(&(*cmPP)->sdP->lock);
    stripClr((*cmPP)->sdP);
    pthread_mutex_unlock(&(*cmPP)->sdP->lock);
  }
}

static void* mtColormapStripUnpack(ThreadFuncArg *thargP) {
  Colormap **cmPP = ((Colormap**) thargP->array) + thargP->startIdx;
  Colormap **cmEndPP = cmPP + thargP->nElemsToProcess;
  for (Error e = SUCCESS; !e && cmPP < cmEndPP; ++cmPP) {
    pthread_mutex_lock(&(*cmPP)->sdP->lock);
    e = stripsetUnpack(&(*cmPP)->sdP->ss);
    pthread_mutex_unlock(&(*cmPP)->sdP->lock);
  }
}

static void* mtColormapNewArray(ThreadFuncArg *thargP) {
  Colormap **cmPP = ((Colormap**) thargP->array) + thargP->startIdx;
  Colormap **cmEndPP = cmPP + thargP->nElemsToProcess;
  for (Error e = SUCCESS; !e && cmPP < cmEndPP; ++cmPP) {
    pthread_mutex_lock(&(*cmPP)->sdP->lock);
    e = arrayNew((void**) &(*cmPP)->sdP->unstrippedDataA, 
                 sizeof(U8), 
                 (*cmPP)->sdP->sm.nIndices * (*cmPP)->sdP->ss.nUnitsPerStrip);
    pthread_mutex_unlock(&(*cmPP)->sdP->lock);
  }
}

static void* mtColormapStripAssemble(ThreadFuncArg *thargP) {
  Colormap **cmPP = ((Colormap**) thargP->array) + thargP->startIdx;
  Colormap **cmEndPP = cmPP + thargP->nElemsToProcess;
  for (Error e = SUCCESS; !e && cmPP < cmEndPP; ++cmPP) {
    pthread_mutex_lock(&(*cmPP)->sdP->lock);
    e = stripAssemble((*cmPP)->sdP);
    pthread_mutex_unlock(&(*cmPP)->sdP->lock);
  }
}

int main(int argc, char **argv) {
  Colormap **colormapPA = NULL;

  Error e = arrayNew((void**) &colormapPA, sizeof(void*), 109);

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
    colormapPA[9] = &redColormap;
    colormapPA[10] = &redColormap;
    colormapPA[11] = &redColormap;
    colormapPA[12] = &redColormap;
    colormapPA[13] = &redColormap;
    colormapPA[14] = &redColormap;
    colormapPA[15] = &redColormap;
    colormapPA[16] = &redColormap;
    colormapPA[17] = &redColormap;
    colormapPA[18] = &redColormap;
    colormapPA[19] = &redColormap;
    colormapPA[20] = &redColormap;
    colormapPA[21] = &redColormap;
    colormapPA[22] = &redColormap;
    colormapPA[23] = &redColormap;
    colormapPA[24] = &redColormap;
    colormapPA[25] = &redColormap;
    colormapPA[26] = &redColormap;
    colormapPA[27] = &redColormap;
    colormapPA[28] = &redColormap;
    colormapPA[29] = &redColormap;
    colormapPA[30] = &redColormap;
    colormapPA[31] = &redColormap;
    colormapPA[32] = &redColormap;
    colormapPA[33] = &redColormap;
    colormapPA[34] = &redColormap;
    colormapPA[35] = &redColormap;
    colormapPA[36] = &redColormap;
    colormapPA[37] = &redColormap;
    colormapPA[38] = &redColormap;
    colormapPA[39] = &redColormap;
    colormapPA[40] = &redColormap;
    colormapPA[41] = &redColormap;
    colormapPA[42] = &redColormap;
    colormapPA[43] = &redColormap;
    colormapPA[44] = &redColormap;
    colormapPA[45] = &redColormap;
    colormapPA[46] = &redColormap;
    colormapPA[47] = &redColormap;
    colormapPA[48] = &redColormap;
    colormapPA[49] = &redColormap;
    colormapPA[50] = &redColormap;
    colormapPA[51] = &redColormap;
    colormapPA[52] = &redColormap;
    colormapPA[53] = &redColormap;
    colormapPA[54] = &redColormap;
    colormapPA[55] = &redColormap;
    colormapPA[56] = &redColormap;
    colormapPA[57] = &redColormap;
    colormapPA[58] = &redColormap;
    colormapPA[59] = &redColormap;
    colormapPA[60] = &redColormap;
    colormapPA[61] = &redColormap;
    colormapPA[62] = &redColormap;
    colormapPA[63] = &redColormap;
    colormapPA[64] = &redColormap;
    colormapPA[65] = &redColormap;
    colormapPA[66] = &redColormap;
    colormapPA[67] = &redColormap;
    colormapPA[68] = &redColormap;
    colormapPA[69] = &redColormap;
    colormapPA[70] = &redColormap;
    colormapPA[71] = &redColormap;
    colormapPA[72] = &redColormap;
    colormapPA[73] = &redColormap;
    colormapPA[74] = &redColormap;
    colormapPA[75] = &redColormap;
    colormapPA[76] = &redColormap;
    colormapPA[77] = &redColormap;
    colormapPA[78] = &redColormap;
    colormapPA[79] = &redColormap;
    colormapPA[80] = &redColormap;
    colormapPA[81] = &redColormap;
    colormapPA[82] = &redColormap;
    colormapPA[83] = &redColormap;
    colormapPA[84] = &redColormap;
    colormapPA[85] = &redColormap;
    colormapPA[86] = &redColormap;
    colormapPA[87] = &redColormap;
    colormapPA[88] = &redColormap;
    colormapPA[89] = &redColormap;
    colormapPA[90] = &redColormap;
    colormapPA[91] = &redColormap;
    colormapPA[92] = &redColormap;
    colormapPA[93] = &redColormap;
    colormapPA[94] = &redColormap;
    colormapPA[95] = &redColormap;
    colormapPA[96] = &redColormap;
    colormapPA[97] = &redColormap;
    colormapPA[98] = &redColormap;
    colormapPA[99] = &redColormap;
    colormapPA[100] = &redColormap;
    colormapPA[101] = &redColormap;
    colormapPA[102] = &redColormap;
    colormapPA[103] = &redColormap;
    colormapPA[104] = &redColormap;
    colormapPA[105] = &redColormap;
    colormapPA[106] = &redColormap;
    colormapPA[107] = &redColormap;
    colormapPA[108] = &redColormap;
  }
  // Inflate colormap inflatables
  if (!e) {
    e = multiThread(mtColormapInflate,       colormapPA);
  }
  // Unpack stripsets
  if (!e) {
    e = multiThread(mtColormapStripUnpack,   colormapPA);
  }
  // Allocate colormap arrays before populating them
  if (!e) {
    e = multiThread(mtColormapNewArray,      colormapPA);
  }
  // Assemble colormaps from strips
  if (!e) {
    e = multiThread(mtColormapStripAssemble, colormapPA);
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
    e = multiThread(mtColormapStripClr,      colormapPA);
  }

  arrayDel((void**) &colormapPA);

  return e;
}
