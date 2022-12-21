#include "img.h"
#include "genie.h"

char TROVE_IMG_OBJ_DIR[] = "/jb/build/Colormap/obj/";
char TROVE_IMG_SRC_DIR[] = "/jb/build/Colormap/src/";
char TROVE_IMG_INC_DIR[] = "/jb/build/Colormap/include/";

void imgDimsIni(ImgDims *imgDimsP, U32 width, U32 height, U32 bpp) {
  imgDimsP->w = width;
  imgDimsP->h = height;
  imgDimsP->bpp = bpp;
  imgDimsP->pitch = width * bpp / 8;  // "bpp / 8" converts from bpp to bytes
}

Error writeJbColorPalette(char *imgNameA, U8 *paletteA, U8 verbose) {
  Error e = SUCCESS;
  const char OBJ_TYPE[] = "ColorPalette_";
  FILE *fP = NULL;
  // Make target filepath to save this stripmap in.
  char *homefp = getenv("HOME");
  if (!homefp) {
    if (verbose)
      printf("no $HOME environment variable! exiting...\n");
    return E_BAD_ARGS;
  }
  char fullPath[strlen(OBJ_TYPE) + strlen(imgNameA) + strlen(homefp) + strlen(TROVE_IMG_SRC_DIR) + strlen(".c")];
  strcpy(fullPath, homefp);
  strcat(fullPath, TROVE_IMG_SRC_DIR);
  strcat(fullPath, OBJ_TYPE);
  strcat(fullPath, imgNameA);
  strcat(fullPath, ".c");
  if (verbose)
    printf("writing color palette for use in jollybean...\n");
  // Open file.
  fP = fopen(fullPath, "w");
  if (!fP) {
    if (verbose)
      printf("[writeJbColorPalette] file opening failed for path %s\n", fullPath);
    e = E_NO_MEMORY;
  }
  if (!e) {
    // Write header.
    fprintf(fP, "#include \"xRender.h\"\n\n");
    // Palette Array
    fprintf(fP, "U32 %sPaletteA[] = {\n", imgNameA);
    writeRawData32(fP, (U32*) paletteA, arrayGetNElems(paletteA));
    fprintf(fP, "};\n\n");
    fprintf(fP, "ColorPalette %sColorPalette = {\n", imgNameA);
    fprintf(fP, "\t.nColors = %d,\n", arrayGetNElems(paletteA));
    fprintf(fP, "\t.paletteA = %sPaletteA\n", imgNameA);
    fprintf(fP, "};\n\n");
  }
  if (fP) {
    // Close file.
    fclose(fP);
  }
  return SUCCESS;
}

Error writeAsepriteColorPalette(char *imgNameA, EntryData *paletteData, U8 verbose) {
  Error e = SUCCESS;
  const char OBJ_TYPE[] = "ColorPalette_";
  FILE *fP = NULL;
  // Make target filepath to save this stripmap in.
  char *homefp = getenv("HOME");
  if (!homefp) {
    if (verbose)
      printf("no $HOME environment variable! exiting...\n");
    return E_BAD_ARGS;
  }
  char fullPath[strlen(OBJ_TYPE) + strlen(imgNameA) + strlen(homefp) + strlen(TROVE_IMG_SRC_DIR) + strlen(".c")];
  strcpy(fullPath, homefp);
  strcat(fullPath, TROVE_IMG_SRC_DIR);
  strcat(fullPath, OBJ_TYPE);
  strcat(fullPath, imgNameA);
  strcat(fullPath, ".c");
  if (verbose)
    printf("writing color palette for reuse in aseprite...\n");
  // Open file.
  fP = fopen(fullPath, "w");
  if (!fP) {
    if (verbose)
      printf("[writeAsepriteColorPalette] file opening failed for path %s\n", fullPath);
    e = E_NO_MEMORY;
  }

  /*
   * Example file of aseprite palette:
   -----------------------------------
        GIMP Palette
        #
        255 188  15	Untitled
        139 172  15	Untitled
         48  98  48	Untitled
         15  56  15	Untitled
         15  56  15	Untitled
        215  56  15	Untitled
         15 254  15	Untitled
         15  56  15	Untitled
         15  56  15	Untitled
         15  56  15	Untitled
         15  56  15	Untitled
          0   0 100	Untitled
         15  56  15	Untitled
         15  56  15	Untitled
 */

  // Write file.
  if (!e) {
    // Write header.
    fprintf(fP, "GIMP Palette\n#\n");
    // Palette Array
    U32 *paletteA = paletteData->palette.paletteA;
    int nColors = paletteData->palette.nColors;
    for (int i = 0; i < nColors; ++i) {
      fprintf(fP, "%-3d %-3d %-3d %s\n", 
          paletteA[i] & 0xFF000000 >> 24,
          paletteA[i] & 0x00FF0000 >> 16,
          paletteA[i] & 0x0000FF00 >>  8,
          "Untitled");
    }
  }
  if (fP) {
    // Close file.
    fclose(fP);
  }
  return SUCCESS;
}


