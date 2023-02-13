#include "img.h"
#include "genie.h"
#include "previewImg.h"

#define DEBUG_IMG_ 1
// TODO: copied from xRender. Fix linker error in cmGen there so you can reuse it without copy/paste.
void cmClr(Colormap *cmP) {
	if (cmP != NULL) {
    stripClr(cmP->sdP);
	}
}

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
    for (int i = 0; i < arrayGetNElems(cmP->sdP->unstrippedDataA); ++i) {
      printf("%d", cmP->sdP->unstrippedDataA[i]);
      if (++j >= cmP->sdP->ss.nUnitsPerStrip) {
        j = 0;
        printf("\n");
      }
    }
  }
}


Error writeColorPalette(char *imgNameA, ColorPalette *cpP, U8 verbose) {
  // Make target filepath to save this stripmap in.
  FILE *fP = getBuildFile("Seed/Genome/Gene/Body/Palette/Color/src", imgNameA, "ColorPalette.c", verbose);
  if (fP) {
    // Write header.
    fprintf(fP, "#include \"xRender.h\"\n\n");
    // Palette Array
    fprintf(fP, "Color_ %sPaletteA[] = {\n", imgNameA);
    writeRawData8(fP, (U8*) cpP->colorA, cpP->nColors * sizeof(Color_));
    fprintf(fP, "};\n\n");
    fprintf(fP, "ColorPalette %sColorPalette = {\n", imgNameA);
    fprintf(fP, "\t.nColors = %d,\n", cpP->nColors);
    fprintf(fP, "\t.colorA = %sPaletteA\n", imgNameA);
    fprintf(fP, "};\n\n");
  }
  else {
    return E_FILE_IO;
  }
  if (fP) {
    // Close file.
    fclose(fP);
  }
  return SUCCESS;
}

