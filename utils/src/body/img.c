#include "img.h"
#include "previewImg.h"

#define DEBUG_IMG_ 0
#define SHOW_PREVIEW 0
// TODO: copied from xRender. Fix linker error in cmGen there so you can reuse it without copy/paste.

static void _printColors(ColorPalette *cpP) {
  return;
  printf("color palette elements:\n");
  if (cpP && cpP->colorA) {
    for (int i = 0; i < cpP->nColors; ++i) {
      printf("\t%d: {%d, %d, %d, %d}\n", i, cpP->colorA[i].r, cpP->colorA[i].g, cpP->colorA[i].b, cpP->colorA[i].a);
    }
  }
}

static void _printColormap(Colormap *cmP) {
  printf("colormap elements:\n");
  if (cmP && cmP->sdP) {
    int j = 0;
    for (int i = 0; i < arrayGetNElems(cmP->sdP->assembledDataA); ++i) {
      printf("%d", cmP->sdP->assembledDataA[i]);
      if (++j >= cmP->sdP->ss.nUnitsPerStrip) {
        j = 0;
        printf("\n");
      }
    }
  }
}

static void setColormapPath( char* dstPath, Colormap *cmP) {
  // Make target filepath to save this stripmap in.
  const static char BASE_PATH[] = "Image/Colormap/%s";
  sprintf( dstPath, BASE_PATH, 
      cmP->bpp == 1 ? "1bpp" :
      cmP->bpp == 2 ? "2bpp" : "4bpp" );
}

static void setColorPalettePath( char* dstPath, ColorPalette *cpP ) {
  // Make target filepath to save this stripmap in.
  const static char BASE_PATH[] = "Image/ColorPalette/%s";
  sprintf( dstPath, BASE_PATH, 
      cpP->nColors <=  2 ? "1bpp" :
      cpP->nColors <=  4 ? "2bpp" : "4bpp" );
}


static void writeColorPalette(char *imgNameA, ColorPalette *cpP, U8 verbose) {
  char directoryPath[500];
  setColorPalettePath( directoryPath, cpP ); 
  FILE *fP = getSrcFile( directoryPath, imgNameA, "ColorPalette.c", verbose );
  assert(fP);
  // Write header.
  fprintf(fP, "#include \"x/xRender.h\"\n\n");
  // Palette Array
  fprintf(fP, "Color_ %sPaletteA[] = {\n", imgNameA);
  writeRawData8(fP, (U8*) cpP->colorA, cpP->nColors * sizeof(Color_));
  fprintf(fP, "};\n\n");
  fprintf(fP, "ColorPalette %sColorPalette = {\n", imgNameA);
  fprintf(fP, "\t.nColors = %d,\n", cpP->nColors);
  fprintf(fP, "\t.colorA = %sPaletteA\n", imgNameA);
  fprintf(fP, "};\n\n");
  // Close file.
  fclose(fP);
}

static void writeColormap(char *imgNameA, Colormap *cmP, U8 verbose) {
  char directoryPath[500];
  setColormapPath( directoryPath, cmP ); 
  FILE *fP = getSrcFile(directoryPath, imgNameA, "Colormap.c", verbose);
  assert(fP);
  // Make Title format of image name.
  fprintf(fP, "#include \"x/xRender.h\"\n\n");
  // Write stripmap and stripset data.
  writeStripDataInFile(fP, verbose, imgNameA, cmP->sdP);
  // Write colormap
  fprintf(fP, "Colormap %sColormap = {\n", imgNameA);
  fprintf(fP, "\t.bpp = %d,\n", cmP->bpp);
  fprintf(fP, "\t.w = %d,\n", cmP->w);
  fprintf(fP, "\t.h = %d,\n", cmP->h);
  fprintf(fP, "\t.pitch = %d,\n", cmP->pitch);
  fprintf(fP, "\t.sdP = &%sStripData,\n", imgNameA);
  fprintf(fP, "};\n\n");
  fclose(fP);
}

