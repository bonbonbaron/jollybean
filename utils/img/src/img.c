#include "img.h"
#include <assert.h>
#include <string.h>

void imgDimsIni(ImgDims *imgDimsP, U32 width, U32 height, U32 bpp) {
  imgDimsP->w = width;
  imgDimsP->h = height;
  imgDimsP->bpp = bpp;
  imgDimsP->pitch = width * bpp / 8;
}

void writeRawData8(FILE *fP, U8 *byteA, U32 nBytes) {
  U8 *cmpDataP = byteA;
  U8 *cmpDataEndP = cmpDataP + nBytes;
  for (U32 counter = 0; cmpDataP < cmpDataEndP; ++cmpDataP) {
    fprintf(fP, "0x%02x, ", *cmpDataP);
    if (++counter >= 16) {
    fprintf(fP, "\n\t");
    counter = 0;
    }
  }
}

void writeRawData16(FILE *fP, U16 *byteA, U16 nBytes) {
  U16 *cmpDataP = byteA;
  U16 *cmpDataEndP = cmpDataP + nBytes;
  for (U16 counter = 0; cmpDataP < cmpDataEndP; ++cmpDataP) {
    fprintf(fP, "0x%02x, ", *cmpDataP);
    if (++counter >= 16) {
    fprintf(fP, "\n\t");
    counter = 0;
    }
  }
}


void writeRawData32(FILE *fP, U32 *byteA, U32 nBytes) {
  U32 *cmpDataP = byteA;
  U32 *cmpDataEndP = cmpDataP + nBytes;
  for (U32 counter = 0; cmpDataP < cmpDataEndP; ++cmpDataP) {
    fprintf(fP, "0x%02x, ", *cmpDataP);
    if (++counter >= 16) {
    fprintf(fP, "\n\t");
    counter = 0;
    }
  }
}


Error writeInflatable(char *imgNameA, U8 *cmpDataA, U32 cmpLen, U32 decompLen, ImgDims *imgDimsP) {
  if (!imgNameA || strlen(imgNameA) == 0 || !cmpDataA || !cmpLen || !decompLen || !imgDimsP)
    return E_BAD_ARGS;
  // Init output filepath.
  char fullPath[strlen(imgNameA) + strlen(TROVE_IMAGE_DIR) + 2];
  strcat(fullPath, TROVE_IMAGE_DIR);
  strcat(fullPath, imgNameA);
  strcat(fullPath, ".c");
  // Make Title format of image name.
  char imgTitle[strlen(imgNameA)];
  strcat(imgTitle, imgNameA);
  imgTitle[0] = toupper(imgTitle[0]);
  // Open file.
  FILE *fP = fopen(fullPath, "w");
  if (!fP)
    return E_NO_MEMORY;
  // Write to file's header and length data.
  fprintf(fP, "#include \"data.h\"\n\n");
  fprintf(fP, "Inflatable inflatable%s = {\n", imgTitle);
  fprintf(fP, "\t.compressedLen  = %d,\n", cmpLen);
  fprintf(fP, "\t.inflatedDataP  = NULL,\n");
  fprintf(fP, "\t.inflatedLen  = %d,\n", decompLen);
  fprintf(fP, "\t.compressedData = {\n\t");
  fprintf(fP, "\t%d,\n\t%d,\n\tNULL,\n\t", cmpLen, decompLen);
  // Write compressed data's raw bytes in hexadecimal format.
  writeRawData8(fP, cmpDataA, cmpLen);
  fprintf(fP, "\n\t}\n};");
  // Write image wrapper struct.
  fprintf(fP, "\n\nImage img%s = {%d, %d, %d, %d, NULL};", imgNameA, imgDimsP->w, imgDimsP->h, imgDimsP->pitch, imgDimsP->bpp);
  // Close file.
  fclose(fP);

  return SUCCESS;
}