Error writeStripsHeaderFile(char *imgNameA, U8 verbose) {
  Error e = SUCCESS;
  if (verbose)
    printf("writing strip header file...\n");
  const char OBJ_TYPE[] = "Strips_";
  // Make target filepath to save this stripmap in.
  char *homefp = getenv("HOME");
  if (!homefp) {
    if (verbose)
      printf("no $HOME environment variable! exiting...\n");
    return E_BAD_ARGS;
  }
  char fullPath[strlen(OBJ_TYPE) + strlen(imgNameA) + strlen(homefp) + strlen(TROVE_IMG_INC_DIR) + strlen(".c")];
  strcpy(fullPath, homefp);
  strcat(fullPath, TROVE_IMG_INC_DIR);
  strcat(fullPath, OBJ_TYPE);
  strcat(fullPath, imgNameA);
  strcat(fullPath, ".h");
  // Open file.
  FILE *fP = fopen(fullPath, "w");
  if (!fP) {
    if (verbose)
      printf("[writeColormap] file opening failed for path %s\n", fullPath);
    return E_NO_MEMORY;
  }
  // Write header.
  fprintf(fP, "#include \"xRender.h\"\n\n");
  // Flip set
  fprintf(fP, "extern StripSetS %sStripSet;\n", imgNameA);
  fprintf(fP, "extern StripMapS %sStripMap;\n\n", imgNameA);
  // Close file.
  fclose(fP);
  return e;
}

Error writeColorPaletteHeaderFile(char *imgNameA, U8 verbose) {
  const char OBJ_TYPE[] = "ColorPalette_";
  if (verbose)
    printf("writing color palette header file...\n");
  // Make target filepath to save this stripmap in.
  char *homefp = getenv("HOME");
  if (!homefp) {
    if (verbose)
      printf("no $HOME environment variable! exiting...\n");
    return E_BAD_ARGS;
  }
  char fullPath[strlen(OBJ_TYPE) + strlen(imgNameA) + strlen(homefp) + strlen(TROVE_IMG_INC_DIR) + strlen(".c")];
  strcpy(fullPath, homefp);
  strcat(fullPath, TROVE_IMG_INC_DIR);
  strcat(fullPath, OBJ_TYPE);
  strcat(fullPath, imgNameA);
  strcat(fullPath, ".h");
  // Open file.
  FILE *fP = fopen(fullPath, "w");
  if (!fP) {
    if (verbose)
      printf("[writeColormap] file opening failed for path %s\n", fullPath);
    return E_NO_MEMORY;
  }
  Error e = SUCCESS;
  // Write header.
  fprintf(fP, "#include \"xRender.h\"\n\n");
  // Flip set
  fprintf(fP, "extern ColorPalette %sColorPalette;\n", imgNameA);
  // Close file.
  fclose(fP);
  return SUCCESS;
}

