#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <png.h>
#include "botox.h"
#include "data.h"
#include "inflatable.h"

#define TEST_ 1

#ifdef WINDOWS_
#define FILE_SEP '\\'
#else
#define FILE_SEP '/'
#endif

#define STRIP_N_PIXELS (64) //images will use 8x8 s to ensure pixel count is multiple of STRIP_N_PIXELS

#define KEY_DELIMITER '#'  // had to make it #define instead of const char to make switch compile
const char TROVE_IMAGE_DIR[] = "~/.jb/src/Image/";
static int verbose = 0;

typedef struct {
  U32 w, h, pitch, bpp;
} ImgDims;

typedef png_color Pixel;
typedef U8 ColormapIdx;
typedef U16 StripmapIdx;

typedef struct {
  U8 *flipIdxA;
  ColormapIdx *dataA;
  U16 *stripMapA;
  U8 bpp;
} Stripset;

void getBaseNameIndices(char *filepathP, char *extension, U32 *startIdxP, U32 *endIdxP);
void imgDimsIni(ImgDims *imgDimsP, U32 width, U32 height, U32 bpp);
void writeRawData8(FILE *fP, U8 *byteA, U32 nBytes);
void writeRawData16(FILE *fP, U16 *byteA, U16 nBytes);
void writeRawData32(FILE *fP, U32 *byteA, U32 nBytes);
Error writeInflatable(char *imgNameA, U8 *cmpDataA, U32 cmpLen, U32 decompLen, ImgDims *imgDimsP);
Error writeStripmap(char *imgNameA, Stripset *stripsetP, ImgDims *imgDimsP);
S32 getColormapIdx(Pixel *pixelA, U32 nPixels, const Pixel *pixelQueryP);
Error readPng(png_image **imgPP, char *imgPathA, void **pixelAP, png_color *colorPaletteP);
Error makeStripsetFromColormap(ColormapIdx *colormapA, Stripset *stripsetP, png_image *imgDataP);
Error makeColorPaletteAndColormap(Pixel *paletteA, U8 **colormapPP, U32 *nColorsP, png_image *imgDataP, Pixel *imgP);
Error packBits(Stripset *stripsetP);
Error compressImg(char *imgFilePathP);
void parseEntityAndKeyNames(char *baseNameP, char *extension, U32 *entityNameStartIdxP, U32 *entityNameEndIdxP, U32 *keyNameStartIdxP, U32 *keyNameEndIdxP);