Error writeStripmap(char *imgNameA, Stripset *stripsetP, ImgDims *imgDimsP) {
  char fullPath[strlen(imgNameA) + strlen(TROVE_IMAGE_DIR) + strlen(".c")];
  strcat(fullPath, TROVE_IMAGE_DIR);
  strcat(fullPath, imgNameA);
  strcat(fullPath, ".c");
  // Make Title format of image name.
  char imgTitle[strlen(imgNameA)];
  strcat(imgTitle, imgNameA);
  imgTitle[0] = toupper(imgTitle[0]);
  // Open file.
  FILE *fP = fopen(fullPath, "w");
  if (!fP)
    return E_NO_MEMORY;
  // Write header and image dim data.
  fprintf(fP, "#include \"data.h\"\n\n");
  fprintf(fP, "StripMap sm%s = {\n", imgTitle);
  fprintf(fP, "\t%d,\n\t%d\n\t%d,\n\t", arrayGetNElems(stripsetP->dataA), imgDimsP->pitch, imgDimsP->bpp);
  // Write index array.
  fprintf(fP, "U16 %sStripIdxA[] = {\n\t", imgNameA);
  writeRawData16(fP, (U16*) stripsetP->dataA, arrayGetNElems(stripsetP->dataA));
  fprintf(fP, "};\n\n");
  // Write flips.
  fprintf(fP, "U16 %sIdxFlipA[] = {", imgNameA);
  writeRawData16(fP, (U16*) stripsetP->flipIdxA, arrayGetNElems(stripsetP->flipIdxA));
  fprintf(fP, "};\n\n");
  // Close file.
  fclose(fP);

  return SUCCESS;
}


// =========================================================================================
// When you're building your color palette, this adds colors that don't already exist in it.
S32 getColormapIdx(Pixel *pixelA, U32 nPixels, const Pixel *pixelQueryP) {
  Pixel *pixelP = pixelA;
  Pixel *pixelEndP = pixelP + nPixels;
  for (; pixelP < pixelEndP; ++pixelP)
    if (!memcmp((const void*) pixelP, (const void*) pixelQueryP, sizeof(Pixel)))
      return pixelP - pixelA;
  return -1;
}

Error readPng(png_image **imgPP, char *imgPathA, void **pixelAP, png_color *colorPaletteP) {
  if (!imgPP || !imgPathA || !pixelAP)
    return E_BAD_ARGS;

  void *colormapA = NULL;
  png_color blackBg = {0}; // used to replace alpha pixels for 1-byte output pixel format
  *imgPP = malloc(sizeof(png_image));
  if (!*imgPP)
    return E_NO_MEMORY;

  memset(*imgPP, 0, sizeof(png_image));  // bombed, seeing if this worked
  (*imgPP)->version = PNG_IMAGE_VERSION;
  // Set up reader.
  int e = png_image_begin_read_from_file(*imgPP, imgPathA);
  // Allocate pixel array.
  if ((*imgPP)->warning_or_error)
    printf("Error message: %s\n", (*imgPP)->message);
  else if (!e) {
    printf("png_image_begin_read() errored with e = %d.\n", e);
    if ((*imgPP)->message[0]) 
      printf("%s\n", (*imgPP)->message);
  }

  // libpng errors work differently; 0 is bad, anything else is good.
  // TODO: Support all PNG formats.
  // Perform actual read.
  if (!(*imgPP)->warning_or_error) {
    U32 rowStride = PNG_IMAGE_ROW_STRIDE(**imgPP);
    U32 bufferSz = PNG_IMAGE_BUFFER_SIZE(**imgPP, rowStride);

    // Output pixel buffer
    e = arrayNew((void**) pixelAP, 1, bufferSz);
    if (e)
      return e;

    // Colormap (input) 
#if 0
    if ( (*imgPP)->colormap_entries ) {
      printf("Allocating %d bytes for colormap.\n", PNG_IMAGE_COLORMAP_SIZE(**imgPP));
      colormapA = malloc(PNG_IMAGE_COLORMAP_SIZE(**imgPP));
      if (!colormapA)
        return E_NO_MEMORY;
    }
#endif

    // row_stride param being 0 forces libpng to calculate the pitch for you.
    e = png_image_finish_read(*imgPP, NULL, *pixelAP, 0, colorPaletteP);
    if ((*imgPP)->warning_or_error) 
      printf("%s\n", (*imgPP)->message);
  }

  if (!(*imgPP)->warning_or_error && verbose)
    printf("no error on read; it says w = %d and h = %d.\n", (*imgPP)->width, (*imgPP)->height);

  //free(colormapA);
  if (!e)
    free(*pixelAP);
  
  return e ? SUCCESS : E_BAD_ARGS;  // libpng errors are weird.
} 

