#include "strip.h"

// These three arrays describe where each consecutive unit
// is located in the current U32 word. We stagger them like this
// so we can inflate 4 units at a time in xRender.c.
const static U8 bitIdx1Bpu[] = {
  0, 8, 16, 24,
  1, 9, 17, 25,
  2, 10, 18, 26,
  3, 11, 19, 27,
  4, 12, 20, 28,
  5, 13, 21, 29,
  6, 14, 22, 30,
  7, 16, 23, 31
};

const static U8 bitIdx2Bpu[] = {
  0, 8, 16, 24,
  2, 10, 18, 26,
  4, 12, 20, 28,
  6, 14, 22, 30,
};

const static U8 bitIdx4Bpu[] = {
  0, 8, 16, 24,
  4, 12, 20, 28,
};

// Pack all the bits in strip set.
static Error _packBits(U8 **ssDataAP, U32 nStrips, U32 nUnits, U32 bpu, U8 verbose) {
  if (!ssDataAP || !*ssDataAP)
    return E_BAD_ARGS;

  if (verbose)
    printf("We're packing %d units into strips containing %d units per strip.\n", nUnits, N_UNITS_PER_STRIP);

  U8 *unitP = *ssDataAP;
  U8 *unitEndP = unitP + nUnits;
  U32 *bpsA = NULL;     // array of packed bits
  U32 *bpsP, *bpsEndP;  // bits packed staggered
  Error e = SUCCESS;

  switch (bpu) {
    case 1:
      e = arrayNew((void**) &bpsA, sizeof(U32), nUnits / (32) + (nUnits & 0x1f > 0));  // 32 units per U32 plus one more for any remainders
      if (!e) {
        bpsP = bpsA;
        bpsEndP = bpsP + arrayGetNElems(bpsA);
        for (; bpsP < bpsEndP && unitP < unitEndP; ++bpsP, unitP += 32) 
          for (U8 i = 0; i < 32; ++i) 
            *bpsP |= *(unitP + i) << bitIdx1Bpu[i];
        U8 nRemainingUnits = countRemainderUnits_(nUnits);
        for (U8 i = 0; i < nRemainingUnits; ++i)
          *bpsP |= *(unitP + i) << bitIdx1Bpu[i];
      }
      break;
    case 2:
      e = arrayNew((void**) &bpsA, sizeof(U32), nUnits / (16) + (nUnits & 0x0f > 0));  // 16 units per U32
      if (!e) {
        bpsP = bpsA;
        bpsEndP = bpsP + arrayGetNElems(bpsA);
        for (; bpsP < bpsEndP && unitP < unitEndP; ++bpsP, unitP += 32) 
          for (U8 i = 0; i < 16; ++i) 
            *bpsP |= *(unitP + i) << bitIdx2Bpu[i];
        U8 nRemainingUnits = countRemainderUnits_(nUnits);
        for (U8 i = 0; i < nRemainingUnits; ++i)
          *bpsP |= *(unitP + i) << bitIdx2Bpu[i];
      }
      break;
    case 4:
      e = arrayNew((void**) &bpsA, sizeof(U32), nUnits / (8) + (nUnits & 0x07 > 0));  // 8 units per U32
      if (!e) {
        bpsP = bpsA;
        bpsEndP = bpsP + arrayGetNElems(bpsA);
        for (; bpsP < bpsEndP && unitP < unitEndP; ++bpsP, unitP += 32) 
          for (U8 i = 0; i < 8; ++i) 
            *bpsP |= *(unitP + i) << bitIdx4Bpu[i];
        U8 nRemainingUnits = countRemainderUnits_(nUnits);
        for (U8 i = 0; i < nRemainingUnits; ++i)
          *bpsP |= *(unitP + i) << bitIdx4Bpu[i];
      }
      break;
    default:
      if (verbose)
        printf("No packing to be done.\n");
      return SUCCESS;  // Nothing's been allocated, so this is safe.
      break;
  }

  // Replace stripset with bit-packed one.
  if (!e) {
    if (verbose)
      printf("before packing data: %d bytes\n", arrayGetNElems(*ssDataAP) * arrayGetElemSz(*ssDataAP));
    arrayDel((void**) ssDataAP);
    *ssDataAP = (U8*) bpsA;
    bpsA = NULL;
    if (verbose)
      printf("after packing data: %d bytes\n", arrayGetNElems(*ssDataAP) * arrayGetElemSz(*ssDataAP));
  }
  else   // Clean up otherwise.
    arrayDel((void**) &bpsA);

  assert(bpsA == NULL && *ssDataAP != NULL);
  return e;
}

void stripDel(StripSetS *ssP, StripMapS *smP) {
  if (ssP) {
    if (ssP->stripSetInfP)
      inflatableDel(&ssP->stripSetInfP);
    arrayDel((void**) &ssP->flipSet.flipIdxA);
  }
  if (smP)
    if (smP->stripMapInfP)
      inflatableDel(&smP->stripMapInfP);
}

