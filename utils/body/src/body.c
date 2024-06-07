#include "body.h"

/* 
 * Passed-in arguments: name of bod[y|ies].
 */
int main(int argc, char ** argv) {
  AnimJsonData *animP = NULL;

  // Verbose 
  U8 verbose = 0;

  // Write all the bodies.
  for (U32 i = 1; !e && i < argc; ++i) {
    if (!strcmp(argv[i], "-v")) {
      verbose = 1;
      continue;
    }
    // Animation
    e = anim(argv[i], verbose, &animP);
    // Colormap and Color palette (autogenerated)
    if (!e) {
      e = img(argv[i], animP, verbose);
    }
    goto skipColl;
    // Collision
    if (!e) {
      e = coll(argv[i], 0, animP, verbose);
    }
skipColl:
    jbFree((void**) &animP);
    // Genome TODO
    // Seed TODO
  }

  return e;
}
