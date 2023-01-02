#include "fileUtils.h"
#include "img.h"
#include "anim.h"
// Rect lists
typedef struct _RectNode {
  S32 x, y, w, h;
  struct _RectNode *nextP;
} RectNode;

static Error rectNodeNew(RectNode **nodePP) {
  if (!nodePP)
    return E_BAD_ARGS;
  Error e = jbAlloc((void**) nodePP, sizeof(RectNode), 1);
  if (!e) {
    (*nodePP)->x = -1;
    (*nodePP)->y = -1;
    (*nodePP)->w = -1;
    (*nodePP)->h = -1;
    (*nodePP)->nextP = NULL;
  }
  return e;
}

static Error rectNodeGrow(RectNode **nodePP) {
  if (!nodePP) {
    return E_BAD_ARGS;
  }
  Error e = SUCCESS;
  if (!*nodePP) {
    e = rectNodeNew(nodePP);
  } else {
    e = rectNodeNew(&(*nodePP)->nextP);
    if (!e) {
      *nodePP = (*nodePP)->nextP;
    }
  }
  return e;
}

static void rectNodeDel(RectNode **nodePP) {
  if (nodePP && *nodePP) {
    jbFree((void**) nodePP);
  }
}

static void rectListDel(RectNode **nodePP) {
  if (nodePP && *nodePP) {
    if ((*nodePP)->nextP) {
      rectListDel(&(*nodePP)->nextP);
    } else {
      rectNodeDel(nodePP);
    }
  }
}

RectNode* getRectNode(RectNode *rootP, U32 idx) {
  if (!rootP) {
    return NULL;
  }
  RectNode *resultP = rootP;
  for (int i = 0; i < idx; ++i)
    resultP = resultP->nextP;
  return resultP;
}


// Pixel calculations
static U8* calcFirstPixelP(FrameNode *fNodeP, U8 *pixelA, U32 pixelSize, U32 imgPitch) {
  if (fNodeP) {
    U8 *ptr = pixelA 
      + (fNodeP->x * pixelSize)  // starting from frame's first pixel's column...
      + (fNodeP->y * imgPitch);  // ... jumping to start of frame's last row
    return ptr;
  }
  else {
    return pixelA;
  }
}

static U8* calcLastPixelP(FrameNode *fNodeP, U8 *firstPixelP, U32 pixelSize) {
  if (fNodeP) {
    U8 *ptr = firstPixelP + (fNodeP->w * pixelSize) * (fNodeP->h);
    return ptr;
  }
  else {
    // assuming firstPixel is the start of the array, as per calcFirstPixel()
    return firstPixelP + (arrayGetNElems(firstPixelP) * arrayGetElemSz(firstPixelP));
  }
}

static U8* calcLastPixelPInRowP(FrameNode *fNodeP, U8 *firstPixelP, U32 pixelSize) {
  if (fNodeP) {
    U8 *ptr = firstPixelP + (fNodeP->w * pixelSize);
    return ptr;
  }
  else {
    // assuming firstPixel is the start of the array, as per calcFirstPixel()
    return firstPixelP + (arrayGetNElems(firstPixelP) * arrayGetElemSz(firstPixelP));
  }
}

