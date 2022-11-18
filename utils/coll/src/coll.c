#include "img.h"
#include "anim.h"

/* I like the binary file-based directory idea, because 
 * this way you don't have to recompile this tool before
 * using it again. */

static ColorCollisionDirectory *newDirectoryP = NULL;  // is used to replace above binary file

Error createExistingDirectory(char *filepath) {
  const static ColorCollisionDirectory palDirPrototype = {0};
  FILE *fP = fopen(filepath, "wb");
  if (fP) {
    fwrite(&palDirPrototype, sizeof(ColorCollisionDirectory), 1, fP);
    fclose(fP);
  }
  else { 
    if (verbose)
      printf("Failed to create missing file, %s\n", filepath);
    return E_BAD_ARGS;
  }
  return SUCCESS;
}

Error getExistingDirectory(int argc) {
  char *HOME = getenv("HOME");
  char filepath[strlen(HOME) + strlen(LOCAL_TROVE_BOOKKEEPING_DIR) + strlen(COLORCOLLMAPPING_FILENAME)];
  strcpy(filepath, HOME);
  strcat(filepath, LOCAL_TROVE_BOOKKEEPING_DIR);
  strcat(filepath, COLORCOLLMAPPING_FILENAME);

  Error e = SUCCESS;

  // Read just top-level object 
  FILE *fP = fopen(filepath, "rb");
  if (!fP) {
    fclose(fP);
    e = createExistingDirectory(filepath);
    if (!e) {
      fP = fopen(filepath, "rb");  // Should open if we successfully created it.
      if (!fP)
        e = E_BAD_ARGS;
    }
  }

  U32 nBytesTotal = 0;
  if (!e) {
    fseek(fP, 0, SEEK_END);
    nBytesTotal = ftell(fP);
    e = jbAlloc((void**) &newDirectoryP, 2 *(nBytesTotal + (argc * sizeof(SavedColorCollisionMapping))), 1);  // "* 2" just for safety
  }
  if (!e) {
    fseek(fP, 0, SEEK_SET);
    U32 nBytesRead = fread(newDirectoryP, 1, nBytesTotal, fP);
    if (verbose)
      printf("read %d bytes out of %d expected\n", nBytesRead, nBytesTotal);
    assert(nBytesRead == nBytesTotal);
    if (verbose)
      printf("Existing directory contains %d entries and %d bytes.\n", newDirectoryP->nSavedColorCollisionMappings, nBytesTotal);
  
    fclose(fP);
  }

  return e;
}

Error replaceExistingDirectory() {
  char *HOME = getenv("HOME");
  char filepath[strlen(HOME) + strlen(LOCAL_TROVE_BOOKKEEPING_DIR) + strlen(COLORCOLLMAPPING_FILENAME)];
  strcpy(filepath, HOME);
  strcat(filepath, LOCAL_TROVE_BOOKKEEPING_DIR);
  strcat(filepath, COLORCOLLMAPPING_FILENAME);

  Error e = SUCCESS;

  // Open the file.
  FILE *fP = fopen(filepath, "wb");
  if (!fP) {
    fclose(fP);
    return E_BAD_ARGS;
  }

  // Write to it.
  U32 nBytesToWrite = sizeof(ColorCollisionDirectory) + (newDirectoryP->nSavedColorCollisionMappings * sizeof(SavedColorCollisionMapping));
  U32 nBytesWritten = fwrite(newDirectoryP, 1, nBytesToWrite, fP);
  if (verbose) {
    printf("\n\n%d actual of %d expected bytes written to directory file:.\n", nBytesWritten, nBytesToWrite);
    printf("\tPalette directory: %d bytes\n", sizeof(ColorCollisionDirectory));
    printf("\t%d saved palettes (%d bytes each): %d bytes\n", newDirectoryP->nSavedColorCollisionMappings, sizeof(SavedColorCollisionMapping), nBytesToWrite);
    printf("Output filepath:\n");
    printf("\t%s\n", filepath);
  }

  // Close it.
  fclose(fP);

  return e;
}

void appendColorCollisionDirectory(char *name, U32 *colorA, U32 nColors) {
  if (verbose)
    printf("adding %s to directory\n", name);

  SavedColorCollisionMapping newPalette = {
    .nColors = nColors,
    .paletteName = "abc",
    .paletteA = {0}
  };
  memcpy(&newPalette.paletteA, colorA, nColors * sizeof(U32));
  strcpy(newPalette.paletteName, name);

  newDirectoryP->savedPalettesA[newDirectoryP->nSavedColorCollisionMappings++] = newPalette;
}

char* queryDirectoryForPaletteName(U32 *colorA, U32 nColors) {
  for (int i = 0; i < newDirectoryP->nSavedColorCollisionMappings; ++i)
    if (newDirectoryP->savedPalettesA[i].nColors == nColors)
      if (!memcmp(newDirectoryP->savedPalettesA[i].paletteA, colorA, sizeof(U32) * nColors))
        return newDirectoryP->savedPalettesA[i].paletteName;
  return NULL;
}

static U8 IS_GRID = false;

Error writeCollisionTree(char *entityName, CollisionTree *collTreeP) {
  char fp[200] = {0};
  strcpy(fp, TROVE_ANIM_DIR);
  strcat(fp, entityName);
  strcat(fp, ".c");
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

int main (int argc, char **argv) {
  Error e = SUCCESS;
  if (argc > 1) {  // argv[0] is this program's name
    // iterate through arguments
    for (int i = 1; i < argc; ++i) {
      if (!strcmp(argv[i], "-v")) {
        verbose = 1;
        continue;
      }
      // allow for interleaving foregound files with background files
      else if (!strcmp(argv[i], "-b")) {
        IS_GRID = 1;
        continue;
      }
      else if (!strcmp(argv[i], "-f"))  {  
        IS_GRID = 0;
        continue;
      }
      // Process the current file.
      else {
        png_image *pngImgP = NULL;
        U32 pixelSize = 0;
        U8 *pixelA = NULL;
        U8 *colorPaletteA = NULL;
        e = readPng(&pngImgP, argv[i], &pixelSize, &pixelA, &colorPaletteA);
        // Background objects are pixel-by-pixel-(grid)-based, so they need to be compressed into strips.
        if (IS_GRID) {
          if (!e) {  // Start doing brackets... Leaving them out hurts dev speed and reading comprehension.
            e = getColorPaletteAndColormap(&colorPaletteA, &colormapA, &nColors, pngImgP, pixelP, 16, srcPixelSize);
          }

          arrayDel((void**) &stripset.dataA);
        }
        // Foreground objects are rectangle-based, so you only need to find the rectangle in each frame.
        else { 
        }
        arrayDel((void**) &colorPaletteA);
        arrayDel((void**) &pixelA);
        free(pngImgP);
      }
    }
  }
  return e;
}

// If there's no json file, the collision BB should be based on the image itself. 