Error writeColormap(char *imgNameA, StripSetS *stripsetP, ImgDims *imgDimsP, U8 verbose) {
  const char OBJ_TYPE[] = "Colormap_";
  if (verbose)
    printf("writing colormap...\n");
  // Make target filepath to save this stripmap in.
  char *homefp = getenv("HOME");
  if (!homefp) {
    if (verbose)
      printf("no $HOME environment variable! exiting...\n");
    return E_BAD_ARGS;
  }
  char fullPath[strlen(OBJ_TYPE) + strlen(imgNameA) + strlen(homefp) + strlen(TROVE_IMG_SRC_DIR) + strlen(".c")];
  strcpy(fullPath, homefp);
  strcat(fullPath, TROVE_IMG_SRC_DIR);
  strcat(fullPath, OBJ_TYPE);
  strcat(fullPath, imgNameA);
  strcat(fullPath, ".c");
  // Open file.
  FILE *fP = fopen(fullPath, "w");
  if (!fP) {
    if (verbose)
      printf("[writeColormap] file opening failed for path %s\n", fullPath);
    return E_NO_MEMORY;
  }
  Error e = SUCCESS;
  // Make Title format of image name.
  char imgTitle[strlen(imgNameA) + 1];
  strcpy(imgTitle, imgNameA);
  imgTitle[0] = toupper(imgTitle[0]);
  // Write header.
  fprintf(fP, "#include \"xRender.h\"\n");
  fprintf(fP, "#include \"Strips_%s.h\"\n\n", imgNameA);
  // Flip set
  fprintf(fP, "ColormapS %sColormap = {\n", imgNameA);
  fprintf(fP, "\t.bpp = %d,\n", imgDimsP->bpp);
  fprintf(fP, "\t.w = %d,\n", imgDimsP->w);
  fprintf(fP, "\t.h = %d,\n", imgDimsP->h);
  fprintf(fP, "\t.pitch = %d,\n", imgDimsP->pitch);
  fprintf(fP, "\t.stripSetP = &%sStripSet,\n", imgNameA);
  fprintf(fP, "\t.stripMapP = &%sStripMap,\n", imgNameA);
  fprintf(fP, "};\n\n");
  // Close file.
  fclose(fP);
  return SUCCESS;
}

// =========================================================================================
// When you're building your color palette, this adds colors that don't already exist in it.
S32 getColormapIdx(U8 *srcPixelA, U32 nPixels, const U8 srcPixelSize, const U8 *srcPixelQueryP) {
  U8 *srcPixelP = srcPixelA;
  U8 *srcPixelEndP = srcPixelP + (nPixels * srcPixelSize);
  for (; srcPixelP < srcPixelEndP; srcPixelP += srcPixelSize) {
    if (!memcmp((const void*) srcPixelP, (const void*) srcPixelQueryP, srcPixelSize)) {
      return (srcPixelP - srcPixelA) / srcPixelSize;
    }
  }
  return -1;
}