//##########################################
Error makeStripsetFromColormap(ColormapIdx *colormapA, Stripset *stripsetP, png_image *imgDataP) {
  U32 nStrips = imgDataP->width * imgDataP->height * sizeof(ColormapIdx) / STRIP_N_PIXELS;
  U16 stripLabel = 0;
  // Allocate all these annoying arrays.
  // nStrips is the max possible number of strips, so prepare to have up to that many.
  // Strips labelled so far
  StripmapIdx stripsLabelled[nStrips];
  memset(stripsLabelled, 0, sizeof(StripmapIdx) * nStrips);
  // Strip set
  ColormapIdx stripsetDataA[nStrips * STRIP_N_PIXELS];
  memset(stripsetDataA, 0, sizeof(ColormapIdx) * nStrips * STRIP_N_PIXELS);
  // Flip set 
  // This gets copied to a properly sized array at the end.
  ColormapIdx flipSet[nStrips];
  memset(flipSet, 0, sizeof(ColormapIdx) * nStrips);
  // Array of Index of strips to flip
  // This gets copied to a properly sized array at the end.
  StripmapIdx flipIdxA[nStrips];
  memset(flipIdxA, 0, sizeof(U8) * nStrips);
  // Strip map
  StripmapIdx *stripMapA = NULL;
  Error e = arrayNew((void**) &stripMapA, sizeof(StripmapIdx), nStrips);
  if (e)
    return e;

  if (verbose)
    printf("Analyzing viability of breaking image into strips...");

  // Find distinct strips and write an index-mapping to them.
  ColormapIdx *currStripP;
  ColormapIdx *nextStripP;  
  U32 nFlips = 0;
  for (U32 i = 0; i < nStrips; ++i) {
    // This strip hasn't been labelled yet. 
    // So give it its own label and add it to the set of unique strips.
    if (!stripsLabelled[i]) {
      memcpy((void*) &stripsetDataA[stripLabel], &colormapA[i * STRIP_N_PIXELS], STRIP_N_PIXELS);
      stripsLabelled[i] = 1;
      stripMapA[i] = stripLabel;
      /* All strips up to current "i" should be labelled, so search for strips
       * beyond that that match the current, newly labelled strip. */
      currStripP = colormapA + (STRIP_N_PIXELS * i);
      for (StripmapIdx ii = i + 1; ii < nStrips; ++ii) {
        if (!stripsLabelled[ii]) {
          nextStripP = colormapA + (ii * STRIP_N_PIXELS);
          if (!memcmp((const void*) nextStripP, 
              (const void*) currStripP, 
              sizeof(ColormapIdx) * STRIP_N_PIXELS)) {
            stripMapA[ii] = stripLabel;  //flipped strip gets same label
            stripsLabelled[ii] = 1;
          }  // if strip is straight-up equal
          // Not identical as-is. Is it identical with flipping?
          else {
            for (StripmapIdx j = STRIP_N_PIXELS - 1; j >= 0; --j) {
              if (nextStripP[j] != currStripP[STRIP_N_PIXELS - j - 1])
                goto flipNotEqual;
            }
            flipIdxA[nFlips++] = ii;
            stripMapA[ii] = stripLabel;
            stripsLabelled[i] = 1;
            flipNotEqual:
            continue;  // only because labels require something
          }  // else, see if flip is equal to strip
        }
      }
      ++stripLabel;
    }
  }
  // Slight compression isn't worth the trouble to inflate; keep as is.
  if (nStrips - stripLabel < 5) { //arbitrary number; maybe decide more scientific number later
    memset(stripsetP, 0, sizeof(Stripset));
  }
  else {
    // If it's worth doing, store compressed stuff into colormap profile.
    stripsetP->stripMapA = stripMapA;
    if (!e)
      e = arrayNew((void**) &stripsetP->flipIdxA, sizeof(flipIdxA[0]), nFlips);
    if (!e)
      e = arrayNew((void**) &stripsetP->dataA, sizeof(ColormapIdx), stripLabel);
    if (!e) {
      memcpy((void*) stripsetP->flipIdxA, (void*) flipIdxA, sizeof(StripmapIdx) * nFlips);
      memcpy((void*) stripsetP->dataA, (void*) stripsetDataA, sizeof(ColormapIdx) * stripLabel);
    }
    else {
      arrayDel((void**) &stripsetP->stripMapA);
      arrayDel((void**) &stripsetP->flipIdxA);
      arrayDel((void**) &stripsetP->dataA);
    }
  }
  return e;
}