Error stripNew(U8 *srcA, U8 verbose, U8 bpu, U32 nUnits, StripSetS *ssP, StripMapS *smP) {
  if (!srcA || !ssP || !smP || !bpu || bpu > 8 || !nUnits)
    return E_BAD_ARGS;

  /* Number of strips needed:
   *              unit          strip      8 bits
   * X units    ----------  * -------  *  -----   = 0 strips
   *            1|2|4 bits   32 units     byte        
  */
  // From image's perspective, this is a 8bpp colormap at this point. Assume 8bit-per-unit to start with BEFORE packing.
  U32 nStrips = nUnits / N_UNITS_PER_STRIP + 1;  // maximum possible number of strips + 1 for remainders, asssuming 8bpp at first
  if (verbose)
    printf("starting strip set has %d units into a MAXIMUM of %d strips %d units each: %d bytes\n", nUnits, nStrips, N_UNITS_PER_STRIP, nStrips * N_UNITS_PER_STRIP);
  U16 stripLabel = 0;
  // Allocate all these annoying arrays.
  // nStrips is the max possible number of strips, so prepare to have up to that many.
  // Strips labelled so far
  StripmapIdx stripsLabelled[nStrips];
  memset(stripsLabelled, 0, sizeof(StripmapIdx) * nStrips);
  
  // Strip set
  U8 flipSet[nStrips];
  StripmapIdx flipIdxA[nStrips];
  U8 *stripsetDataA = NULL;
  StripmapIdx *stripMapA = NULL;

  Error e = arrayNew((void**) &stripsetDataA, sizeof(U8), nStrips * N_UNITS_PER_STRIP);
  if (verbose)
    printf("array is %d bytes.\n", nStrips * N_UNITS_PER_STRIP);
  if (!e) {
    memset(stripsetDataA, 0, sizeof(U8) * nStrips * N_UNITS_PER_STRIP);
    // Flip set 
    // This gets copied to a properly sized array at the end.
    memset(flipSet, 0, sizeof(U8) * nStrips);
    // Array of Index of strips to flip
    // This gets copied to a properly sized array at the end.
    memset(flipIdxA, 0, sizeof(U8) * nStrips);
    // Strip map
    e = arrayNew((void**) &stripMapA, sizeof(StripmapIdx), nStrips);
  }
  if (verbose) {
    printf("Analyzing viability of breaking image into %d strips of %d units each...\n", nStrips, N_UNITS_PER_STRIP);
  }

  // Find distinct strips and write an index-mapping to them.
  U8 *currStripP;
  U8 *nextStripP;  
  U32 nFlips = 0;
  if (!e) {
    for (U32 i = 0; i < nStrips; ++i) {
      // This strip hasn't been labelled yet. 
      // So give it its own label and add it to the set of unique strips.
      if (!stripsLabelled[i]) {
        memcpy((void*) &stripsetDataA[stripLabel], &srcA[i * N_UNITS_PER_STRIP], N_UNITS_PER_STRIP);
        stripsLabelled[i] = 1;  // If it's not labeled, you're labelling it now.
        stripMapA[i] = stripLabel++;
        /* All strips up to current "i" should be labelled, so search for strips
         * beyond that that match the current, newly labelled strip. */
        currStripP = srcA + (N_UNITS_PER_STRIP * i);
        for (StripmapIdx ii = i + 1; ii < nStrips; ++ii) {
          if (!stripsLabelled[ii]) {
            nextStripP = srcA + (ii * N_UNITS_PER_STRIP);
            if (!memcmp((const void*) nextStripP, 
                (const void*) currStripP, 
                sizeof(U8) * N_UNITS_PER_STRIP)) {
              stripMapA[ii] = stripLabel;  //flipped strip gets same label
              stripsLabelled[ii] = 1;
            }  // if strip is straight-up equal
            // Not identical as-is. Is it identical with flipping?
            else {
              for (StripmapIdx j = N_UNITS_PER_STRIP - 1; j >= 0; --j) {
                if (nextStripP[j] != currStripP[N_UNITS_PER_STRIP - j - 1])
                  goto flipNotEqual;
              }
              flipIdxA[nFlips++] = ii;
              stripMapA[ii] = stripLabel;
              stripsLabelled[ii] = 1;
              // Not identical at all
              flipNotEqual:
              continue;  // only because labels require something
            }  // else, see if flip is equal to strip
          }
        }
      }
    }
    if (verbose) {
      printf("%d distinct strips out of %d\n", stripLabel, nStrips);
    }
  }
  // Slight compression isn't worth the trouble to inflate; keep as is.
  if (!e) {
    if (nStrips - stripLabel < 5) { //arbitrary number; maybe decide more scientific number later
      memset(ssP, 0, sizeof(StripSetS));
    }
    else {
      // If it's worth doing, store compressed stuff into colormap profile.
      // Compress data into an inflatable.
      if (!e)
         e = inflatableNew((void*) stripMapA, &smP->stripMapInfP);
      if (!e)
        e = arrayNew((void**) &ssP->flipSet.flipIdxA, sizeof(StripmapIdx), nFlips);
      if (!e)
        e = _packBits(&stripsetDataA, stripLabel, nUnits, bpu, verbose);
      if (!e)
        e = inflatableNew((void*) stripsetDataA, &ssP->stripSetInfP);
      ssP->nUnits = nUnits;
      ssP->nUnitsPerStrip = N_UNITS_PER_STRIP;
      ssP->nStrips = stripLabel;
      if (e) {
        inflatableDel(&ssP->stripSetInfP);
        inflatableDel(&smP->stripMapInfP);
        arrayDel((void**) &ssP->flipSet.flipIdxA);
      }
    }
  }
  arrayDel((void**) &stripMapA);
  arrayDel((void**) &stripsetDataA);
  if (e) {
    printf("Something errored out in stripNew.\n");
  }
  return e;
}