/*
 * 
 * typedef struct {
 *   U8 state;  // prevents copies of images from being added to texture atlas
 *   Key sortedRectIdx;  // Index of sorted rectangle so you can adjust src rect's XY offset in atlas
 *   Colormap* cmP;
 *   ColorPalette* cpP;
 * } Image;
 *
 */

// TODO implement this
void writeImage(char *imgNameA, U8 verbose) {
  if (verbose) {
    printf("writing image...\n");
  }
  FILE *fP = getSrcFile("Image/", imgNameA, "Img.c", verbose);
  assert(fP);
  // Make Title format of image name.
  fprintf(fP, "#include \"x/xRender.h\"\n");
  fprintf(fP, "extern Colormap %sColormap;\n", imgNameA);
  fprintf(fP, "extern ColorPalette %sColorPalette;\n", imgNameA);
  // Write stripmap and stripset data.
  // Write colormap
  fprintf(fP, "Image %sImg = {\n", imgNameA);
  fprintf(fP, "\t.state = 0,\n");
  fprintf(fP, "\t.sortedRectIdx = 0,\n");
  fprintf(fP, "\t.cmP = &%sColormap,\n", imgNameA);
  fprintf(fP, "\t.cpP = &%sColorPalette,\n", imgNameA);
  fprintf(fP, "};\n\n");
  fclose(fP);
}

// =========================================================================================
// When you're building your color palette, this adds colors that don't already exist in it.
// This acts agnostic to the size of color palette elements.
S32 getColormapIdx(U8 *cpColorA, const U8 *colorQueryP, U8 verbose) {
  U8 *colorP = cpColorA;
  const U32 cpElemSz = arrayGetElemSz(cpColorA);
  U8 *cpEndP = colorP + (arrayGetNElems(cpColorA) * cpElemSz);
  // TODO put verbose printing matching and mismatching here to see why it's returning -1.
  for (; colorP < cpEndP; colorP += cpElemSz) {
    if (!memcmp((const void*) colorP, (const void*) colorQueryP, cpElemSz)) {
      return (colorP - cpColorA) / cpElemSz;
    }
  }
  // Tell the caller we've never encountered this color before because it's not in our palette.
  return -1;  
}

#if 0 // unused function
static void mallocSanityCheck(char *msgDetails) {
  U8 *memA = memAdd(sizeof(U8) * 100000, GENERAL );
  printf("malloc works fine: %s\n", msgDetails);
}
#endif