Error readPng(png_image **imgPP, char *imgPathA, U8 *pixelSizeP, U8 **pixelAP, U8 **colorPalettePP, U8 verbose) {
  if (!imgPP || !imgPathA || !pixelAP || !pixelSizeP)
    return E_BAD_ARGS;

  png_color blackBg = {0}; // used to replace alpha pixels for 1-byte output pixel format
  *imgPP = malloc(sizeof(png_image));
  if (!*imgPP)
    return E_NO_MEMORY;

  memset(*imgPP, 0, sizeof(png_image));  // bombed, seeing if this worked
  (*imgPP)->version = PNG_IMAGE_VERSION;
  // Set up reader.
  int e = png_image_begin_read_from_file(*imgPP, imgPathA);
  // Allocate pixel array.
  if ((*imgPP)->warning_or_error) {
    printf("libpng error message: %s\n", (*imgPP)->message);
    printf("related to file %s.\n", imgPathA);
  }
  else if (!e) {
    printf("png_image_begin_read() errored with e = %d.\n", e);
    if ((*imgPP)->message[0]) 
      printf("%s\n", (*imgPP)->message);
  }
  // Perform actual read.
  if (!(*imgPP)->warning_or_error) {
    U32 rowStride = PNG_IMAGE_ROW_STRIDE(**imgPP);
    U32 bufferSz = PNG_IMAGE_BUFFER_SIZE(**imgPP, rowStride);
    if (verbose) {
      printf("row stride: %d\n", rowStride);
      printf("buffer size: %d\n", bufferSz);
    }

    *pixelSizeP = PNG_IMAGE_PIXEL_SIZE((*imgPP)->format);
    if (verbose)
      printf("source pixel size: %d\n", *pixelSizeP);

    // Output pixel buffer
    e = arrayNew((void**) pixelAP, 1, bufferSz);
    if (e)
      return e;

    // In PNGs that're alreaday colormaps, we go ahead and populate the color palette here.
    if (*pixelSizeP == 1) {
      if ( (*imgPP)->colormap_entries ) {
        if (verbose)
          printf("Allocating %d bytes for libpng to give us its color palette.\n", PNG_IMAGE_COLORMAP_SIZE(**imgPP));
        e = arrayNew((void**) colorPalettePP, sizeof(U8), PNG_IMAGE_COLORMAP_SIZE(**imgPP));
      }
      if (!e) {
        e = png_image_finish_read(*imgPP, NULL, *pixelAP, 0, *colorPalettePP);
      }
    }
    // If PNG is NOT a colormap, then read it like a normal image without getting a color palette.
    else
      // row_stride param being 0 forces libpng to calculate the pitch for you.
      e = png_image_finish_read(*imgPP, NULL, *pixelAP, 0, NULL);

    if ((*imgPP)->warning_or_error) 
      printf("%s\n", (*imgPP)->message);
  }

  if (!(*imgPP)->warning_or_error && verbose)
    printf("no error on read; it says w = %d and h = %d.\n", (*imgPP)->width, (*imgPP)->height);

  //free(colormapA);
  if (!e) { // png's error returns 0 if it fails.
    arrayDel((void**) colorPalettePP);
    arrayDel((void**) pixelAP);
  }
  
  return (e != 0) ? SUCCESS : E_BAD_ARGS;  // libpng errors are weird.
} 

//##########################################
// 1-byte PNG images are understood to be color palettes. Usually they have extra colors we don't need, so we're removing the unused ones here.
Error stealColorPaletteAndColormap(U8 **paletteAP, U8 **colormapPP, U32 *nColorsP, U8 srcPixelSize, png_image *srcImgDataP, U8 *srcImgP, U8 verbose) {
  if (!paletteAP || !colormapPP || !nColorsP || !srcImgDataP || !srcImgP)
    return E_BAD_ARGS;
  if (srcPixelSize != 1) {
    if (verbose)
      printf("[makeColorPaletteAndColormap] srcPixelSize is %d.\n", srcPixelSize);
    return E_BAD_ARGS; 
  }

  // We're dealing with 1-byte pixels, meaning the palette should already be updated.
  assert(srcPixelSize == 1 && *paletteAP != NULL);

  U8 *newColormapP = NULL;
  U32 *newColorPalette = NULL;

  // Allocate new colormap to replace the old one.
  Error e = arrayNew((void**) &newColormapP, sizeof(ColormapIdx), srcImgDataP->width * srcImgDataP->height);
  // Allocate new palette to replace the old one.
  if (!e)
    e = arrayNew((void**) paletteAP, srcPixelSize, N_COLORS_SUPPORTED_MAX_); 
  // Loop through 
  if (!e) {
  }
  return e;
}

