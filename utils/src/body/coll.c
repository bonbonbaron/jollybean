#include "fileUtils.h"
#include "img.h"
#include "anim.h"
// Rect lists
typedef struct _RectNode {
  S32 x, y, w, h;
  struct _RectNode *nextP;
} RectNode;

static RectNode* rectNodeNew() {
  RectNode* rnP = jbAlloc(sizeof(RectNode), 1);
  rnP->x = -1;
  rnP->y = -1;
  rnP->w = -1;
  rnP->h = -1;
  rnP->nextP = NULL;
  return rnP;
}

static void rectNodeGrow(RectNode **nodePP) {
  assert(nodePP);
  if (!*nodePP) {
    rectNodeNew(nodePP);
  } else {
    rectNodeNew(&(*nodePP)->nextP);
    *nodePP = (*nodePP)->nextP;
  }
}

#if 0 // unused function
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
#endif

RectNode* getRectNode(RectNode *rootP, U32 idx) {
  if (!rootP) {
    return NULL;
  }
  RectNode *resultP = rootP;
  for (int i = 0; i < idx; ++i) {
    resultP = resultP->nextP;
  }
  return resultP;
}


// Pixel calculations
static U8* calcFirstPixelP(FrameNode *fNodeP, U8 *pixelA, U32 pixelSize, U32 imgPitch) {
  if (fNodeP) {
    U8 *ptr = pixelA 
      + (fNodeP->y * imgPitch)    // ... jumping to start of frame's last row
      + (fNodeP->x * pixelSize);  // starting from frame's first pixel's column...
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

static void findCollisionRects(Colormap *cmP, AnimJsonData *animP, RectNode **rectNodePP, U8 verbose) {
  assert (cmP && cmP->bpp <= 4 && rectNodePP);

  // U32 nPixels = cmP->w * cmP->h;
  U32 imgPitch = cmP->w * arrayGetElemSz(cmP->sdP->assembledDataA);

  // Keep track of the first node so we don't pass out the most recent one and lose all predecessors. 
  RectNode **currRectNodePP = rectNodePP; 
  RectNode *firstRectNodeP = NULL; 

  // Make your pappy proud and go find some rectangles, son.
  // Collision rect coords are offsets from current animation frame's top-left corner.
  /* if this is an animated collison rectangle... */ 
  if (animP) { 
    /* for each frame... */ 
    for (FrameNode *fNodeP = animP->frameNodeA; fNodeP; fNodeP = fNodeP->nextP) { 
      /* Grow a new frame node either from root or head. */ 
      if (!*currRectNodePP) { 
        rectNodeGrow(currRectNodePP); 
        firstRectNodeP = *currRectNodePP; 
      } 
      else { 
        rectNodeGrow(currRectNodePP); 
      } 
      /* If rect node creation succeeded... */ 
      RectNode *rectNodeP = *currRectNodePP; 
      U8 *pixelP = (U8*) calcFirstPixelP(fNodeP, cmP->sdP->assembledDataA, arrayGetElemSz(cmP->sdP->assembledDataA), imgPitch); 
      /* Number of pixels to jump from end of current frame's row to the start of its next row */
      U32 nPixelsToNextRow = (imgPitch / arrayGetElemSz(cmP->sdP->assembledDataA)) - fNodeP->w; 
      /* for each row in frame... */ 
      for (int i = 0; i < fNodeP->h; ++i, pixelP += nPixelsToNextRow) { 
        if (verbose) { 
          printf("%02d: ", i); 
        } 
        U8 *pixelRowEndP = (U8*) calcLastPixelPInRowP(fNodeP, (U8*) pixelP, arrayGetElemSz(cmP->sdP->assembledDataA)); 
        /* for each pixel in frame's current row... */ 
        for (int j = 0; pixelP < pixelRowEndP; ++j, ++pixelP) { 
          if (verbose) { 
            if (*pixelP) printf("1"); else printf("0"); 
          } 
          if (*pixelP) { 
            if (rectNodeP->x < 0) { 
              rectNodeP->x = j; 
            } 
            if (rectNodeP->y < 0) { 
              rectNodeP->y = i; 
            } 
            rectNodeP->h = i - rectNodeP->y + 1; 
          } 
          /* If you're at row's end and haven't terminated the width yet */ 
          if (j == fNodeP->w - 1 && rectNodeP->w < 0) { 
            rectNodeP->w = j + 1; 
          } 
          /* If you hit an empty pixel... */ 
          if (!pixelP) { 
            if ((j + 1) > rectNodeP->w) { 
              rectNodeP->w = j + 1; 
            } 
          } 
        }  /* for each pixel in frame's current row */ 
        if (verbose) { 
          printf("n");
        } 
      }  /* for each row in frame... */ 
      if (verbose) { 
        printf("rect xywh: %d %d %d %dnn", rectNodeP->x, rectNodeP->y, rectNodeP->w, rectNodeP->h); 
      } 
    }  /* for each frame... */ 
  }  /* if this is an animated collsion rectangle... */ 
  /* If this is not animated, you only need to find one rectangle. */ 
  else { 
    U8 *pixelP = (U8*) calcFirstPixelP(NULL, cmP->sdP->assembledDataA, arrayGetElemSz(cmP->sdP->assembledDataA), imgPitch); 
    U8 *lastPixelP = (U8*) calcLastPixelP(NULL, (U8*) pixelP, arrayGetElemSz(cmP->sdP->assembledDataA)); 
    rectNodeGrow(rectNodePP); 
    /* If rect node creation succeeded... */ 
    RectNode rect = **rectNodePP; 
    for (; pixelP < lastPixelP; ++pixelP) { 
      if (*pixelP) { 
        if (rect.x < 0 ) { 
          rect.x = (pixelP - cmP->sdP->assembledDataA) % imgPitch; 
        } 
        if (rect.y < 0) { 
          rect.y = (pixelP - cmP->sdP->assembledDataA) / imgPitch; 
        } 
      } 
      /* If you've hit an empty pixel and you've found the rect's start already... */ 
      else { 
        if (rect.x >= 0 && !rect.w) { 
          rect.w = ((pixelP - cmP->sdP->assembledDataA) % imgPitch) - rect.x; 
        } 
        if (rect.y >= 0) {  /* height needs to be continually updated till end of rect */ 
          rect.h = ((pixelP - cmP->sdP->assembledDataA) / imgPitch) - rect.y; 
        } 
      }  /* If you've hit an empty pixel and you've found the rect's start already... */ 
    } 
  } 

  *rectNodePP = firstRectNodeP;
}

// Write a collision grid to a C file.
void writeCollisionGridMap(char *entityName, Colormap *cmP, U8 verbose) {
  FILE *fP = getBuildFile("Seed/Genome/Gene/Body/Collision/Grid/src", entityName, "CollGrid.c", verbose);
  assert(fP);
  writeStripDataInFile(fP, verbose, entityName, cmP->sdP);
  if (fP) {
    fclose(fP);
  }
}

// Write a map of rectangle arrays to a C file.
void writeCollisionRectMap(char *entityName, AnimJsonData *animP, RectNode *collTreeP, U8 verbose) {
  // char *filepath = NULL;
  FILE *fP = getBuildFile("Seed/Genome/Gene/Body/Collision/Rect/src", entityName, "ColRect.c", verbose);
  assert(fP);

  U32 nKeyValPairs = 0;
  fprintf(fP, "#include \"xCollision.h\"\n\n");
  // Write collision rect arrays.
  for (TagNode *tagP = animP->tagNodeA; tagP != NULL; tagP = tagP->nextP) {
    // Collision rect arrays
    fprintf(fP, "CollRect *collRect_%s_%s_A[] = {\n", entityName, tagP->name);
    RectNode *rectNodeP = getRectNode(collTreeP, tagP->from);
    assert(rectNodeP);
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
    fprintf(fP, "\t.frameA = collRect_%s_%s_A\n", entityName, tagP->name);
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


// If animation is passed in, then we want to find the collision rect in each individual frame.
// 
// We consider each rectangle to have only one collision type, but the type may differ from 
// one animation frame to the next.
//
// If it's a background, we treat it as a pixel-perfect (AKA grid) collision object with 
// multiple collision types (where each pixel is a collision type value).
//
// For now at least, the input cannot be an animated background. (But what about water!? Hmmm...)
void coll(char *entityName, U8 isBg, AnimJsonData *animP, U8 verbose) {
  assert (entityName && !(isBg && animP));

  Colormap cm = {0};
  ColorPalette cp = {0};
  RectNode *rectListP = NULL;
  char *srcFilePath =  getSrcFilePath("Body/Graybody/Collision", entityName, "_col.png", verbose); 
  // Get pixels of collision image
  readPng(srcFilePath, &cm, &cp, animP, verbose);
  // Background objects are pixel-by-pixel-(grid)-based, so they need to be compressed into strips.
  if (isBg) {
    // Write collision grid map source file.
    writeCollisionGridMap(entityName, &cm, verbose);
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
    assert (xMax <= cm.w);
    assert (yMax <= cm.h);
    // Find collision rectangles in each animation frame.
    findCollisionRects(&cm, animP, &rectListP, verbose);
    // Write collision rect map source file.
    writeCollisionRectMap(entityName, animP, rectListP, verbose);
  }
  cmClr(&cm);
  arrayDel((void**) &cp.colorA);
  jbFree((void**) &srcFilePath);
}
