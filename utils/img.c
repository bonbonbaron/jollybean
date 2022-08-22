#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <png.h>
#include "botox.h"
#include "data.h"


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

const static U8 PNG_SIGNATURE[]     = {137, 80, 78, 71, 13, 10, 26, 10};
const static U8 PNG_IHDR_START[]    = {0, 0, 0, 13, 73, 72, 68, 82};
const static U8 PNG_PLTE_CHUNK_TYPE[] = {80, 76, 84, 69};
const static U8 PNG_IDAT_CHUNK_TYPE[] = {73, 68, 65, 84};
const static U8 PNG_IEND[]      = {0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130};
const static U8 PNG_IDAT_START[]    = {'I', 'D', 'A', 'T'};
const static U8 PNG_IDAT_END[]    = {'I', 'E', 'E', 'D'};
const char TROVE_IMAGE_DIR[]      = "~/.jb/src/Image";

static verbose = 0;
typedef struct {
  U32 w, h, pitch, bpp;
} ImgDims;

typedef struct {
  U8 r, g, b, a;
} Pixel;

typedef U8 ColormapIdx;

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


// Checks for existence of multi-element sub-array in container array
//##########################################
U8 contains(Pixel *pixelA, U32 nPixels, const Pixel *pixelQueryP) {
  Pixel *pixelP = pixelA;
  Pixel *pixelEndP = pixelP + nPixels;
  for (; pixelP < pixelEndP; ++pixelP)
    if (!memcmp((const void*) pixelP, (const void*) pixelQueryP, sizeof(Pixel)))
      return 1;
  return 0;
}

Error readPng(png_image **imgPP, char *imgPathA, U32 w, U32 h, U32 bpp) {
  Pixel *pixelA = NULL;
  (*imgPP)->version = PNG_IMAGE_VERSION;
  (*imgPP)->opaque = NULL;
  int e = png_image_begin_read_from_file(*imgPP, imgPathA);
  if (!e) {
    (*imgPP)->format = PNG_FORMAT_RGBA;  // I think it'll xform TO this regardless of source type.
    pixelA = malloc(w * h * sizeof(Pixel));
  }
  png_color blackBg = {0};
  U32 pitch = w * bpp / 8;
  if (pixelA) 
    png_image_finish_read(*imgPP, &blackBg, pixelA, pitch, NULL);

  return (Error) e;
} 

