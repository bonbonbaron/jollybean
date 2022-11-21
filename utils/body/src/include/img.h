#ifndef IMG_H_
#define IMG_H_
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <png.h>
#include <assert.h>
#include <string.h>
#include "xRender.h"
#include "inflatable.h"
#include "fileUtils.h"
#include <stdlib.h>
#include "strip.h"
#include "dir.h"
#include "genie.h"

//const char LOCAL_TROVE_IMAGE_DIR[] = "/jb/src/Image/";
const char TROVE_IMG_OBJ_DIR[] = "/jb/build/Image/obj/";
const char TROVE_IMG_SRC_DIR[] = "/jb/build/Image/src/";
const char TROVE_IMG_INC_DIR[] = "/jb/build/Image/include/";

typedef struct {
  U32 w, h, pitch, bpp;
} ImgDims;

typedef png_color Pixel;
typedef U8 ColormapIdx;

void getBaseNameIndices(char *filepathP, char *extension, U32 *startIdxP, U32 *endIdxP);
void imgDimsIni(ImgDims *imgDimsP, U32 width, U32 height, U32 bpp);
S32 getColormapIdx(U8 *srcPixelA, U32 nPixels, const U8 srcPixelSize, const U8 *srcPixelQueryP);
Error readPng(png_image **imgPP, char *imgPathA, U8 *pixelSizeP, U8 **pixelAP, U8 **colorPalettePP, U8 verbose);
Error getColorPaletteAndColormap(U8 **colorPaletteAP, U8 **colormapAP, U32 *nColorsP, png_image *pngImgP, U8 *pixelP, U8 maxNColorsAllowed, U8 srcPixelSize, U8 verbose);
Error makeColorPaletteAndColormap(U8 **paletteAP, U8 **colormapPP, U32 *nColorsP, U8 srcPixelSize, png_image *srcImgDataP, U8 *srcImgP, U8 verbose);
#endif