//##########################################
Error makeColorPaletteAndColormap(Pixel *paletteA, U8 **colormapPP, U32 *nColorsP, png_image *imgDataP, Pixel *imgP) {
  if (!paletteA || !colormapPP || !nColorsP || !imgDataP || !imgP)
    return E_BAD_ARGS;

  // Allocate colormap.
  Error e = arrayNew((void**) colormapPP, sizeof(U8), imgDataP->width * imgDataP->height);
  if (!e) {
    // Set up pointers for looping through image.
    Pixel *pixelP = imgP;
    Pixel *pixelEndP = pixelP + (imgDataP->width * imgDataP->height);
    // Set up pointers for populating colormap.
    U8 *colormapP = *colormapPP;
    U8 *colormapEndP = colormapP + arrayGetNElems((void*) *colormapPP);
    S32 colormapIdx = -1;

    for (*nColorsP = 0; pixelP < pixelEndP && colormapP < colormapEndP; ++pixelP) {
      colormapIdx = getColormapIdx(paletteA, *nColorsP, pixelP);
      if (colormapIdx < 0) {
        paletteA[*nColorsP] = *pixelP;
        *colormapP = (*nColorsP)++;
      }
      else 
        *colormapP = (U8) colormapIdx;
    }
  }
  return e;
}