//##########################################
// Coming in, colors in palette may be 2 or 4 bytes depending on whether it's gray-alpha or not.
// That's why this function acts agnostic to color palette elem sizes which are, in fact,
// defined as Color_ (4 bytes for RGBA in SDL's case).
static void _makeColorPaletteAndColormap(U8 **colormapAP, ColorPalette *cpP, Colormap *cmP, png_image *srcImgDataP, U8 *srcPixelA, U8 verbose) {
  assert (cpP && cmP && srcImgDataP && srcPixelA);

  U8 srcPixelSz = arrayGetElemSz(srcPixelA);
  // If we're dealing with 1-byte pixels, then the palette should already be updated.
  assert((srcPixelSz == 2 || srcPixelSz == 4) && cpP->colorA == NULL);
  // Allocate colormap (in a placeholder colormap array, not the proper location due to testing).
  *colormapAP = arrayNew(sizeof(ColormapIdx) , srcImgDataP->width * srcImgDataP->height, GENERAL  );
  // Allocate color palette.
  // TODO this is the problem.
  cpP->colorA = arrayNew(srcPixelSz, N_COLORS_SUPPORTED_MAX_, GENERAL ); 
  memset(cpP->colorA, 0, arrayGetNElems(cpP->colorA) * arrayGetElemSz(cpP->colorA));
  // Set up pointers for looping through input image.
  U8 *srcPixelP = srcPixelA;
  U8 *pixelEndP = srcPixelP + (srcImgDataP->width * srcImgDataP->height * srcPixelSz);
  // Set up pointers for populating colormap.
  U8 *cmElemP = *colormapAP;
  U8 *colormapEndP = cmElemP + arrayGetNElems((void*) *colormapAP);
  S32 colormapIdx = -1;
  // Make a palette of distinct colors.
  for (cpP->nColors = 0; srcPixelP < pixelEndP && cmElemP < colormapEndP; srcPixelP += srcPixelSz, ++cmElemP) {
    colormapIdx = getColormapIdx((U8*) cpP->colorA, srcPixelP, verbose);
#if 0
    if (srcPixelSz == 2) {
      printf("Idx %d: {%d, %d}\n", colormapIdx, srcPixelP[0], srcPixelP[1]);
    }
    else if (srcPixelSz == 4) {
      printf("Idx %d: {%d, %d, %d, %d}\n", colormapIdx, srcPixelP[0], srcPixelP[1], srcPixelP[2], srcPixelP[3]);
    }
#endif
    // < 0 means this is an unencountered color; add it to the palette.
    if (colormapIdx < 0) {
      memcpy((void*) &cpP->colorA[cpP->nColors], (const void*) srcPixelP, srcPixelSz);
      *cmElemP = (U8) (cpP->nColors++);  // set colormap element to current palette count
    }
    else {
      *cmElemP = (U8) colormapIdx;
    }
  }
  // If aseprite's output PNG uses the 16-bit [ gray | alpha ] pixel format, rectify that here.
  if (srcPixelSz == 2) {
    if (verbose) {
      printf("pixel size: 2\n");
    }

    U8 *grayPaletteA = (U8*) cpP->colorA;
    U32 *tmpA = arrayNew(sizeof(U32), cpP->nColors, GENERAL );
    register U8 grayElem = 0;
    // += 2 skips over alpha channels of gray-alpha image.
    for (int i = 0; i < cpP->nColors; i += 2) {
      grayElem = grayPaletteA[i];
      tmpA[i] = grayElem << 24 |
        grayElem << 16 |
        grayElem <<  8 | 
        0xff;  // alpha is 0xff
    }
    cpP->colorA = (Color_*) tmpA;  
  }
}