Error writeStripDataInFile(FILE *fP, U8 verbose, char *objNameA, StripSetS *ssP, StripMapS *smP) {
  if (!fP || !ssP || !smP) {
    return E_BAD_ARGS;
  }

  char infName[strlen(objNameA) + strlen("StripSet") + strlen("Inf")];

  // Flip set
  fprintf(fP, "static U16 %sFlipIdxA[] = {\n", objNameA);
  writeRawData16(fP, (U16*) ssP->flipSet.flipIdxA, arrayGetNElems(ssP->flipSet.flipIdxA));
  fprintf(fP, "};\n\n");
  strcpy(infName, objNameA);
  strcat(infName, "StripSet");
  strcat(infName, "Inf");
  Error e = inflatableAppend(ssP->stripSetInfP, fP, infName);

  if (!e) {
    fprintf(fP, "\n\n");
    // Strip set 
    fprintf(fP, "StripSetS %sStripSet = {\n", objNameA);
    fprintf(fP, "\t.nUnitsPerStrip = %d,\n", N_UNITS_PER_STRIP);  // in case we ever want to give each sprite its own strip length... wuh oh.
    fprintf(fP, "\t.nStrips = %d,\n", ssP->nStrips);
    fprintf(fP, "\t.nUnits  = %d,\n", ssP->nUnits);
    fprintf(fP, "\t.flipSet = {\n", objNameA);
    fprintf(fP, "\t\t.nFlips = %d,\n", ssP->flipSet.nFlips);
    fprintf(fP, "\t\t.flipIdxA = %sFlipIdxA,\n", objNameA);
    fprintf(fP, "\t},\n");
    fprintf(fP, "\t.stripSetInfP = &%s,\n", infName);
    fprintf(fP, "};\n\n");
  }
  if (!e) {
    memset(infName, 0, strlen(infName));
    strcpy(infName, objNameA);
    strcat(infName, "StripMap");
    strcat(infName, "Inf");
    e = inflatableAppend(smP->stripMapInfP, fP, infName);
  }
  if (!e) {
    fprintf(fP, "\n\n");
    fprintf(fP, "StripMapS %sStripMap = {\n", objNameA);
    fprintf(fP, "\t.nIndices = %d,\n", smP->nIndices);
    fprintf(fP, "\t.stripMapInfP = &%s\n", infName);
    fprintf(fP, "};\n\n");
  }

  return e;
}

Error writeStripData(char *objNameA, char *OBJ_TYPE, U8 verbose, StripSetS *ssP, StripMapS *smP) {
  Error e = SUCCESS;
  // Make target filepath to save this stripmap in.
  char *HOME = getenv("HOME");
  if (!HOME) {
    if (verbose)
      printf("no $HOME environment variable! exiting...\n");
    return E_BAD_ARGS;
  }
  char TROVE_STRIP_DIR[] = "/jb/build/";
  char fullPath[strlen(HOME) + strlen(TROVE_STRIP_DIR) + strlen(OBJ_TYPE) + strlen("/") + strlen(objNameA) + strlen(".c")];
  strcpy(fullPath, HOME);
  strcat(fullPath, TROVE_STRIP_DIR);
  strcat(fullPath, OBJ_TYPE);
  strcat(fullPath, "/");
  strcat(fullPath, objNameA);
  strcat(fullPath, ".c");
  // Open file.
  FILE *fP = fopen(fullPath, "w");
  if (!fP) {
    if (verbose)
      printf("[writeStripData] file opening failed for path %s\n", fullPath);
    e = E_BAD_ARGS;
  }
  if (!e) {
    e = writeStripDataInFile(fP, verbose, objNameA, ssP, smP);
  }

  return e;
}