// Collision rect coords are offsets from current animation frame's top-left corner.
#define findRect_(bpp_) \
  /* if this is an animated collison rectangle... */ \
  if (animP) { \
    /* for each frame... */ \
    for (FrameNode *fNodeP = animP->frameNodeA; !e && fNodeP; fNodeP = fNodeP->nextP) { \
      /* Grow a new frame node either from root or head. */ \
      if (!*currRectNodePP) { \
        e = rectNodeGrow(currRectNodePP); \
        firstRectNodeP = *currRectNodePP; \
      } \
      else { \
        e = rectNodeGrow(currRectNodePP); \
      } \
      /* If rect node creation succeeded... */ \
      if (!e) { \
        RectNode *rectNodeP = *currRectNodePP; \
        U##bpp_ *pixelP = (U##bpp_*) calcFirstPixelP(fNodeP, (U8*) pixelA, pixelSize, imgPitch); \
        /* Number of pixels to jump from end of current frame's row to the start of its next row */\
        U32 nPixelsToNextRow = imgPitch - (fNodeP->w * pixelSize); \
        /* for each row in frame... */ \
        for (int i = 0; i < fNodeP->h; ++i, pixelP += nPixelsToNextRow) { \
          U##bpp_ *pixelRowEndP = (U##bpp_*) calcLastPixelPInRowP(fNodeP, (U8*) pixelP, pixelSize); \
          /* for each pixel in frame's current row... */ \
          for (int j = 0; pixelP < pixelRowEndP; ++j, ++pixelP) { \
            if (*pixelP) { \
              printf("1");\
              if (rectNodeP->x < 0) { \
                rectNodeP->x = j; \
              } \
              if (rectNodeP->y < 0) { \
                rectNodeP->y = i; \
              } \
              rectNodeP->h = i - rectNodeP->y + 1; \
            } \
            /* If you're at row's end and haven't terminated the width yet */ \
            if (j == fNodeP->w - 1 && rectNodeP->w < 0) { \
              rectNodeP->w = j - rectNodeP->x; \
            } \
            /* If you hit an empty pixel... */ \
            if (!pixelP) { \
              if (rectNodeP->w < 0) { \
                rectNodeP->w = j - rectNodeP->x; \
              } \
            } \
            /* If you're at frame's end or have hit an empty pixel after starting rect... */ \
            if (!*pixelP) { \
              printf("0");\
            }  /* If you've hit an empty pixel and you've found the rect's start already... */ \
          }  /* for each pixel in frame's current row */ \
          printf("\n");\
        }  /* for each row in frame... */ \
        printf("rect xywh: %d %d %d %d\n\n", rectNodeP->x, rectNodeP->y, rectNodeP->w, rectNodeP->h); \
      }  /* if rect node creation succeeded... */ \
    }  /* for each frame... */ \
  }  /* if this is an animated collsion rectangle... */ \
  /* If this is not animated, you only need to find one rectangle. */ \
  else { \
    U##bpp_ *pixelP = (U##bpp_*) calcFirstPixelP(NULL, (U8*) pixelA, pixelSize, imgPitch); \
    U##bpp_ *lastPixelP = (U##bpp_*) calcLastPixelP(NULL, (U8*) pixelP, pixelSize); \
    e = rectNodeGrow(rectNodePP); \
    /* If rect node creation succeeded... */ \
    if (!e) { \
      RectNode rect = **rectNodePP; \
      for (; pixelP < lastPixelP; ++pixelP) { \
        if (*pixelP) { \
          if (rect.x < 0 ) { \
            rect.x = (pixelP - (U##bpp_*) pixelA) % imgPitch; \
          } \
          if (rect.y < 0) { \
            rect.y = (pixelP - (U##bpp_*) pixelA) / imgPitch; \
          } \
        } \
        /* If you've hit an empty pixel and you've found the rect's start already... */ \
        else { \
          if (rect.x >= 0 && !rect.w) { \
            rect.w = ((pixelP - (U##bpp_*) pixelA) % imgPitch) - rect.x; \
          } \
          if (rect.y >= 0) {  /* height needs to be continually updated till end of rect */ \
            rect.h = ((pixelP - (U##bpp_*) pixelA) / imgPitch) - rect.y; \
          } \
        }  /* If you've hit an empty pixel and you've found the rect's start already... */ \
      } \
    } \
  } 

Error findCollisionRects(U8 *pixelA, png_image *imgP, U32 pixelSize, Animation *animP, RectNode **rectNodePP) {
  if (!pixelA || !imgP || !pixelSize || pixelSize > 4 || !rectNodePP) {
    return E_BAD_ARGS;
  }
  U32 nPixels = imgP->width * imgP->height;
  U32 imgPitch = imgP->width * pixelSize;
  Error e = SUCCESS;

  // Keep track of the first node so we don't pass out the most recent one and lose all predecessors. 
  RectNode **currRectNodePP = rectNodePP; 
  RectNode *firstRectNodeP = NULL; 

  // Make your pappy proud and go find some rectangles, son.
  switch (pixelSize) {
    case 1:
      findRect_(8)
      break;
    case 2:
      findRect_(16)
      break;
    case 4:
      findRect_(32)
      break;
    default:
      return E_UNSUPPORTED_PIXEL_FORMAT;
  }

  *rectNodePP = firstRectNodeP;

  return e;
}

// Write a collision grid to a C file.
Error writeCollisionGridMap(char *entityName, StripMapS *stripmapP, StripSetS *stripsetP, U8 verbose) {
  return writeStripData(entityName, "CollGrid", verbose, stripsetP, stripmapP);
}

// Write a map of rectangle arrays to a C file.
Error writeCollisionRectMap(char *entityName, Animation *animP, RectNode *collTreeP) {
  char *filepath = NULL;
  Error  e = getBuildFilePath(&filepath, "Collision/Rect", entityName, "_col.c");
  if (!e) {
    FILE *fP = fopen(filepath, "w");
    Key nKeyValPairs = 0;
    if (!fP) {
      printf("failed to open %s\n", filepath);
      return E_NO_MEMORY;
    }
    fprintf(fP, "#include \"xCollision.h\"\n\n");
    // Write collision rect arrays.
    for (TagNode *tagP = animP->tagNodeA; tagP != NULL; tagP = tagP->nextP) {
      // Collision rect arrays
      fprintf(fP, "CollRect *collRect_%s_%s_A[] = {\n", entityName, tagP->name);
      RectNode *rectNodeP = getRectNode(collTreeP, tagP->from);
      if (!rectNodeP) {
        return E_BAD_ARGS;
      }
      for (int i = tagP->from; rectNodeP && i <= tagP->to; ++i, rectNodeP = rectNodeP->nextP) {
        if (rectNodeP) {
          fprintf(fP, "\t{\n");
          fprintf(fP, "\t\t.x = %d,\n", rectNodeP->x);
          fprintf(fP, "\t\t.y = %d,\n", rectNodeP->y);
          fprintf(fP, "\t\t.w = %d,\n", rectNodeP->w);
          fprintf(fP, "\t\t.h = %d,\n", rectNodeP->h);
          fprintf(fP, "\t},\n");
        }
      }
      fprintf(fP, "};\n\n");
      // CollisionTree strips
      fprintf(fP, "CollStrip collStrip_%s_%s = {\n", entityName, tagP->name);
      fprintf(fP, "\t.nFrames = %d,\n", 1 + tagP->to - tagP->from);
      int len = strlen(tagP->name);
      Bln isLeft = !strncasecmp(&tagP->name[len - 4], "LEFT", 4);
      fprintf(fP, "\t.flip = %d,\n", isLeft);
      // Aseprite left out looping in their tags for some reason, so using the otherwise useless reverse!
      Bln isReverse = !strncasecmp(tagP->direction, "reverse", 4);
      fprintf(fP, "\t.repeat = %d,\n", isReverse);  
      Bln isPingPong = !strncasecmp(tagP->direction, "pingpong", 4);
      fprintf(fP, "\t.pingPong = %d,\n", isPingPong);
      fprintf(fP, "\t.frameA = collRect_%s_%sA\n", entityName, tagP->name);
      fprintf(fP, "};\n\n");
      ++nKeyValPairs;
    }
    // Write key-val pairs between tag names and coll strips
    fprintf(fP, "KeyValPairArray tagName2CollRectAMap_%s[] = {\n", entityName);
    fprintf(fP, "\t.nKeyValPairs = %d,\n", nKeyValPairs);
    fprintf(fP, "\t.keyValPairA = {\n");
    for (TagNode *tagP = animP->tagNodeA; tagP != NULL; tagP = tagP->nextP) {
      fprintf(fP, "\t\t{\n");
      fprintf(fP, "\t\t\t.key  = %s,\n", tagP->name);
      fprintf(fP, "\t\t\t.valP = &collStrip_%s_%s\n", entityName, tagP->name);
      fprintf(fP, "\t\t},\n");
    }
    fprintf(fP, "};\n\n");
    fclose(fP);
  }
  return e;
}


// If animation is passed in, then we want to find the collision rect in each individual frame.
// 
// We consider each rectangle to have only one collision type, but the type may differ from 
// one animation frame to the next.
//
// If it's a background, we treat it as a pixel-perfect (AKA grid) collision object with 
// multiple collision types (where each pixel is a collision type value).
//
// For now at least, the input cannot be an animated background. (But what about water!? Hmmm...)
Error coll(char *fp, char *entityName, U8 isBg, Animation *animP, U8 verbose) {
  if (!fp || (isBg && animP)) {
    return E_BAD_ARGS;
  }
  StripSetS stripset = {0};
  StripMapS stripmap = {0};
  png_image *pngImgP = NULL;
  U8 pixelSize = 0;
  U32 nColors;
  U8 bpp = 0;
  U8 *pixelA = NULL;
  U8 *colorPaletteA = NULL;
  U8 *colormapA = NULL;
  RectNode *rectListP = NULL;
  // Get pixels of collision image
  Error e = readPng(&pngImgP, fp, &pixelSize, &pixelA, &colorPaletteA, verbose);
  if (!e) {
    // Background objects are pixel-by-pixel-(grid)-based, so they need to be compressed into strips.
    if (isBg) {
      if (!e) {  // Start doing brackets... Leaving them out hurts dev speed and reading comprehension.
        e = getColorPaletteAndColormap(&colorPaletteA, &colormapA, &nColors, pngImgP, pixelA, 16, pixelSize, verbose);
      }
      // Colormap StripSet  & StripMap
      if (!e) {
        e = stripNew(colormapA, verbose, bpp, arrayGetNElems(colormapA), &stripset, &stripmap);
      }
      // Write collision grid map source file.
      if (!e) {
        e = writeCollisionGridMap(entityName, &stripmap, &stripset, verbose);
      }
    }
    // Foreground objects are rectangle-based, so you only need to find the rectangle in each frame.
    else { 
      // Make sure all animation frames fit within image.
      U32 xMax, yMax;
      xMax = yMax = 0;
      for (FrameNode *fNodeP = animP->frameNodeA; fNodeP; fNodeP = fNodeP->nextP) {
        if ((fNodeP->x + fNodeP->w) > xMax) {
          xMax = fNodeP->x + fNodeP->w;
        }
        if ((fNodeP->y + fNodeP->h) > yMax) {
          yMax = fNodeP->y + fNodeP->h;
        }
      }
      if (xMax > pngImgP->width) {
        printf("Maximum x %d is bigger than collision image width %d.\n", xMax, pngImgP->width);
        return E_BAD_ARGS;
      }
      if (yMax > pngImgP->height) {
        printf("Maximum y %d is bigger than collision image height %d.\n", yMax, pngImgP->height);
        return E_BAD_ARGS;
      }

      e = findCollisionRects(pixelA, pngImgP, pixelSize, animP, &rectListP);
#if 0
      // Parse entity  name. 
      if (!e) {
        U32 startIdx, len;
        parseName(filepath, ".json", &startIdx, &len);
        char entityName[len + 1];
        memset(entityName, 0, sizeof(char) * (len + 1));
        memcpy((void*) entityName, (void*) &filepath[startIdx], len);
        if (verbose) {
          printf("Entity name is %s with length %d.\n", entityName, len);
        }
        memcpy(entityName, &filepath[startIdx], len);
        entityName[len] = '\0';
      }
#endif
      // Write collision rect map source file.
      if (!e) {
        e = writeCollisionRectMap(entityName, animP, rectListP);
      }
    }
  }
  stripDel(&stripset, &stripmap);
  arrayDel((void**) &colorPaletteA);
  arrayDel((void**) &pixelA);
  free(pngImgP);
  return e;
}


/* Three problems:
 *
 * \It's not writing any rects to output file
 * \It's writing one less frame. 
 * Y isn't right.
 * W is always -1
 * X is relative to image instead of frame
 * */
