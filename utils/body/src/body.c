#include "body.h"

/*
 *
 * Should I pass "anything" into body?
 * It would be fastest group-wise to keep files open while going through them,
 * but it would be fastest compile-time to only access unbuilt files.
 *
 * So make two modes:
 *  If argc == 1, do them all.
 *  Otherwise, only process the arguments.
 *
 * Now what should those arguments be?
 * Remember, pipes work from the front backwards. 
 * Since I'm grouping palettes package-wide (so anybody can use them),
 * I'm storing bodies as <[collision] + colormap + [animation]> ([...] = "optional").
 * Collision palette-- perhaps only one-- exists in its own folder.
 *
 * I'm not grouping them together in the source folders; they're raw materials there.
 * I'm grouping them together in the genome db.
 * 
 */

int main(int argc, char ** argv) {
  const char CP_DIR_NAME[] = "ColorPalette";
  const char CM_DIR_NAME[] = "ColorMap";
  const char BODY_DIR_NAME[] = "Graybody";

  Error e = getSrcFilePath(


  Directory *cpDirP, *cmDirP;
  U32 origNColorPalettes, origNCollisionTypes, origNColormaps;
  U8 verbose = 0;

  if (!memcmp(argv[1], "-v", strlen(argv[1]))) {
    verbose = 1;
  }

  // Get old palette directory.
  Error e = dirGet(&cpDirP, CP_DIR_NAME, argc, verbose);
  if (!e) {
    origNSavedPalettes = cpDirP->nEntries;
  }

  // Write all the bodies.
  if (!e) {
    cpDirP = cmDirP = NULL;
    origNPalettes = origNColormaps = 0;
    FILE *fP = NULL;
  
    for (U32 i = 1; i < argc; ++i) {
      char currFilepath[strlen(argv[i]) + strlen(
      fP = openFile(
    }
  }

  // Save the updated palette directory AFTER all updates are done.
  if (!e && cpDirP->nEntries != origNSavedPalettes) {
    e = dirReplaceOriginal(cpDirP, CP_DIR_NAME, verbose);
  }
  else if (verbose) {
    printf("\nNo new palettes to write.\n");
  }

  return e;
}