// Pack all the bits in strip set.
Error packBits(Stripset *stripsetP) {
  if (!stripsetP || !stripsetP->dataA)
    return E_BAD_ARGS;

  U32 nPixels = arrayGetNElems((void*) stripsetP->dataA);
  U8 *pixP = stripsetP->dataA;
  U8 *pixEndP = pixP + (nPixels);
  U32 *bpsA = NULL;     // array of packed bits
  U32 *bpsP, *bpsEndP;  // bits packed staggered
  Error e = SUCCESS;
  //bpg = np.zeros(len(unpackedBytes) // (8 // bpp)).astype("uint8") //bits packed contiguous 
  //bps = np.zeros(bpg.shape).astype("uint8") //bits packed staggered
  switch (stripsetP->bpp) {
    case 1:
      e = arrayNew((void**) &bpsA, sizeof(U32), nPixels / (32));  // 32 pixels per U32
      if (!e) {
        bpsP = bpsA;
        bpsEndP = bpsP + arrayGetNElems(bpsA);
        for (; bpsP < bpsEndP && pixP < pixEndP; ++bpsP, pixP += 32) {
          *bpsP = 
            (*(pixP+0)  << 0) | (*(pixP+1)  << 8) | (*(pixP+2)  << 16) | (*(pixP+3)  << 24) |
            (*(pixP+4)  << 1) | (*(pixP+5)  << 9) | (*(pixP+6)  << 17) | (*(pixP+7)  << 25) |
            (*(pixP+8)  << 2) | (*(pixP+9)  << 10) | (*(pixP+10) << 18) | (*(pixP+11) << 26) |
            (*(pixP+12) << 3) | (*(pixP+13) << 11) | (*(pixP+14) << 19) | (*(pixP+15) << 27) |
            (*(pixP+16) << 4) | (*(pixP+17) << 12) | (*(pixP+18) << 20) | (*(pixP+19) << 28) |
            (*(pixP+20) << 5) | (*(pixP+21) << 13) | (*(pixP+22) << 21) | (*(pixP+23) << 29) |
            (*(pixP+24) << 6) | (*(pixP+25) << 14) | (*(pixP+26) << 22) | (*(pixP+27) << 30) |
            (*(pixP+28) << 7) | (*(pixP+29) << 15) | (*(pixP+30) << 23)  | (*(pixP+31) << 31);
        }
      }
      break;
    case 2:
      e = arrayNew((void**) &bpsA, sizeof(U32), nPixels / (16));  // 16 pixels per U32
      if (!e) {
        bpsP = bpsA;
        bpsEndP = bpsP + arrayGetNElems(bpsA);
        for (; bpsP < bpsEndP && pixP < pixEndP; ++bpsP, pixP += 32) {
          *bpsP = 
            (*(pixP+0)  << 0) | (*(pixP+1)  << 8) |  (*(pixP+2)  << 16) | (*(pixP+3)  << 24) |
            (*(pixP+4)  << 2) | (*(pixP+5)  << 10) | (*(pixP+6)  << 18) | (*(pixP+7)  << 26) |
            (*(pixP+8)  << 4) | (*(pixP+9)  << 12) | (*(pixP+10) << 20) | (*(pixP+11) << 28) |
            (*(pixP+12) << 6) | (*(pixP+13) << 14) | (*(pixP+14) << 22) | (*(pixP+15) << 30);
        }
      }
      break;
    case 4:
      e = arrayNew((void**) &bpsA, sizeof(U32), nPixels / (8));  // 8 pixels per U32
      if (!e) {
        bpsP = bpsA;
        bpsEndP = bpsP + arrayGetNElems(bpsA);
        for (; bpsP < bpsEndP && pixP < pixEndP; ++bpsP, pixP += 32) {
          *bpsP = 
            (*(pixP+0)  << 0) | (*(pixP+1)  << 8) |  (*(pixP+2)  << 16) | (*(pixP+3)  << 24) |
            (*(pixP+4)  << 4) | (*(pixP+5)  << 12) | (*(pixP+6)  << 20) | (*(pixP+7)  << 28);
        }
      }
      break;
    case 8:
      return SUCCESS;  // No changes need to be made as no packing can be done.
      break;
    default:
      if (verbose)
        printf("Jollybean doesn't support more than 256 colors in an image.\n");
      return E_BAD_ARGS;
      break;
  }

  if (!e && stripsetP->bpp < 8) {
    arrayDel((void**) &stripsetP->dataA);
    stripsetP->dataA = (ColormapIdx*) bpsP;
  }
  else if (e)
    arrayDel((void**) &bpsP);

  return e;
}

void toUpper(char *stringP) {
  for (char *charP = stringP; *charP; ++charP) 
    if (*charP >= 'a' && *charP <= 'z')
      *charP -= 32;
}