typedef struct {
  U8 *flipIdxA;
  ColormapIdx *stripSetA;
} ColormapProfile;
//##########################################
#define STRIP_N_PIXELS (64) //images will use 8x8 s to ensure pixel count is multiple of STRIP_N_PIXELS
void makeStripmapFromColormap(ColormapIdx *colormapA, ColormapProfile *cmProfileP, png_image *imgDataP) {
  U32 nStrips = imgDataP->width * imgDataP->height * sizeof(ColormapIdx) / STRIP_N_PIXELS;
  U16 stripLabel = 0;
  // Allocate all these annoying arrays.
  // nStrips is the max possible number of strips, so prepare to have up to that many.
  U16 stripsLabelled[nStrips];
  memset(stripsLabelled, 0, sizeof(U16) * nStrips);
  ColormapIdx stripSet[nStrips];
  memset(stripSet, 0, sizeof(ColormapIdx) * nStrips);
  ColormapIdx flipSet[nStrips];
  memset(flipSet, 0, sizeof(ColormapIdx) * nStrips);
  U16 stripMap[nStrips];
  memset(stripMap, 0, sizeof(U8) * nStrips);
  U8 flipIdxA[nStrips];
  memset(flipIdxA, 0, sizeof(U8) * nStrips);
  
// Find distinct s and write an index-mapping to them.
  printf("Analyzing viability of breaking image into strips...");
  ColormapIdx *currStripP;
  ColormapIdx *nextStripP;  
  U32 nFlips = 0;
  for (U32 i = 0; i < nStrips; ++i) {
    // This strip hasn't been labelled yet. 
    // So give it its own label and add it to the set of unique strips.
    if (!stripsLabelled[i]) {
      memcpy((void*) &stripSet[stripLabel], &colormapA[i * STRIP_N_PIXELS], STRIP_N_PIXELS);
      stripsLabelled[i] = 1;
      stripMap[i] = stripLabel;
      /* All strips up to current "i" should be labelled, so search for strips
       * beyond that that match the current, newly labelled strip. */
      currStripP = colormapA + (STRIP_N_PIXELS * i);
      for (U32 ii = i + 1; ii < nStrips; ++ii) {
        if (!stripsLabelled[ii]) {
          nextStripP = colormapA + (ii * STRIP_N_PIXELS);
          if (!memcmp((const void*) nextStripP, 
              (const void*) currStripP, 
              sizeof(ColormapIdx) * STRIP_N_PIXELS)) {
            stripMap[ii] = stripLabel;  //flipped strip gets same label
            stripsLabelled[ii] = 1;
          }  // if strip is straight-up equal
          // Not identical as-is. Is it identical with flipping?
          else {
            for (U32 j = STRIP_N_PIXELS - 1; j >= 0; --j) {
              if (nextStripP[j] != currStripP[STRIP_N_PIXELS - j - 1])
                goto flipNotEqual;
            }
            flipIdxA[nFlips++] = ii;
            stripMap[ii] = stripLabel;
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
    memset(cmProfileP, 0, sizeof(ColormapProfile));
  }
  else {
    arrayNew((void**) &cmProfileP->flipIdxA, sizeof(flipIdxA[0]), nFlips);
    arrayNew((void**) &cmProfileP->stripSetA, sizeof(ColormapIdx), stripLabel);
  }
}

// TODO to reuse color palettes: Order by raw U32 (RGBA) value instead of brightness.
//      Then let JB figure out whether to copy the palette over or use an existing match.
//##########################################
def  get_keymap_psn(filename):
  underscore_position = 0
  for i in range(len(filename)):
    if filename[i] == "#":
      underscore_position = i + 1
      break
  return underscore_position

//##########################################
// returns array of indices
def rank_elems(array):
  L = len(array)
  result = np.zeros(L, np.uint8)
  for i in range(L):
    rank = 0
    for j in range(L):
      if array[j] < array[i]:
        rank += 1  // higher ranks for brighter pixels
    result[i] = rank
  return result

//##########################################
def get_color_palette(img):
  color_palette = []
  for x in range(img.shape[0]):
    for y in range(img.shape[1]):
      pixel = img[x, y]
      if not contains(color_palette, pixel):
        color_palette.append(pixel)
  return color_palette

//##########################################
def convertToRGBHex(rgbNPArray):
  r = rgbNPArray[0]
  g = rgbNPArray[1]
  b = rgbNPArray[2]
  return ((b & 0xFF) << 16) | ((g & 0xFF) << 8) | ((r & 0xFF) << 0)
// Gray-out sprite colors for data compression. Enforce SNES standards of 8 max colors/sprite.
// Color brightness formula:
//   https://stackoverflow.com/questions/59628/formula-to-determine-brightness-of-rgb-color
//##########################################
def iniColorMap(img):
// Get unique pixel values to make color palette
// I only want to support up to 8 colors like the SNES.
  color_palette = get_color_palette(img)
  colorDict = {}
  idx = 0

  for color in color_palette[:8]:
    rgbHex = convertToRGBHex(color)
    colorDict[rgbHex] = idx
    idx += 1
  colorDict[255] = len(color_palette)
   
  colormap = (255 * np.ones((img.shape[0], img.shape[1]))).astype(np.uint8)
// Map each value in color image to its rank
  for x in range(img.shape[0]):
    for y in range(img.shape[1]):
      rgbHex = convertToRGBHex(img[x, y])
      try:
        colormap[x, y] = colorDict[rgbHex]
      except:
        printf("trying to access colorDict with key %d"%(rgbHex))
        quit()
       
  return colormap, color_palette
   
//##########################################
def calcExpDeflatedNBytes(w, h, bpp):
  return (w * h * bpp) / 8

//##########################################
def packBytes(stripList, bpp):
  np_stripSet = np.zeros((len(stripList) * STRIP_N_PIXELS)).astype("uint8")
  for i in range(len(stripList)):
    np_stripSet[(i * STRIP_N_PIXELS) : ((i + 1) * STRIP_N_PIXELS)] = stripList[i]
  unpackedBytes = np_stripSet.flatten().astype("uint8")
  bpg = np.zeros(len(unpackedBytes) // (8 // bpp)).astype("uint8") //bits packed contiguous 
  bps = np.zeros(bpg.shape).astype("uint8") //bits packed staggered
  if bpp == 1:
// It's too messy to pack on the fly with our 8-bit staggering. Better to swap after.
// So pack as if every pixel is still contiguous first:
    for i in range(len(bpg)):
      bpg[i] = (unpackedBytes[i * 8] & 0x01 << 0) | \
               (unpackedBytes[i * 8] & 0x01 << 1) | \
               (unpackedBytes[i * 8] & 0x01 << 2) | \
               (unpackedBytes[i * 8] & 0x01 << 3) | \
               (unpackedBytes[i * 8] & 0x01 << 4) | \
               (unpackedBytes[i * 8] & 0x01 << 5) | \
               (unpackedBytes[i * 8] & 0x01 << 6) | \
               (unpackedBytes[i * 8] & 0x01 << 7).astype("uint8") 
// For every 4 bytes (U32), order bits as [0, 4, 8, 16, 20, 24, 28], [1, 5, 9, 13, 17, 21, 25, 29], ... etc. for the next 2 bytes
    for i in range(0, len(bpg) - 4, 4):
// only bother swapping bits if they're not the same
      for j in range(0, 8):
        bit = 1 << j
        if (bpg [i] & bit) ^ (bpg [i + 4] & bit): //doesn't need differentBits like the 2bpp and 4bpp cases
          bpg [i] ^= bit  //toggle the jth bit in each byte
          bpg [i + 4] ^= bit
  elif bpp == 2:
    for i in range(len(bpg)):
      bpg[i] = ((unpackedBytes[(i * 4) + 0] & 0x03) << 0) | \
           ((unpackedBytes[(i * 4) + 1] & 0x03) << 2) | \
           ((unpackedBytes[(i * 4) + 2] & 0x03) << 4) | \
           ((unpackedBytes[(i * 4) + 3] & 0x03) << 6).astype("uint8")
// For every 4 bytes (U32), order crumbs as [0, 4, 8, 12], [1, 5, 9, 13], ... etc. for the next 2 bytes
    for i in range(0, len(bpg) - 4, 4):
// only bother swapping bits if they're not the same
      for j in range(0, 8, 2):
        crumb = 0x03 << j
        differentBits = (bpg [i] & crumb) ^ (bpg [i + 4] & crumb)
        if differentCrumbs:
          bpg [i] ^= differentCrumbs  //toggle the jth bit in each byte
          bpg [i + 4] ^= differentCrumbs 
  elif bpp == 4:
    for i in range(len(bpg)):
      bpg[i] = ((unpackedBytes[(i * 2) + 0] & 0x0f) << 0) | \
           ((unpackedBytes[(i * 2) + 1] & 0x0f) << 4).astype("uint8")
// For every 4 bytes (U32), order nibbles as [0, 4], [1, 5], ... etc. for the next 2 bytes
    for i in range(0, bps.shape[0], 4): //[i] is a byte index
// 0th and 7th nibbles stay put... Order is confusing as hell with no rhyme or reason lol.
      bps[i + 0] = ((bpg[i + 0] & 0x0f) >> 0) | ((bpg[i + 2] & 0x0f) << 4)
      bps[i + 1] = ((bpg[i + 0] & 0xf0) >> 4) | ((bpg[i + 2] & 0xf0) << 0)
      bps[i + 2] = ((bpg[i + 1] & 0x0f) >> 0) | ((bpg[i + 3] & 0x0f) << 4)
      bps[i + 3] = ((bpg[i + 1] & 0xf0) >> 4) | ((bpg[i + 3] & 0xf0) << 0)

  return bps 

//##########################################
def unpackBytes(packedBytes, bpp):
  np_stripSet = np.zeros(len(packedBytes)).astype("uint8")
  colormap = np.zeros(len(packedBytes) * (8 // bpp)).astype("uint8")
  printf("colormap shape: " + str(colormap.shape))
  printf("packed bytes len: " + str(len(packedBytes)))
  j = 0
  if bpp == 1:
    for i in range(0, len(colormap), 32):
      colormap[i +  0] = ((packedBytes[j +  0] & 0x01) >> 0) 
      colormap[i +  1] = ((packedBytes[j +  1] & 0x01) >> 0)
      colormap[i +  2] = ((packedBytes[j +  2] & 0x01) >> 0)
      colormap[i +  3] = ((packedBytes[j +  3] & 0x01) >> 0)
      colormap[i +  4] = ((packedBytes[j +  0] & 0x02) >> 1)
      colormap[i +  5] = ((packedBytes[j +  1] & 0x02) >> 1)
      colormap[i +  6] = ((packedBytes[j +  2] & 0x02) >> 1)
      colormap[i +  7] = ((packedBytes[j +  3] & 0x02) >> 1)
      colormap[i +  8] = ((packedBytes[j +  0] & 0x04) >> 2) 
      colormap[i +  9] = ((packedBytes[j +  1] & 0x04) >> 2)
      colormap[i + 10] = ((packedBytes[j +  2] & 0x04) >> 2)
      colormap[i + 11] = ((packedBytes[j +  3] & 0x04) >> 2)
      colormap[i + 12] = ((packedBytes[j +  0] & 0x08) >> 3)
      colormap[i + 13] = ((packedBytes[j +  1] & 0x08) >> 3)
      colormap[i + 14] = ((packedBytes[j +  2] & 0x08) >> 3)
      colormap[i + 15] = ((packedBytes[j +  3] & 0x08) >> 3)
      colormap[i + 16] = ((packedBytes[j +  0] & 0x10) >> 4) 
      colormap[i + 17] = ((packedBytes[j +  1] & 0x10) >> 4)
      colormap[i + 18] = ((packedBytes[j +  2] & 0x10) >> 4)
      colormap[i + 19] = ((packedBytes[j +  3] & 0x10) >> 4)
      colormap[i + 20] = ((packedBytes[j +  0] & 0x20) >> 5)
      colormap[i + 21] = ((packedBytes[j +  1] & 0x20) >> 5)
      colormap[i + 22] = ((packedBytes[j +  2] & 0x20) >> 5)
      colormap[i + 23] = ((packedBytes[j +  3] & 0x20) >> 5)
      colormap[i + 24] = ((packedBytes[j +  0] & 0x40) >> 6) 
      colormap[i + 25] = ((packedBytes[j +  1] & 0x40) >> 6)
      colormap[i + 26] = ((packedBytes[j +  2] & 0x40) >> 6)
      colormap[i + 27] = ((packedBytes[j +  3] & 0x40) >> 6)
      colormap[i + 28] = ((packedBytes[j +  0] & 0x80) >> 7)
      colormap[i + 29] = ((packedBytes[j +  1] & 0x80) >> 7)
      colormap[i + 30] = ((packedBytes[j +  2] & 0x80) >> 7)
      colormap[i + 31] = ((packedBytes[j +  3] & 0x80) >> 7)
      j += 4
  elif bpp == 2:
    for i in range(0, len(colormap), 16):
      colormap[i +  0] = ((packedBytes[j +  0] & 0x03) >> 0) 
      colormap[i +  1] = ((packedBytes[j +  1] & 0x03) >> 0)
      colormap[i +  2] = ((packedBytes[j +  2] & 0x03) >> 0)
      colormap[i +  3] = ((packedBytes[j +  3] & 0x03) >> 0)
      colormap[i +  4] = ((packedBytes[j +  0] & 0x0c) >> 2)
      colormap[i +  5] = ((packedBytes[j +  1] & 0x0c) >> 2)
      colormap[i +  6] = ((packedBytes[j +  2] & 0x0c) >> 2)
      colormap[i +  7] = ((packedBytes[j +  3] & 0x0c) >> 2)
      colormap[i +  8] = ((packedBytes[j +  0] & 0x30) >> 4) 
      colormap[i +  9] = ((packedBytes[j +  1] & 0x30) >> 4)
      colormap[i + 10] = ((packedBytes[j +  2] & 0x30) >> 4)
      colormap[i + 11] = ((packedBytes[j +  3] & 0x30) >> 4)
      colormap[i + 12] = ((packedBytes[j +  0] & 0xc0) >> 6)
      colormap[i + 13] = ((packedBytes[j +  1] & 0xc0) >> 6)
      colormap[i + 14] = ((packedBytes[j +  2] & 0xc0) >> 6)
      colormap[i + 15] = ((packedBytes[j +  3] & 0xc0) >> 6)
      j += 4
  elif bpp == 4:
    for i in range(0, len(colormap), 8):
      colormap[i +  0] = ((packedBytes[j +  0] & 0x0f) >> 0) 
      colormap[i +  1] = ((packedBytes[j +  1] & 0x0f) >> 0)
      colormap[i +  2] = ((packedBytes[j +  2] & 0x0f) >> 0)
      colormap[i +  3] = ((packedBytes[j +  3] & 0x0f) >> 0)
      colormap[i +  4] = ((packedBytes[j +  0] & 0xf0) >> 4)
      colormap[i +  5] = ((packedBytes[j +  1] & 0xf0) >> 4)
      colormap[i +  6] = ((packedBytes[j +  2] & 0xf0) >> 4)
      colormap[i +  7] = ((packedBytes[j +  3] & 0xf0) >> 4)
      j += 4
  elif bpp == 8:
    pass
//printf(colormap)

  return colormap

//##########################################
def compress_img(img_fp):
  img_tokens = img_fp.split(os.sep)
  img_dir = (os.sep).join(img_tokens[:-1])
  if "_" in img_tokens[-1]:
    img_nm_tokens = img_tokens[-1].split("_")
    img_nm   = img_nm_tokens[0].split(".")[0]
    cp_nm  = img_nm_tokens[1]
  else:
    img_nm = img_tokens[-1].split(".")[0]
// FOR DEBUGGING
  origFileSz = os.path.getsize(img_fp)
  img = cv2.imread(img_fp)
   
  printf("image shape: %d x %d"%(img.shape[0], img.shape[1]))
  assert img.flatten().shape[0] % STRIP_N_PIXELS == 0

  color_palette = get_color_palette(img)
   
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
//~~~~~~~~~~~~~~~ Tilemap ~~~~~~~~~~~~~~~~#
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
  numColors = len(color_palette)
  if numColors <= 2:
    bpp = 1
  elif numColors <= 4:
    bpp = 2
  elif numColors <= 16:
    bpp = 4
  else:
    bpp = 8
// Whatever pixel format I settle on for Jollybean, the above sizes are for storage only.
   
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
//~~~~~ Colormap StripSet & StripSet ~~~~~~~#
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
  colormap, colorPalette = iniColorMap(img)
  stripSet, stripMap, flipList = mapStrips(colormap)
  stripSetPacked = bytes(packBytes(stripSet, bpp).tolist())

// I was RIGHT!! The PNG writer function sucks ass by corrupting data! Raw zlib compression it is then.

  printf("pre-compression length: %d"%(len(stripSetPacked)))
  from zlib import compress, decompress
  c = compress(stripSetPacked)
  d = decompress(c)
  printf("compressed length: %d"%(len(c)))
  printf("inflated length: %d"%(len(d)))
  
// Tell game engine how to deflate colormap stripSet
  if stripMap is None:
    printf("Image was not worth breaking into strips. Compressing unmapped, grayed-out image.")
    printf("number of strips: %d"%(img.flatten().shape[0] // STRIP_N_PIXELS))
  else:
    printf("Image was worth mapping strips for. Compressing strip set.")
    printf("number of strips: %d"%max(stripMap))

  inflatable = Inflatable(img_nm, len(c), len(d), c)
  inflatable.writeInflationData("%s/%s.c"%(img_dir, img_nm), (w, h, pitch, bpp))

// Give game engine the stripMap for this image if it applies.
  if stripMap is not None: //pixel strips were worth whittling down further for zlib compressor
    sm = StripMap(img_name, STRIP_N_PIXELS * len(stripMap), img.shape[1] * bpp, 8, bpp, flipList, stripMap)
  else:
    sm = StripMap(img_name, STRIP_N_PIXELS * len(stripMap), img.shape[1] * bpp, 8, bpp, flipList, stripMap)
    
  sm.write("%s/%sStripMap.c"%(img_dir, img_nm))

// Make sure the inflated image matches the input!     
  stripSet = unpackBytes(stripSetPacked, bpp)
  reconImg = reconstructImage(stripSet, stripMap, flipList, colorPalette, img.shape, bpp)
  printf("recon shape: %s... input shape: %s"%(str(reconImg.shape), str(img.shape)))
  if ((reconImg == img).all()):
    printf("Reconstructed image is identical to input.")
  else:
    printf("Reconstructed image is different from input.")

def proc_img(img_name):
  img_obj = compress_img(img_name)
  
def reconstructImage(stripSet, stripMap, flipList, colorPalette, imgShape, bpp):
// Construct colored-in stripSet
  img = np.zeros((imgShape[0] * imgShape[1], 3)).astype("uint8")
  fullImgColormap = np.zeros((imgShape[0] * imgShape[1])).astype("uint8")
// First write pixels in.
  if stripMap is not None:
    for i in range(stripMap.shape[0]):
      fullImgColormap[(i * STRIP_N_PIXELS) : ((i + 1) * STRIP_N_PIXELS)] \
      = stripSet[stripMap[i] * STRIP_N_PIXELS : (stripMap[i] + 1) * STRIP_N_PIXELS]
// Then flip whatever needs flipping.
    printf(flipList)
    for i in range(len(flipList)):
      fullImgColormap[(i * STRIP_N_PIXELS) : ((i + 1) * STRIP_N_PIXELS)] \
      = np.flip(fullImgColormap[(i * STRIP_N_PIXELS) : ((i + 1) * STRIP_N_PIXELS)])

  else: //input stripSet is a full image, not a strip set
    for i in range(fullImgColormap.shape[0]):
      fullImgColormap = stripSet

// Now color the pixels in.
//printf(img.shape) //(12992, 3)
//printf(fullImgColormap.shape)//(16384,)
//printf(len(colorPalette)) //7
  for i in range(fullImgColormap.shape[0]):
    img[i] = colorPalette[fullImgColormap[i]]

  img = img.reshape((imgShape[0], imgShape[1], imgShape[2]))
  printf("output img shape: %s"%(str(img.shape)))
  cv2.imwrite("./reconstructed.png", img)
  return img


args = sys.argv
if len(args) > 1:
  printf("running mkimg for each image in [" + ", ".join(args[1:])+ "].")
  for arg in args[1:]:
    pytime = os.path.getmtime(os.path.join(root, sc))
    proc_img(arg)
else:
  printf("No arguments provided. Exiting...")