Error writeAsepriteColorPalette(char *imgNameA, EntryData *paletteData, U8 verbose) {
  FILE *fP = getBuildFile("Seed/Genome/Gene/Body/Palette/Color/src", imgNameA, "ColorPalette.gpl", verbose);
  // Write file.
  if (!fP) {
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
  else {
    return E_FILE_IO;
  }
  if (fP) {
    // Close file.
    fclose(fP);
  }
  return SUCCESS;
}

Error writeColormapHeaderFile(char *imgNameA, U8 verbose) {
  FILE *fP = getBuildFile("Seed/Genome/Gene/Body/Graybody/Colormap/include", imgNameA, "Colormap.h", verbose);
  if (fP) {
    // Write header.
    fprintf(fP, "#include \"xRender.h\"\n\n");
    // Flip set
    fprintf(fP, "extern Colormap %sColormap;\n\n", imgNameA);
    // Close file.
  }
  else {
    return E_FILE_IO;
  }
  if (fP) {
    fclose(fP);
  }
  return SUCCESS;
}

Error writeColorPaletteHeaderFile(char *imgNameA, U8 verbose) {
  FILE *fP = getBuildFile("Seed/Genome/Gene/Body/Palette/Color/include", imgNameA, "ColorPalette.h", verbose);
  // Write header.
  fprintf(fP, "#include \"xRender.h\"\n\n");
  // Flip set
  fprintf(fP, "extern ColorPalette %sColorPalette;\n", imgNameA);
  // Close file.
  fclose(fP);
  return SUCCESS;
}

Error writeColormap(char *imgNameA, Colormap *cmP, U8 verbose) {
  if (verbose) {
    printf("writing colormap...\n");
  }
  FILE *fP = getBuildFile("Seed/Genome/Gene/Body/Graybody/Colormap/src", imgNameA, "Colormap.c", verbose);
  Error e = SUCCESS;
  if (fP) {
    // Make Title format of image name.
    fprintf(fP, "#include \"xRender.h\"\n\n");
    // Write stripmap and stripset data.
    e = writeStripDataInFile(fP, verbose, imgNameA, cmP->sdP);
  }
  else {
    if (fP) {
      fclose(fP);
    }
    return E_FILE_IO;
  }
  if (!e) { 
    // Write colormap
    fprintf(fP, "Colormap %sColormap = {\n", imgNameA);
    fprintf(fP, "\t.bpp = %d,\n", cmP->bpp);
    fprintf(fP, "\t.w = %d,\n", cmP->w);
    fprintf(fP, "\t.h = %d,\n", cmP->h);
    fprintf(fP, "\t.pitch = %d,\n", cmP->pitch);
    fprintf(fP, "\t.sdP = &%sStripData,\n", imgNameA);
    fprintf(fP, "};\n\n");
  }
  // Close file.
  if (fP) {
    fclose(fP);
  }
  return e;
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
  return -1;
}

static Error mallocSanityCheck(char *msgDetails) {
  U8 *memA = NULL;
  Error e = jbAlloc((void**) &memA, sizeof(U8), 100000);
  if (!e) {
    printf("malloc works fine: %s\n", msgDetails);
  }
  else {
    printf("ERROR: malloc breaks: %s\n", msgDetails);
  }
  jbFree((void**) &memA);
}

//##########################################
// Coming in, colors in palette may be 2 or 4 bytes depending on whether it's gray-alpha or not.
// That's why this function acts agnostic to color palette elem sizes which are, in fact,
// defined as Color_ (4 bytes for RGBA in SDL's case).
static Error _makeColorPaletteAndColormap(U8 **colormapAP, ColorPalette *cpP, Colormap *cmP, png_image *srcImgDataP, U8 *srcPixelA, U8 verbose) {
  if (!cpP || !cmP || !srcImgDataP || !srcPixelA) {
    return E_BAD_ARGS;
  }
  U8 srcPixelSz = arrayGetElemSz(srcPixelA);
  // If we're dealing with 1-byte pixels, then the palette should already be updated.
  assert((srcPixelSz == 2 || srcPixelSz == 4) && cpP->colorA == NULL);
  // Allocate colormap (in a placeholder colormap array, not the proper location due to testing).
  Error e = arrayNew((void**) colormapAP, sizeof(ColormapIdx), srcImgDataP->width * srcImgDataP->height);
  // Allocate color palette.
  if (!e) {
    e = arrayNew((void**) &cpP->colorA, srcPixelSz, N_COLORS_SUPPORTED_MAX_); 
  }
  if (!e) {
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
  }
  // If aseprite's output PNG uses the 16-bit [ gray | alpha ] pixel format, rectify that here.
  if (!e && srcPixelSz == 2) {
    if (verbose) {
      printf("pixel size: 2\n");
    }
    U32 *tmpA;
    U8 *grayPaletteA = (U8*) cpP->colorA;
    e = arrayNew((void**) &tmpA, sizeof(U32), cpP->nColors);
    if (!e) {
      register U8 grayElem = 0;
      // += 2 skips over alpha channels of gray-alpha image.
      for (int i = 0; i < cpP->nColors; i += 2) {
        grayElem = grayPaletteA[i];
        tmpA[i] = grayElem << 24 |
                  grayElem << 16 |
                  grayElem <<  8 | 
                  0xff;  // alpha is 0xff
      }
      arrayDel((void**) &cpP->colorA);
      cpP->colorA = (Color_*) tmpA;  
    }
  }


  return e;
}


Error readPng(char *imgPathA, Colormap *cmP, ColorPalette *cpP, AnimJsonData *animP, U8 verbose) {
  if (!imgPathA || !cmP || !cpP) {
    return E_BAD_ARGS;
  }

#if DEBUG_IMG_
  FILE *fP = fopen(imgPathA, "rb");
  if (fP && verbose) {
    fseek(fP, 0, SEEK_END);
    printf("PNG: original size is %d bytes.\n", ftell(fP));
    fclose(fP);
  }
#endif

  png_image *pngP = NULL;
  png_color blackBg = {0}; // used to replace alpha pixels for 1-byte output pixel format
  U8 srcPixelSize = 0;
  // Allocate PNG image info
  Error e = jbAlloc((void**) &pngP, sizeof(png_image), 1);
  memset(pngP, 0, sizeof(png_image));  // bombed, seeing if this worked
  pngP->version = PNG_IMAGE_VERSION;
  // Set up PNG reader.
  if (!e) {
    int e = png_image_begin_read_from_file(pngP, imgPathA);
    e = !e ? E_FILE_IO : SUCCESS;
    // Handle errors if any
    if (pngP->warning_or_error) {
      printf("libpng error message: %s\n", pngP->message);
      printf("related to file %s.\n", imgPathA);
    }
    else if (e) {  // in this context, !e is an error (libpng way of doing it)
      printf("png_image_begin_read() errored with e = %d.\n", e);
      if (pngP->message[0]) {
        printf("%s\n", pngP->message);
      }
    }
  }
  // No errors. Perform actual read.
  U8 *colormapA = NULL;
  if (!e) {
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
    if (!e) {
      /**************************/
      // PNG IS A COLORMAP if its pixels are one byte each.
      /**************************/
      if (srcPixelSize == 1) {
        // Output 1-byte pixel buffer
        e = arrayNew((void**) &colormapA, sizeof(U8), bufferSz);
        if ( !e && pngP->colormap_entries ) {
          if (verbose) {
            printf("Allocating %d bytes for libpng to give us its color palette.\n", PNG_IMAGE_COLORMAP_SIZE(*pngP));
          }
          e = arrayNew((void**) &cpP->colorA, sizeof(Color_), PNG_IMAGE_COLORMAP_SIZE(*pngP) / sizeof(Color_));
        }
        if (!e) {
          cpP->nColors = PNG_IMAGE_COLORMAP_SIZE(*pngP) / sizeof(Color_);
          if ((cpP->nColors / sizeof(Color_)) > N_COLORS_SUPPORTED_MAX_)  {
            printf("Jollybean doesn't support color palettes of more than %d colors.\n%s uses %d colors.\n", N_COLORS_SUPPORTED_MAX_, imgPathA, cpP->nColors/sizeof(Color_));
            e = E_UNSUPPORTED_PIXEL_FORMAT;
          }
        }
        if (!e) {
          e = png_image_finish_read(pngP, NULL, colormapA, 0, cpP->colorA);
          // png's error returns 0 if it fails, hence !e.
          e = !e ? E_FILE_IO : SUCCESS;
          // mallocSanityCheck("after finishing reeading PNG 1");
          if (!e && verbose) {
            printf("\nColor palette in colormap-type PNG:\n");
            for (Color_ *colorP = cpP->colorA; colorP < cpP->colorA + cpP->nColors; ++colorP) {
              printf("\t{%d, %d, %d, %d}\n", colorP->r, colorP->g, colorP->b, colorP->a);
            }
            printf("\n");
          }
        }
      }
      /**************************/
      // IF PNG IS NOT A COLORMAP, then read it like a normal image without getting a color palette.
      /**************************/
      else {
        U8 *srcPixelA = NULL;
        e = arrayNew((void**) &srcPixelA, srcPixelSize, bufferSz / srcPixelSize);
        if (!e) {
          // row_stride param being 0 forces libpng to calculate the pitch for you.
          e = png_image_finish_read(pngP, NULL, srcPixelA, 0, NULL);
          e = !e ? E_FILE_IO : SUCCESS;
          // mallocSanityCheck("after finishing reeading PNG 2");
        }
        if (!e & verbose) {
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
        if (!e) {
          // mallocSanityCheck("before making CP and CM");
          e = _makeColorPaletteAndColormap(&colormapA, cpP, cmP, pngP, srcPixelA, verbose);
          // mallocSanityCheck("after making CP and CM");
        }
        if (!e && verbose) {
          printf("\nColor palette in RGBA-type PNG:\n");
          for (Color_ *colorP = cpP->colorA; colorP < cpP->colorA + cpP->nColors; ++colorP) {
            printf("\t{%d, %d, %d, %d}\n", colorP->r, colorP->g, colorP->b, colorP->a);
          }
          printf("\n");
        }
        arrayDel((void**) &srcPixelA);
      }
    }

    if (pngP->warning_or_error) {
      printf("%s\n", pngP->message);
    }

  }

  if (!e && verbose) {
    _printColors(cpP);
    _printColormap(cmP);
  }

  // Set colormap's bpp.
  if (!e) {
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
      e = E_UNSUPPORTED_PIXEL_FORMAT;
    }
  }
  // Colormap's stripset & stripmap

  if (!e) {
    // If image is animated, strip length should be the width of an animation frame.
    // Otherwise, it should be the image width.
    U32 stripLen = cmP->w;
    if (animP && animP->frameNodeA) {
      stripLen = animP->frameNodeA[0].w;
    }
    e = stripNew(colormapA, stripLen, cmP->bpp, &cmP->sdP, verbose);
    //if (!e) {
      //e = stripIni(cmP->sdP);
    //}
    if (!e) {
      // We're gonna cheat and preview the ground truth image here. 
      // We'll test stripIni properly in _validateWholeInput().
      arrayDel((void**) &cmP->sdP->unstrippedDataA);
      cmP->sdP->unstrippedDataA = colormapA;
    }
    // Sanity check before making strips
    if (!e && verbose) {
      e = previewImg(cmP, cpP, 1000);
    }
  }

  if (!e && verbose) {
    _printColors(cpP);
    _printColormap(cmP);
  }

  if (!pngP->warning_or_error && verbose) {
    printf("PNG image load success: %d x %d \n", pngP->width, pngP->height);
  }

  if (e) { 
    cmClr(cmP);
  }
  jbFree((void**) &pngP);
  if (e) {
    arrayDel((void**) &colormapA);
  }

  return e;  // libpng errors are weird.
}

static Error _validateWholeInput(Colormap *cmP, ColorPalette *cpP) {
  if (!cmP || !cpP) {
    return E_BAD_ARGS;
  }

  // Deflate colormap/palette and get rid of their resulting outputs to truly test strip-piecing.
  U32 wOrig = cmP->w;
  U32 hOrig = cmP->h;
  U32 bppOrig = cmP->bpp;
  U8 *cmOrigP = NULL;
  Error e = arrayNew((void**) &cmOrigP, arrayGetElemSz(cmP->sdP->unstrippedDataA), arrayGetNElems(cmP->sdP->unstrippedDataA));
  if (!e) {
    memcpy(cmOrigP, cmP->sdP->unstrippedDataA, arrayGetElemSz(cmP->sdP->unstrippedDataA) * arrayGetNElems(cmP->sdP->unstrippedDataA));
  }
  // Clear out old colormap
  if (!e) {
    printf("original input\n");
    printStripData(cmP->sdP);
    e = previewImg(cmP, cpP, 1000);
  }
  if (!e) {
    cmClr(cmP);
  }
  if (!e) {
    e = stripIni(cmP->sdP);
  }
  if (!e) {
    printf("reconstructed input\n");
    printStripData(cmP->sdP);
  }
#if 1
  // Verify array lengths match.
  printf("orig cm has %d elems; new cm has %d\n", 
       arrayGetNElems(cmOrigP), arrayGetNElems(cmP->sdP->unstrippedDataA));
  printf("orig cm elem size is %d; new cm elem size is %d\n", 
       arrayGetElemSz(cmOrigP), arrayGetElemSz(cmP->sdP->unstrippedDataA));
  if (!e && (arrayGetNElems(cmP->sdP->unstrippedDataA) != arrayGetNElems(cmOrigP))) {
    e = E_BAD_ARGS;
  }
  // Verify array elem sizes match.
  if (!e && (arrayGetElemSz(cmP->sdP->unstrippedDataA) != arrayGetElemSz(cmOrigP))) {
    e = E_BAD_ARGS;
  }
#endif
  // Compare data directly
  U8 theyMatch = 0;
  if (!e) {
#if 0
    printf("\n\n\nOriginal colormap array:\n");
    for (U8 *p = cmOrigP; p < cmOrigP + arrayGetNElems(cmOrigP); ++p) {
      printf("%d", *p);
    }
    printf("\n\nNew colormap array:\n");
    for (U8 *p = cmP->sdP->unstrippedDataA; p < cmP->sdP->unstrippedDataA + arrayGetNElems(cmP->sdP->unstrippedDataA); ++p) {
      printf("%d", *p);
    }
    printf("\n\n\n");
#endif
    if ((theyMatch = !memcmp(cmP->sdP->unstrippedDataA, 
                           cmOrigP, 
                           arrayGetElemSz(cmP->sdP->unstrippedDataA) 
                           * arrayGetNElems(cmP->sdP->unstrippedDataA)))) {
      printf("\n\nthey match!\n");
    }
    else {
      printf("\n\nthey don't match :( \n");
    }
    if (theyMatch) {
      e = previewImg(cmP, cpP, 1000);
    }
  }

  arrayDel((void**) &cmOrigP);
  return e;
}

//##########################################
Error img(char *entityNameP, Database *cpDirP, Database *cmDirP, AnimJsonData *animP, U8 verbose) {
  U8 *pixelP = NULL;  // can be 1bpp colormap, 2bpp gray+alpha, or 4bpp RGBA
  U8 srcPixelSize = 0;
  ColorPalette cp = {0};
  Colormap cm = {0};
  U8 *groundTruthColormapA = NULL;
  char *imgFilePathP = NULL;

  // Only supports PNG for now
  Error e = getSrcFilePath(&imgFilePathP, "Body/Graybody/Colormap", entityNameP, ".png", verbose); 

  // Rule them out one by one till you find the culprit!
  // Read PNG into colormap and color palette in one fell swoop.
  if (!e) {
    e = readPng(imgFilePathP, &cm, &cp, animP, verbose);
  }

  if (!e && verbose) {
    _printColors(&cp);
    _printColormap(&cm);
  }
  // Write output source files 
  if (!e) {
    e = writeColormap(entityNameP, &cm, verbose);
  }
  if (!e) {
    e = writeColormapHeaderFile(entityNameP, verbose);
  }
  if (!e) {
    e = writeColorPaletteHeaderFile(entityNameP, verbose);
  }
  if (!e) {
    e = writeColorPalette(entityNameP, &cp, verbose);
  }
  // Verify input independently of how the output treats it
  if (!e && verbose) {
    e = _validateWholeInput(&cm, &cp);
  }
  if (!e && verbose) {
    //printStripData(cm.sdP);
  }

  // Update color palette database if necessary
  // TODO remove 0 &&
  if (0 && !e) {
    EntryData entryDataToFind = {0};
    entryDataToFind.palette.nColors = cp.nColors;
    memcpy(entryDataToFind.palette.paletteA, &cp, sizeof(U32) * cp.nColors);
    char *existingPaletteName = dbFindNameByValue(cpDirP, &entryDataToFind, verbose);
    if (!existingPaletteName) {
      genieAsk(
          "This palette is new. What would you like to name it?", 
          DIR_FILE, (List*) cpDirP, verbose);
      // Store our palette only because it doesn't already exist in the directory.
      dbAddEntry(cpDirP, entityNameP, &entryDataToFind, verbose);  
    }
    else {
      if (verbose) {
        printf("%s found a matching palette. Skipping write stage.\n", entityNameP);
      }
    }
  }

  cmClr(&cm);
  arrayDel((void**) &cp.colorA);
  jbFree((void**) &imgFilePathP);
  return e;
}
