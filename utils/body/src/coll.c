#include "fileUtils.h"
#include "img.h"
#include "anim.h"

/* I like the binary file-based directory idea, because 
 * this way you don't have to recompile this tool before
 * using it again. */

Error writeCollisionTree(char *entityName, CollisionTree *collTreeP) {
  char *filepath = NULL;
  getSrcFilePath(&fp, 
  FILE *fP = fopen(fp, "w");
  if (!fP) {
    printf("failed to open %s\n", fp);
    return E_NO_MEMORY;
  }
  fprintf(fP, "#include \"xAnim.h\"\n\n");
  // Write frame arrays.
  for (TagNode *tagP = collTreeP->tagNodeA; tagP != NULL; tagP = tagP->nextP) {
    // CollisionTree frame arrays
    fprintf(fP, "AnimFrame *frame_%s_%sA[] = {\n", entityName, tagP->name);
    FrameNode *frameNodeP = getFrameNode(collTreeP->frameNodeA, tagP->from);
    if (!frameNodeP)
      return E_BAD_ARGS;
    for (int i = tagP->from; i < tagP->to; ++i) {
      fprintf(fP, "\t{\n");
      fprintf(fP, "\t\t.x = %d,\n", frameNodeP->x);
      fprintf(fP, "\t\t.y = %d,\n", frameNodeP->y);
      fprintf(fP, "\t\t.w = %d,\n", frameNodeP->w);
      fprintf(fP, "\t\t.h = %d,\n", frameNodeP->h);
      fprintf(fP, "\t\t.duration = %d\n", frameNodeP->duration);
      fprintf(fP, "\t},\n");
    }
    fprintf(fP, "};\n\n");
    // CollisionTree strips
    fprintf(fP, "AnimStrip strip_%s_%s = {\n", entityName, tagP->name);
    fprintf(fP, "\t.nFrames = %d,\n", 1 + tagP->to - tagP->from);
    int len = strlen(tagP->name);
    Bln isLeft = !strncasecmp(&tagP->name[len - 4], "LEFT", 4);
    fprintf(fP, "\t.flip = %d,\n", isLeft);
    // Aseprite left out looping in their tags for some reason, so using the otherwise useless reverse!
    Bln isReverse = !strncasecmp(tagP->direction, "reverse", 4);
    fprintf(fP, "\t.repeat = %d,\n", isReverse);  
    Bln isPingPong = !strncasecmp(tagP->direction, "pingpong", 4);
    fprintf(fP, "\t.pingPong = %d,\n", isPingPong);
    fprintf(fP, "\t.frameA = frame_%s_%sA\n", entityName, tagP->name);
    fprintf(fP, "};\n\n");
  }
  fclose(fP);
}

static U8* calcFirstPixelP(FrameNode *fNodeP, U8 *pixelA, U32 pixelSize, U32 imgPitch) {
  if (fNodeP) {
    return pixelA 
      + (fNodeP->x * pixelSize)  // starting from frame's first pixel's column...
      + (fNodeP->y * imgPitch);  // ... jumping to start of frame's last row
  }
  else {
    return pixelA;
  }
}

static U8* calcLastPixelP(FrameNode *fNodeP, U8 *firstPixelP, U32 pixelSize, U32 imgPitch) {
  if (fNodeP) {
    return firstPixelP + 
      + (fNodeP->w * pixelSize)  // starting from frame's first pixel's column...
      + (fNodeP->h * imgPitch);  // ... jumping to start of frame's last row
  }
  else {
    // assuming firstPixel is the start of the array, as per calcFirstPixel()
    return firstPixelP + (arrayGetNElems(firstPixelP) * arrayGetElemSz(firstPixelP));
  }
}

#define findRect_(bpp_) \
  /* if this is an animated collison rectangle... */ \
  if (animP) { \
    /* for each frame... */ \
    for (FrameNode *fNodeP = animP->frameNodeA; fNodeP; fNodeP = fNodeP->nextP) { \
      U##bpp_ *pixelP = calcFirstPixelP(fNodeP, pixelA, pixelSize, imgPitch); \
      /* for each row in frame... */ \
      for (int i = 0; i < fNodeP->h; ++i, pixelP += imgPitch) { \
        U##bpp_ *pixelEndP = calcLastPixelP(fNodeP, pixelP, pixelSize, imgPitch); \
        /* for each pixel in frame's current row... */ \
        for (int j = 0; pixelP < pixelEndP; ++j, ++pixelP) { \
          if (*pixelP) { \
            if (rectP->x < 0) { \
              rectP->x = fNodeP->x + j; \
            } \
            if (rectP->y < 0) { \
              rectP->y = fNodeP->y + i; \
            } \
          } \
          /* If you've hit an empty pixel and you've found the rect's start already... */ \
          else { \
            if (rectP->x >= 0 && !rectP->w) { \
              rectP->w = j - rectP->x; \
            } \
            if (rectP->y >= 0) {  /* height needs to be continually updated till end of rect */ \
              rectP->h = i - rectP->y; \
            } \
          }  /* If you've hit an empty pixel and you've found the rect's start already... */ \
        }  /* for each pixel in frame's current row */ \
      }  /* for each row in frame... */ \
    }  /* for each frame... */ \
  }  /* if this is an animated collsion rectangle... */ \
  /* If this is not animated, you only need to find one rectangle. */ \
  else { \
    U##bpp_ *pixelP = calcFirstPixel(NULL, pixelA, pixelSize, imgPitch); \
    U##bpp_ *lastPixelP = calcLastPixel(NULL, pixelP, pixelSize, imgPitch); \
    for (; pixelP < lastPixelP; ++pixelP) { \
      if (*pixelP) { \
        if (rectP->x < 0 ) { \
          rectP->x = (pixelP - pixelA) % imgPitch; \
        } \
        if (rectP->y < 0) { \
          rectP->y = (pixelP - pixelA) / imgPitch; \
        } \
      } \
      /* If you've hit an empty pixel and you've found the rect's start already... */ \
      else { \
        if (rectP->x >= 0 && !rectP->w) { \
          rectP->w = ((pixelP - pixelA) % imgPitch) - rectP->x; \
        } \
        if (rectP->y >= 0) {  /* height needs to be continually updated till end of rect */ \
          rectP->h = ((pixelP - pixelA) / imgPitch) - rectP->y; \
        } \
      }  /* If you've hit an empty pixel and you've found the rect's start already... */ \
    } \
  } 

Error findCollisionRects(U8 *pixelA, png_image *imgP, U32 pixelSize, Animation *animP, Rect_ *rectP) {
  if (!pixelA || !imgP || !pixelSize || pixelSize > 4) {
    return E_BAD_ARGS;
  }
  U32 nPixels = imgP->width * imgP->height;
  U32 imgPitch = imgP->width * pixelSize;

  // Initialize rectangle to negative numbers to make parameter-tracking easier.
  rectP->x = rectP->y = rectP->w = rectP->h = -1;

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

  return SUCCESS;
}

// If animation is passed in, then we want to find the collision rect in each individual frame.
// 
// We consider each rectangle to have only one collision type, but the type may differ from 
// one rectangle to the next.
//
// If it's a background, we treat it as a pixel-perfect (AKA grid) collision object with 
// multiple collision types.
//
// For now at least, the input cannot be both animation and background. 
Error coll(char *fp, U8 isBg, Animation *animP, U8 verbose) {
  if (!dirp || (isBg && animP)) {
    return E_BAD_ARGS;
  }
  png_image *pngImgP = NULL;
  U32 pixelSize = 0;
  U8 *pixelA = NULL;
  U8 *colorPaletteA = NULL;
  // Get pixels of collision image
  Error e = readPng(&pngImgP, fp, &pixelSize, &pixelA, &colorPaletteA);
  if (!e) {
    // Background objects are pixel-by-pixel-(grid)-based, so they need to be compressed into strips.
    if (isBg) {
      if (!e) {  // Start doing brackets... Leaving them out hurts dev speed and reading comprehension.
        e = getColorPaletteAndColormap(&colorPaletteA, &colormapA, &nColors, pngImgP, pixelP, 16, srcPixelSize);
      }
      arrayDel((void**) &stripset.dataA);
    }
    // Foreground objects are rectangle-based, so you only need to find the rectangle in each frame.
    else { 
    }
  }
  arrayDel((void**) &colorPaletteA);
  arrayDel((void**) &pixelA);
  free(pngImgP);
  return e;
}

// If there's no json file, the collision BB should be based on the image itself. 