//##########################################
Error makeColorPaletteAndColormap(U8 **paletteAP, U8 **colormapPP, U32 *nColorsP, U8 srcPixelSize, png_image *srcImgDataP, U8 *srcImgP, U8 verbose) {
  if (!paletteAP || !colormapPP || !nColorsP || !srcImgDataP || !srcImgP)
    return E_BAD_ARGS;

  if (srcPixelSize != 2 && srcPixelSize != 4) {
    if (verbose)
      printf("[makeColorPaletteAndColormap] srcPixelSize is %d.\n", srcPixelSize);
    return E_BAD_ARGS; 
  }

  // If we're dealing with 1-byte pixels, then the palette should already be updated.
  assert((srcPixelSize == 2 || srcPixelSize == 4) && *paletteAP == NULL);

  // Allocate colormap.
  Error e = arrayNew((void**) colormapPP, sizeof(ColormapIdx), srcImgDataP->width * srcImgDataP->height);
  // Allocate color palette.
  if (!e)
    e = arrayNew((void**) paletteAP, srcPixelSize, N_COLORS_SUPPORTED_MAX_); 
  if (!e) {
    // Set up pointers for looping through input image.
    U8 *srcPixelP = srcImgP;
    U8 *pixelEndP = srcPixelP + (srcImgDataP->width * srcImgDataP->height * srcPixelSize);
    // Set up pointers for populating colormap.
    U8 *colormapP = *colormapPP;
    U8 *colormapEndP = colormapP + arrayGetNElems((void*) *colormapPP);
    S32 colormapIdx = -1;
    // Make a palette of distinct colors.
    for (*nColorsP = 0; srcPixelP < pixelEndP && colormapP < colormapEndP; srcPixelP += srcPixelSize, ++colormapP) {
      colormapIdx = getColormapIdx(*paletteAP, *nColorsP, srcPixelSize, srcPixelP);
      if (colormapIdx < 0) {
        memcpy(&(*paletteAP)[*nColorsP * srcPixelSize], srcPixelP, srcPixelSize);
        *colormapP = (*nColorsP)++;
      }
      else 
        *colormapP = (U8) colormapIdx;
    }
  }
  // If aseprite's output PNG uses the 16-bit [ gray | alpha ] pixel format, rectify that here.
  if (!e && srcPixelSize == 2) {
    U32 *tmpA;
    e = arrayNew((void**) &tmpA, sizeof(U32), *nColorsP);
    if (!e) {
      for (int i = 0; i < *nColorsP; ++i) 
        tmpA[i] = (*paletteAP)[i*2] | (*paletteAP)[i*2] | (*paletteAP)[i*2] | 0xff;  // alpha is 0xff
      arrayDel((void**) paletteAP);
      *paletteAP = (U8*) tmpA;  
    }
  }
  return e;
}

// TODO migrate this to genie code
// TODO Use this when you start putting things into enums with key.s
void toUpper(char *stringP) {
  for (char *charP = stringP; *charP; ++charP) 
    if (*charP >= 'a' && *charP <= 'z')
      *charP -= 32;
}

Error getColorPaletteAndColormap(U8 **colorPaletteAP, U8 **colormapAP, U32 *nColorsP, png_image *pngImgP, U8 *pixelP, U8 maxNColorsAllowed, U8 srcPixelSize, U8 verbose) {
  if (!colorPaletteAP || !nColorsP || !pngImgP || !maxNColorsAllowed)
    return E_BAD_ARGS;
  Error e = SUCCESS;
  if (*colorPaletteAP) { // libpng gave us a color palette; use it.
    *nColorsP = PNG_IMAGE_COLORMAP_SIZE(*pngImgP);
    if (*nColorsP > maxNColorsAllowed) 
      return E_BAD_ARGS;
    e = stealColorPaletteAndColormap(colorPaletteAP, colormapAP, nColorsP, srcPixelSize, pngImgP, pixelP, verbose);
  }
  else
    e = makeColorPaletteAndColormap(colorPaletteAP, colormapAP, nColorsP, srcPixelSize, pngImgP, pixelP, verbose);
  return e;
}