void readPng(char *imgPathA, Colormap *cmP, ColorPalette *cpP, AnimJsonData *animP, U8 verbose) {
  assert (imgPathA && cmP && cpP);

#if DEBUG_IMG_
  FILE *fP = fopen(imgPathA, "rb");
  if (fP && verbose) {
    fseek(fP, 0, SEEK_END);
    printf("PNG: original size is %ld bytes.\n", ftell(fP));
    fclose(fP);
  }
#endif

  
  // png_color blackBg = {0}; // used to replace alpha pixels for 1-byte output pixel format
  U8 srcPixelSize = 0;
  // Allocate PNG image info
  png_image *pngP = memAdd(sizeof(png_image), GENERAL);
  memset(pngP, 0, sizeof(png_image));  // bombed, seeing if this worked
  pngP->version = PNG_IMAGE_VERSION;
  // Set up PNG reader.
  int e = png_image_begin_read_from_file(pngP, imgPathA);
  assert(e);  // means file IO error if it's 0
  // No errors. Perform actual read.
  U8 *colormapA = NULL;
  srcPixelSize = PNG_IMAGE_PIXEL_SIZE(pngP->format);
  cmP->pitch = PNG_IMAGE_ROW_STRIDE(*pngP);
  U32 bufferSz = PNG_IMAGE_BUFFER_SIZE(*pngP, cmP->pitch);  // number of bytes whole src image
  cmP->w = cmP->pitch / srcPixelSize;
  cmP->h = bufferSz / cmP->pitch;

  if (verbose) {
    printf("src img pitch: %d\n", cmP->pitch);
    printf("src img size in bytes: %d\n", bufferSz);
    printf("src img pixel size: %d\n", srcPixelSize);
  }

  // In PNGs that're already colormaps, we go ahead and populate the color palette here.
  /**************************/
  // PNG IS A COLORMAP if its pixels are one byte each.
  /**************************/
  if (srcPixelSize == 1) {
    // Output 1-byte pixel buffer
    colormapA = arrayNew(sizeof(U8), bufferSz, GENERAL );
    if ( pngP->colormap_entries ) {
      if (verbose) {
        printf("Allocating %d bytes for libpng to give us its color palette.\n", PNG_IMAGE_COLORMAP_SIZE(*pngP));
      }
      cpP->colorA = arrayNew(sizeof(Color_), PNG_IMAGE_COLORMAP_SIZE(*pngP) / sizeof(Color_), GENERAL );
    }
    cpP->nColors = PNG_IMAGE_COLORMAP_SIZE(*pngP) / sizeof(Color_);
    assert ((cpP->nColors / sizeof(Color_)) <= N_COLORS_SUPPORTED_MAX_);
    e = png_image_finish_read(pngP, NULL, colormapA, 0, cpP->colorA);
    // png's error returns 0 if it fails, hence e.
    assert(e);
    // mallocSanityCheck("after finishing reeading PNG 1");
    if (verbose) {
      printf("\nColor palette in colormap-type PNG:\n");
      for (Color_ *colorP = cpP->colorA; colorP < cpP->colorA + cpP->nColors; ++colorP) {
        printf("\t{%d, %d, %d, %d}\n", colorP->r, colorP->g, colorP->b, colorP->a);
      }
      printf("\n");
    }
  }
  /**************************/
  // IF PNG IS NOT A COLORMAP, then read it like a normal image without getting a color palette.
  /**************************/
  else {
    U8 *srcPixelA = arrayNew(srcPixelSize, bufferSz / srcPixelSize, GENERAL);
    // row_stride param being 0 forces libpng to calculate the pitch for you.
    png_image_finish_read(pngP, NULL, srcPixelA, 0, NULL);
    // mallocSanityCheck("after finishing reeading PNG 2");
    if (verbose) {
      Color_ *pixelP = (Color_*) srcPixelA;
      Color_ *pixelEndP = (Color_*) srcPixelA + arrayGetNElems(srcPixelA);
      assert ((pixelEndP - pixelP) == (bufferSz / srcPixelSize));
      assert ((pixelEndP - pixelP) == (cmP->w * cmP->h));
#if src 
      for (; pixelP < pixelEndP; ++pixelP) {
        printf("{%d, %d, %d, %d}, ", pixelP->r, pixelP->g, pixelP->b, pixelP->a);
      }
#endif
    }
    // png's error returns 0 if it fails, hence !e.
    // mallocSanityCheck("before making CP and CM");
    _makeColorPaletteAndColormap(&colormapA, cpP, cmP, pngP, srcPixelA, verbose);
    // mallocSanityCheck("after making CP and CM");
    if (verbose) {
      printf("\nColor palette in RGBA-type PNG:\n");
      for (Color_ *colorP = cpP->colorA; colorP < cpP->colorA + cpP->nColors; ++colorP) {
        printf("\t{%d, %d, %d, %d}\n", colorP->r, colorP->g, colorP->b, colorP->a);
      }
      printf("\n");
    }
  }

  if (pngP->warning_or_error) {
    printf("%s\n", pngP->message);
  }


  if (verbose) {
    _printColors(cpP);
    _printColormap(cmP);
  }

  // Set colormap's bpp.
  if (cpP->nColors <= 2) {
    cmP->bpp = 1;
  }
  else if (cpP->nColors <= 4) {
    cmP->bpp = 2;
  }
  else if (cpP->nColors < 16) {
    cmP->bpp = 4;
  }
  else {
    assert(0);
  }

  // Colormap's stripset & stripmap
  // If image is animated, strip length should be the width of an animation frame.
  // Otherwise, it should be the image width.
  U32 stripLen = cmP->w;
  if (animP && animP->frameNodeA) {
    stripLen = animP->frameNodeA[0].w;
  }
  cmP->sdP = stripNew(colormapA, stripLen, cmP->bpp, SD_SKIP_ASSEMBLY_, verbose );
  // We're gonna cheat and preview the ground truth image here. 
  // We'll test stripIni properly in _validateWholeInput().
  cmP->sdP->assembledDataA = colormapA;
  // Sanity check before making strips
  if (verbose && SHOW_PREVIEW) {
    previewImg(cmP, cpP, 1000);
  }

  if (!e && verbose) {
    _printColors(cpP);
    _printColormap(cmP);
  }

  if (!pngP->warning_or_error && verbose) {
    printf("PNG image load success: %d x %d \n", pngP->width, pngP->height);
  }

}  // readPng()

