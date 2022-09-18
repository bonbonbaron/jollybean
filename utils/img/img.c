#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <png.h>
#include "botox.h"
#include "data.h"
#include "inflatable.h"

#define IDX_R           0
#define IDX_G           1
#define IDX_B           2
#define TILE_WIDTH        8
#define TILE_HEIGHT       8
#define NBR_COLOR_CHANNELS    3
#define TILE_SIZE = TILE_WIDTH * TILE_HEIGHT * NBR_COLOR_CHANNELS
#define PNG_BITDEPTH_LOCATION  24
#define PNG_BITDEPTH_DATA_SZ   1
#define PNG_IDATA_LOCATION   PNG_BITDEPTH_LOCATION + 8
#define PNG_IEND_LENGTH    8
#define IMG_DIR_IDX_NAME     0
#define IMG_DIR_IDX_LOCATION   1
#define IMG_DIR_IDX_NBR_TILES  2

void getBaseNameIndices(char *filepathP, U32 *startIdxP, U32 *endIdxP);

const static U8 PNG_SIGNATURE[]     = {137, 80, 78, 71, 13, 10, 26, 10};
const static U8 PNG_IHDR_START[]    = {0, 0, 0, 13, 73, 72, 68, 82};
const static U8 PNG_PLTE_CHUNK_TYPE[] = {80, 76, 84, 69};
const static U8 PNG_IDAT_CHUNK_TYPE[] = {73, 68, 65, 84};
const static U8 PNG_IEND[]      = {0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130};
const static U8 PNG_IDAT_START[]    = {'I', 'D', 'A', 'T'};
const static U8 PNG_IDAT_END[]    = {'I', 'E', 'E', 'D'};
const char TROVE_IMAGE_DIR[]      = "~/.jb/src/Image";

static int verbose = 0;

typedef struct {
  U32 w, h, pitch, bpp;
} ImgDims;

typedef struct {
  U8 r, g, b, a;
} Pixel;

typedef struct {
  U8 *flipIdxA;
  ColormapIdx *dataA;
  U16 *stripMapA;
  U8 bpp;
} Stripset;

typedef U8 ColormapIdx;
typedef U16 StripmapIdx;

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