//##########################################
Error img(char *imgFilePathP, Database *cpDirP, Database *cmDirP, U8 verbose) {
  U8 *pixelP = NULL;  // can be 1bpp colormap, 2bpp gray+alpha, or 4bpp RGBA
  U8 *colorPaletteA = NULL;
  U8 *colormapA = NULL;
  png_image *pngImgP = NULL;
  U32 nColors = 0;
  U8 srcPixelSize = 0;
  StripSetS stripset = {0};
  StripMapS stripmap = {0};
  ImgDims imgDims = {0};
  U8 bpp = 0;
  
  Error e = readPng(&pngImgP, imgFilePathP, &srcPixelSize, &pixelP, &colorPaletteA, verbose);

  if (!e) 
    e = getColorPaletteAndColormap(&colorPaletteA, &colormapA, &nColors, pngImgP, pixelP, 16, srcPixelSize, verbose);

  if (verbose) {
    printf("%d distinct colors w/ %d byte source pixel size.\n", nColors, srcPixelSize);
  }

  // Compute pixel bitdepth.
  if (!e) {
    if (nColors <= 2)
      imgDimsIni(&imgDims, pngImgP->width, pngImgP->height, 1);
    else if (nColors <= 4)
      imgDimsIni(&imgDims, pngImgP->width, pngImgP->height, 2);
    else if (nColors <= 16)
      imgDimsIni(&imgDims, pngImgP->width, pngImgP->height, 4);
    else {
badPixel:
      if (verbose)
        printf("Jollybean doesn't support color palettes of more than %d colors.\n%s uses %d colors.\n", N_COLORS_SUPPORTED_MAX_, imgFilePathP, nColors/4);
      e = E_UNSUPPORTED_PIXEL_FORMAT;
    }
    if (!e)
      bpp = imgDims.bpp;
  }

  // Colormap StripSet  & StripMap
  if (!e)
    e = stripNew(colormapA, verbose, bpp, arrayGetNElems(colormapA), &stripset, &stripmap);

  // Requires inflatable from data.h and util's inflatable writer function.
  char entityName[250] = {0};
  if (!e) {
    U32 entityNameIdx, entityNameLen;
    parseName(imgFilePathP, ".png", &entityNameIdx, &entityNameLen);
    memcpy((void*) entityName, (void*) &imgFilePathP[entityNameIdx], entityNameLen);
    if (verbose) {
      printf("Entity name is %s.\n", entityName);
    }
  }
  if (!e) {
    if (!entityName[0]) {
      if (verbose)
        printf("entity name is empty!\n");
      return E_BAD_ARGS;
    }
  }

  // Give game engine the stripMap for this image if it applies.
  if (!e)
    e = writeStripData(entityName, "Colormap", verbose, &stripset, &stripmap);
  if (!e)
    e = writeColormap(entityName, &stripset, &imgDims, verbose);
  if (!e)
    e = writeStripsHeaderFile(entityName, verbose);

  // Time to see whether to update the new palette directory.
  if (!e) {
    EntryData entryDataToFind;
    entryDataToFind.palette.nColors = nColors;
    memcpy(entryDataToFind.palette.paletteA, colorPaletteA, sizeof(U32) * nColors);
    char *existingPaletteName = dbFindNameByValue(cpDirP, &entryDataToFind, verbose);
    if (!existingPaletteName) {
      genieAsk(
          "This palette is new. What would you like to name it?", 
          DIR_FILE, (List*) cpDirP, verbose);
      // Store our palette only because it doesn't already exist in the directory.
      e = writeColorPaletteHeaderFile(entityName, verbose);
      if (!e)
        e = writeJbColorPalette(entityName, colorPaletteA, verbose);
      if (!e)
        dbAddEntry(cpDirP, entityName, &entryDataToFind, verbose);  // TODO replace entity name with name which genie prompots you for
    }
    else 
      if (verbose)
        printf("%s found a matching palette. Skipping write stage.\n", entityName);
  }

  stripDel(&stripset, &stripmap);
  arrayDel((void**) &colorPaletteA);
  arrayDel((void**) &pixelP);
  free(pngImgP);
  return e;
}