static void _validateWholeInput(Colormap *cmP, ColorPalette *cpP) {
  assert (cmP && cpP);
  assert (cmP->sdP);

  U8* outputA = ssGetOutput( cmP->sdP );  // Output pointer is asserted to be non-null internally.

  // Deflate colormap/palette and get rid of their resulting outputs to truly test strip-piecing.
  U8 *cmOrigP = NULL;
  cmOrigP = arrayNew(arrayGetElemSz( outputA ), arrayGetNElems( outputA ), GENERAL);
  memcpy(cmOrigP, outputA, arrayGetElemSz( outputA ) * arrayGetNElems( outputA ));
  // Clear out old colormap
  printf("original input\n");
  printStripData(cmP->sdP);
#if SHOW_PREVIEW
  previewImg(cmP, cpP, 1000);
#endif
  stripIni(cmP->sdP, TEMPORARY );
  outputA = ssGetOutput( cmP->sdP );  // Old pointer's dangling after deletion, so you have to reset it.
  printf("reconstructed input\n");
  printStripData(cmP->sdP);
#if 1
  // Verify array lengths match.
  printf("orig cm has %d elems; new cm has %d\n", 
      arrayGetNElems(cmOrigP), arrayGetNElems( outputA ));
  printf("orig cm elem size is %d; new cm elem size is %d\n", 
      arrayGetElemSz(cmOrigP), arrayGetElemSz( outputA ));
  assert ((arrayGetNElems( outputA ) == arrayGetNElems(cmOrigP)));
  // Verify array elem sizes match.
  assert (arrayGetElemSz( outputA ) == arrayGetElemSz(cmOrigP));
#endif
  // Compare data directly
  U8 theyMatch = 0;
#if 0
  printf("\n\n\nOriginal colormap array:\n");
  for (U8 *p = cmOrigP; p < cmOrigP + arrayGetNElems(cmOrigP); ++p) {
    printf("%d", *p);
  }
  printf("\n\nNew colormap array:\n");
  for (U8 *p = outputA; p < outputA + arrayGetNElems(outputA); ++p) {
    printf("%d", *p);
  }
  printf("\n\n\n");
#endif
  if ((theyMatch = !memcmp(outputA, 
          cmOrigP, 
          arrayGetElemSz(outputA) 
          * arrayGetNElems(outputA)))) {
    printf("\n\n\e[92mthey match! :)\e[0m\n");
  }
  else {
    printf("\n\n\e[91mthey don't match :(\e[0m \n");
  }
  if (theyMatch) {
#if SHOW_PREVIEW
    previewImg(cmP, cpP, 1000);
#endif
  }
}

//##########################################
void img(char *entityNameP, AnimJsonData *animP, U8 verbose) {
  // U8 *pixelP = NULL;  // can be 1bpp colormap, 2bpp gray+alpha, or 4bpp RGBA
  // U8 srcPixelSize = 0;
  ColorPalette cp = {0};
  Colormap cm = {0};

  // Only supports PNG for now
  char* imgFilePathP = getResourceFilePath("Image/", entityNameP, ".png", verbose); 

  // Rule them out one by one till you find the culprit!
  // Read PNG into colormap and color palette in one fell swoop.
  readPng(imgFilePathP, &cm, &cp, animP, verbose);

  if (verbose) {
    _printColors(&cp);
    _printColormap(&cm);
  }
  // Write output source files 

  // We need to keep track of the following:
  //    Which colormap we used, by name.
  //    Which color palette we used, by name.
  //    Which image we used, by name.
  //
  //
  writeColormap(entityNameP, &cm, verbose);  // this doesn't seem right. Lots of people can share a color palette.
  writeColorPalette(entityNameP, &cp, verbose);  // this doesn't seem right. Lots of people can share a color palette.
  writeImage(entityNameP, verbose);

  // Verify input independently of how the output treats it
  if (verbose) {
    _validateWholeInput(&cm, &cp);
  }
}