Error writeStripmap(char *imgNameA, U16 *idxA, U16 nIndices, ImgDims *imgDimsP, U16 *flipA, U16 nFlips, U32 nBytes) {
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
  fprintf(fP, "\t%d,\n\t%d\n\t%d,\n\t", nBytes, imgDimsP->pitch, imgDimsP->bpp);
  // Write index array.
  fprintf(fP, "U16 %sStripIdxA[] = {\n\t", imgNameA);
  writeRawData16(fP, idxA, nIndices);
  fprintf(fP, "};\n\n");
  // Write flips.
  fprintf(fP, "U16 %sIdxFlipA[] = {", imgNameA);
  writeRawData16(fP, flipA, nFlips);
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

Error readPng(png_image **imgPP, char *imgPathA, Pixel **pixelAP) {
  if (!imgPP || !imgPathA || !pixelAP)
    return E_BAD_ARGS;

  (*imgPP)->version = PNG_IMAGE_VERSION;
  (*imgPP)->opaque = NULL;
  // Set up reader.
  int e = png_image_begin_read_from_file(*imgPP, imgPathA);
  // Allocate pixel array.
  if (!e) {
    (*imgPP)->format = PNG_FORMAT_RGBA;  // I think it'll xform TO this regardless of source type.
    *pixelAP = malloc((*imgPP)->width * (*imgPP)->height * sizeof(Pixel));
  }
  // TODO: Support all PNG formats.
  // Perform actual read.
  if (*pixelAP) {
    png_color blackBg = {0}; // Black background color for "blank" pixels.
    U32 pitch = (*imgPP)->width;  // Assume bpp doesn't factor in right now.
    png_image_finish_read(*imgPP, &blackBg, pixelA, pitch, NULL);
  }
  return (Error) e;
} 
 
//##########################################
#define STRIP_N_PIXELS (64) //images will use 8x8 s to ensure pixel count is multiple of STRIP_N_PIXELS
Error makeStripsetFromColormap(ColormapIdx *colormapA, Stripset *stripsetP, png_image *imgDataP) {
  U32 nStrips = imgDataP->width * imgDataP->height * sizeof(ColormapIdx) / STRIP_N_PIXELS;
  U16 stripLabel = 0;
  // Allocate all these annoying arrays.
  // nStrips is the max possible number of strips, so prepare to have up to that many.
  // Strips labelled so far
  StripmapIdx stripsLabelled[nStrips];
  memset(stripsLabelled, 0, sizeof(StripmapIdx) * nStrips);
  // Strip set
  ColormapIdx dataA[nStrips];
  memset(dataA, 0, sizeof(ColormapIdx) * nStrips);
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
      memcpy((void*) &dataA[stripLabel], &colormapA[i * STRIP_N_PIXELS], STRIP_N_PIXELS);
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
      memcpy((void*) stripsetP->dataA, (void*) dataA, sizeof(ColormapIdx) * stripLabel);
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

//##########################################
Error compressImg(char *imgFilePathP) {
  Pixel *pixelP = NULL;
  Pixel colorPaletteA[256] = {0};
  U8 *colormapA = NULL;
  png_image *pngImgP = NULL;
  U32 nColors = 0;
  U32 bpp = 0;
  Stripset stripset = {0};
  Error e = readPng(&pngImgP, imgFilePathP, &pixelP);

  if (!e)
    e = makeColorPaletteAndColormap(colorPaletteA, &colormapA, &nColors, pngImgP, pixelP);
   
  // Compute pixel bitdepth.
  if (!e) {
    if (nColors <= 2)
      bpp = 1;
    else if (nColors <= 4)
      bpp = 2;
    else if (nColors <= 16)
      bpp = 4;
    else
      bpp = 8;
  }

  // Colormap StripSet  & StripMap
  if (!e)
    e = makeStripsetFromColormap(colormapA, &stripset, pngImgP);
  if (!e)
    e = packBits(&stripset);
  if (verbose && !e)
    printf("pre-compression length: %d", (arrayGetNElems(stripset.dataA)));

  Inflatable *stripsetInflatableP = NULL;
  if (!e)
    e = inflatableNew((void*) stripset.dataA, &stripsetInflatableP);
  if (!e) {
    U32 startIdx, endIdx;
    getBaseNameIndices(imgFilePathP, &startIdx, &endIdx) {
    char fp[500] = {0};
    char imgNameA[100] = {0};
    memcpy((void*) imgNameA, (void*) &imageFilePathP[startIdx], endIdx - startIdx);
    // Generate stripset's inflatable  C source file
    strcpy(fp, TROVE_IMAGE_DIR);
    strcat(fp, imgNameA);
    strcat(fp, ".c");

    e = inflatableWrite(stripsetInflatableP, fp, imgNameA);
  }

  // Give game engine the stripMap for this image if it applies.
  if (!e)
    writeStripmap(img_name, STRIP_N_PIXELS * len(stripMap), img.shape[1] * bpp, 8, bpp, flipList, stripMap)

  return e;
}

void getBaseNameIndices(char *filepathP, char *extension, U32 *startIdxP, U32 *endIdxP) {
  U8 filenameLen = strlen(filepathP);
  U8 basenameFirstIdx = 0;
  U8 extLen = strlen(extension) + 1;  // including the leading "."
  for (U8 i = filenameLen - 1; i >= 0; --i) {
    char currChar = filepathP[i];
    if (currChar == FILE_SEP) {
      basenameFirstIdx = i + 1;
      break;
    }
  }
  *startIdxP = basenameFirstIdx;
  *endIdxP   = filenameLen - extLen;   
}

int main (int argc, char **argv) {
  if (argc >= 1) {
    if (!memcmp(argv[1], "-v"))
      verbose = 1;

    Error e = SUCCESS;
    for (U8 i = 0; !e && i < argc; ++i)
      e = compressImg(argv[i]);
  }

  return 0;
}