//##########################################
Error compressImg(char *imgFilePathP) {
  void *pixelP = NULL;  // i think this is actually meant to be colormap data
  Pixel colorPaletteA[256] = {0};
  U8 *colormapA = NULL;
  png_image *pngImgP = NULL;
  U32 nColors = 0;
  Stripset stripset = {0};
  ImgDims imgDims = {0};

  Error e = readPng(&pngImgP, imgFilePathP, &pixelP, &colorPaletteA[0]);

  if (!e && (nColors = pngImgP->colormap_entries) == 0)
    e = makeColorPaletteAndColormap(colorPaletteA, &colormapA, &nColors, pngImgP, pixelP);
  else 
    colormapA = pixelP;

  // Compute pixel bitdepth.
  if (!e) {
    if (nColors <= 2)
      imgDimsIni(&imgDims, pngImgP->width, pngImgP->height, 1);
    else if (nColors <= 4)
      imgDimsIni(&imgDims, pngImgP->width, pngImgP->height, 2);
    else if (nColors <= 16)
      imgDimsIni(&imgDims, pngImgP->width, pngImgP->height, 4);
    else
      imgDimsIni(&imgDims, pngImgP->width, pngImgP->height, 8);  // assume no greater than 8 bits per pixel

    stripset.bpp = imgDims.bpp;
  }

  // Colormap StripSet  & StripMap
  if (!e)
    e = makeStripsetFromColormap(colormapA, &stripset, pngImgP);
  if (!e)
    e = packBits(&stripset);
  if (verbose && !e)
    printf("pre-compression length: %d", (arrayGetNElems(stripset.dataA)));

  Inflatable *stripsetInflatableP = NULL;
  char fp[500] = {0};
  char entityName[250] = {0};
  char keyName[250] = {0};

  // Requires inflatable from data.h and util's inflatable writer function.
  if (!e)
    e = inflatableNew((void*) stripset.dataA, &stripsetInflatableP);
  if (!e) {
    U32 entityNameIdx, entityNameLen, keyNameIdx, keyNameLen;
    parseEntityAndKeyNames(imgFilePathP, ".png", &entityNameIdx, &entityNameLen, &keyNameIdx, &keyNameLen);
    memcpy((void*) entityName, (void*) &imgFilePathP[entityNameIdx], entityNameLen);
    memcpy((void*) keyName, (void*) &imgFilePathP[keyNameIdx], keyNameLen);
    toUpper(keyName);  // key is going to be an enum, so format it accordingly
#if TEST_
    printf("Entity name is %s.\n", entityName);
    printf("Key name is %s.\n", keyName);
#endif

    // Generate stripset's inflatable C source file
    strcpy(fp, TROVE_IMAGE_DIR);
    strcat(fp, entityName);
    strcat(fp, ".c");
    printf("fp is %s.\n", fp);

    e = inflatableWrite(stripsetInflatableP, fp, entityName);
  }

  // Give game engine the stripMap for this image if it applies.
  if (!e && entityName[0])
    writeStripmap(entityName, &stripset, &imgDims);

  return e;
}

// An image name looks like this: /some/path/to/entityName#keyName.png
// A base name looks like this: entityName#keyName
// Extension is the file extension including the leading '.'.
// The key name (string after KEY_DELIMITER) is used to map an enum to an animation strip.
// The entity name is assumed to begin at the start of base name.
// The key name is assumed to extend to the end of the base name.
void parseEntityAndKeyNames(char *filepathP, char *extension, U32 *entityNameIdxP, U32 *entityNameLenP, U32 *keyNameIdxP, U32 *keyNameLenP) {
  U32 filenameLen = strlen(filepathP);
  U32 extLen = strlen(extension);  // including the leading "."
  // Make sure this file has a ".png" extension.
  assert(!strncasecmp(&filepathP[filenameLen - extLen], extension, extLen));

  // Init all outputs to zero.
  *entityNameIdxP = *keyNameIdxP = *entityNameLenP = *keyNameLenP = 0;

  // Get both entity and key's starting positions.
  for (U8 i = filenameLen - 1; i >= 0; --i) {
    switch (filepathP[i]) {
      case KEY_DELIMITER:
        *keyNameIdxP = i + 1;
        break;
      case FILE_SEP:
        *entityNameIdxP = i + 1;
        goto breakout;
    }
  }
  breakout:
  // Get both entity and key's lengths.
  if (*keyNameIdxP) {
    *entityNameLenP = *keyNameIdxP - *entityNameIdxP - 1;  // 1 is the '#'
    *keyNameLenP = filenameLen - *keyNameIdxP - extLen;
  }
  else 
    *entityNameLenP = filenameLen - *entityNameIdxP - extLen;
}

// Problem with this design is being unable to edit all the animation strips of 
// the same entity at the same time. If you change the color in one animation strip, 
// you're in big trouble. But the rationale for separating it out was to make key-giving easy.
// Aseprite doesn't exactly provide that easy functionality. Until someone makes a jollybean-
// compatible Aseprite (and it sure as hell ain't gonna be me), this is the way we'll do it.

int main (int argc, char **argv) {
  Error e = SUCCESS;
  if (argc > 1) {  // argv[0] is this program's name
    // iterate through arguments
    for (int i = 1; i < argc; ++i) {
      if (!memcmp(argv[1], "-v", strlen(argv[i])))
        verbose = 1;
      else  {
        e = compressImg(argv[i]);
      }
    }
  }
  return e;
}

