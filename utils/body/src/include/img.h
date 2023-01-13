#ifndef IMG_H_
#define IMG_H_
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <png.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "xRender.h"
#include "inflatable.h"
#include "fileUtils.h"
#include "strip.h"
#include "db.h"
#include "genie.h"

//const char LOCAL_TROVE_IMAGE_DIR[] = "/jb/src/Image/";
extern char TROVE_IMG_OBJ_DIR[];
extern char TROVE_IMG_SRC_DIR[];
extern char TROVE_IMG_INC_DIR[];

typedef struct {
  U32 w, h, pitch, bpp;
} ImgDims;

typedef png_color Pixel;

void getBaseNameIndices(char *filepathP, char *extension, U32 *startIdxP, U32 *endIdxP);
void imgDimsIni(ImgDims *imgDimsP, U32 width, U32 height, U32 bpp);
S32 getColormapIdx(U8 *cpColorA, const U8 *colorQueryP, U8 verbose);
Error readPng(char *imgPathA, Colormap *cmP, ColorPalette *cpP, U8 verbose);
Error getColorPaletteAndColormap(ColorPalette *cpP, Colormap *cmP, png_image *pngImgP, U8 *pixelP, U8 srcPixelSize, U8 verbose);
Error makeColorPaletteAndColormap(ColorPalette *cpP, Colormap *cmP, png_image *srcImgDataP, U8 *srcPixelA, U8 verbose);
Error img(char *imgFilePathP, Database *cpDirP, Database *cmDirP, U8 verbose);
void cpClr(ColorPalette *cpP);

#endif
