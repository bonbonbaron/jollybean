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

  U8 *bitIdxA;
  switch (bpu) {
    case 1:
      bitIdxA = (U8*) bitIdx1Bpu;
      break;
    case 2:
      bitIdxA = (U8*) bitIdx2Bpu;
      break;
    case 4:
      bitIdxA = (U8*) bitIdx4Bpu;
      break;
    default:
      printf("[_packBits] no packing to be done.\n");
      return SUCCESS;;
  }
  // TODO; where i left off: I'm putting in the STRIP data, not the full colormap. Doh.
  //       So I need to evaluate the stripmap's accuracy first.
  //       Then I need to directly unpack one strip at a time to see it equals original unpacked.
  //       If that looks good, I need to go ahead and see the full unpacker match the original.
  //       If that works, then I need to validate the stripmap generator. I'll figure out how later.
  if (verbose) {
    printf("about to pack bits...\n");
  }
  Error e = arrayNew((void**) &bpsA, sizeof(U32), (nUnits / (8 / bpu)) + ((nUnits & 0x1f) > 0));  // 8 units per U32
  if (!e) {
    U32 nUnitsPerWord = 32 / bpu;
    bpsP = bpsA;
    bpsEndP = bpsP + arrayGetNElems(bpsA);
    // For each output word....
    for (; bpsP < bpsEndP && unitP < unitEndP; ++bpsP) {
      // For each unit you can pack into that output word...
      for (U8 i = 0; i < nUnitsPerWord; ++i) {
        printf("%d", *unitP);
        if (1 || *unitP) {
          //printf("\npacking 0x%08x into 0x%08x--> ", (*unitP << bitIdxA[i]), *bpsP);
        }
        // Pack unit into output word.
        *bpsP |= *(unitP++) << bitIdxA[i];
        if (1 || *unitP) {
          //printf("0x%08x", *bpsP);
        }
      }
    }
    U8 nRemainingUnits = countRemainderUnits_(nUnits);  // packs 4-bit values into a word
    for (U8 i = 0; bpsP < bpsEndP && i < nRemainingUnits; ++i, ++bpsP) {
      *bpsP |= *(unitP + i) << bitIdxA[i];  // packs 4-bit values into a word
      //printf("%d", (*bpsP & (0x0f << (i * 4))) >> (i * 4));
    }
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

#define SHOW_OUTPUT_BYTES 1
//##########################################
// Debugging tools to validate input integrity (independent of xRender's inflation tools)
inline static void _unpackWholeStrip(U32 **dstWordPP, U32 **srcStripPP, const U32 mask, const U8 bpu, const U32 nWordsPerPackedStrip) {
  U32 *srcStripP = *srcStripPP;
  U32 *dstWordP = *dstWordPP;
  /* Although the first loop line appears unnecessary for 1 word per 1Bpu strip,
     it safeguards us from changes in the number of units per strip. */
  for (int i = 0; i < nWordsPerPackedStrip; ++i, ++srcStripP) {
    for (int j = 0; j < N_BITS_PER_BYTE; j += bpu) {
      *dstWordP =  (*srcStripP >> j) & mask;
      ++dstWordP;
#if SHOW_OUTPUT_BYTES
      printf("%d%d%d%d", 
            ((U8*) (dstWordP - 1))[0],
            ((U8*) (dstWordP - 1))[1],
            ((U8*) (dstWordP - 1))[2],
            ((U8*) (dstWordP - 1))[3]);
#endif
    }
  }
  *dstWordPP = dstWordP;  // update original pointer to its new location
}

static inline U32 countWholeBytesOfUnits(U32 nUnits_, U32 bpu) {
 return (nUnits_ / (8 / bpu));
}

__inline__ static void _unpackRemainderUnits(U8 *byteA, U8 *dstByteP, const U32 mask, const U8 bpu, U32 nRemainderUnits) {
  U8 *byteP = byteA;
  U8 *byteEndP = byteP + countWholeBytesOfUnits(nRemainderUnits, bpu);
  /* Handle all the whole bytes of units. */
  //printf("# whole bytes of remainder data: %d\n", byteEndP - byteP);
  while (byteP < byteEndP) {
    for (U8 i = 0; i < N_BITS_PER_BYTE; i += bpu) {
      *(dstByteP++) =  (*(byteP++) >> i) & mask;
#if SHOW_OUTPUT_BYTES
      printf("%d", *(dstByteP - 1));
#endif
    }
  }
  /* Handle the last, partial byte of data. */
  U8 iEnd = countUnitsInPartialByte4BPU_(nRemainderUnits);
  //printf("# partial bytes of remainder data: %d\n", iEnd);
  for (U8 i = 0; i < iEnd; i += bpu) {
    *(dstByteP++) =  (*(byteP++) >> i) & mask;
#if SHOW_OUTPUT_BYTES
    printf("%d", *(dstByteP - 1));
#endif
  }
}


// Unpack bits to reconstruct original data
Error unpackStripset(StripsetS *ssP, StripmapS *smP, U8 **outputPP) {
  if (!ssP || !smP || !outputPP || (outputPP && *outputPP != NULL)) {
    return E_BAD_ARGS;
  }

  Error e = arrayNew((void**) outputPP, sizeof(U8), ssP->nUnits);
  if (!e) {
    printf("giving new dataP %d bytes\n", ssP->nUnits);
  }

  if (!e) {
    const U32 nWordsPerPackedStrip = (N_UNITS_PER_STRIP * ssP->bpu) / N_BITS_PER_WORD;
    printf("bpu: %d\n", ssP->bpu);
    printf("nWordsPerPackedStrip: %d\n", nWordsPerPackedStrip);
    const U32 nWholeStrips = countWholeStrips_(ssP->nUnits); 
    assert(nWholeStrips == 217);
    const U32 nRemainderUnits = countRemainderUnits_(ssP->nUnits); 
    assert(nRemainderUnits == 16);

    U32 mask = 0;

    switch(ssP->bpu) {
      case 1:
        mask = 0x01010101;
        break;
      case 2:
        mask = 0x03030303;
        break;
      case 4:
        mask = 0x0f0f0f0f;
        break;
      default:
        mask = 0xffffffff;
        break;
    }

    U32 stripIdx = 0;
    U32 *srcStripP = NULL;
    U32 *dstP = (U32*) *outputPP;
    U32 *dstOrigP = dstP;  // restore destination pointer to its original location afterward.

    // Unpack whole strips
    for (U16 *smElemP = smP->infP->inflatedDataP;
         smElemP < (U16*) smP->infP->inflatedDataP + nWholeStrips; 
         ++smElemP) {
      stripIdx = *smElemP * nWordsPerPackedStrip;
      srcStripP = &((U32*) ssP->infP->inflatedDataP)[stripIdx];
      _unpackWholeStrip(&dstP, &srcStripP, (const U32) mask, ssP->bpu, nWordsPerPackedStrip);
    }
    // Unpack remainder units
    _unpackRemainderUnits((U8*) srcStripP, (U8*) dstP, mask, ssP->bpu, nRemainderUnits);
    /* Flip whatever strips need flipping. Remember data's already expanded to U8s! */
    if (ssP->flipset.nFlips) {
      flipUnpackedStrips(ssP, dstOrigP);  // go ahead and rely on data.c here
    }

    *outputPP = (U8*) dstOrigP;
  }

  return e;
}

void stripDel(StripsetS *ssP, StripmapS *smP) {
  if (ssP) {
    if (ssP->infP)
      inflatableDel(&ssP->infP);
    arrayDel((void**) &ssP->flipset.flipIdxA);
  }
  if (smP)
    if (smP->infP)
      inflatableDel(&smP->infP);
}

Error stripNew(U8 *srcA, U8 verbose, U8 bpu, U32 nUnits, StripsetS **ssPP, StripmapS **smPP) {
  if (!srcA || !ssPP || !smPP || !bpu || bpu > 8 || !nUnits)
    return E_BAD_ARGS;

  /* Number of strips needed:
   *              unit          strip      8 bits
   * X units    ----------  * -------  *  -----   = 0 strips
   *            1|2|4 bits   32 units     byte        
  */
  // From image's perspective, this is a 8bpp colormap at this point. Assume 8bit-per-unit to start with BEFORE packing.
  U32 nStrips = nUnits / N_UNITS_PER_STRIP + 1;  // maximum possible number of strips + 1 for remainders, asssuming 8bpp at first
  if (verbose) {
    printf("starting strip set has %d units into a MAXIMUM of %d strips %d units each: %d bytes\n", nUnits, nStrips, N_UNITS_PER_STRIP, nStrips * N_UNITS_PER_STRIP);
  }
  U16 stripLabel = 0;
  // Allocate all these annoying arrays.
  // nStrips is the max possible number of strips, so prepare to have up to that many.
  // Strips labelled so far
  StripmapIdx stripsLabelled[nStrips];
  memset(stripsLabelled, 0, sizeof(StripmapIdx) * nStrips);
  // Strip set
  U8 flipset[nStrips];
  StripmapIdx flipIdxA[nStrips];
  U8 *stripsetDataA = NULL;  // stripset placeholder till we compress it into output's inflatable
  StripmapIdx *stripmapDataA = NULL;  // ditto

  // Allocate output stripmap and stripset
  *ssPP = NULL;
  *smPP = NULL;
  Error e = jbAlloc((void**) ssPP, sizeof(StripsetS), 1);
  if (!e) {
    e = jbAlloc((void**) smPP, sizeof(StripmapS), 1);
  }

  if (!e ) {
    e = arrayNew((void**) &stripsetDataA, sizeof(U8), nStrips * N_UNITS_PER_STRIP);
  }
  if (verbose)
    printf("array is %d bytes.\n", nStrips * N_UNITS_PER_STRIP);
  if (!e) {
    memset(stripsetDataA, 0, sizeof(U8) * nStrips * N_UNITS_PER_STRIP);
    // Flip set 
    // This gets copied to a properly sized array at the end.
    memset(flipset, 0, sizeof(U8) * nStrips);
    // Array of Index of strips to flip
    // This gets copied to a properly sized array at the end.
    memset(flipIdxA, 0, sizeof(U8) * nStrips);
    // Strip map
    e = arrayNew((void**) &stripmapDataA, sizeof(StripmapIdx), nStrips);
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
        stripmapDataA[i] = stripLabel++;
        /* All strips up to current "i" should be labelled, so search for strips
         * beyond that that match the current, newly labelled strip. */
        currStripP = srcA + (N_UNITS_PER_STRIP * i);
        for (StripmapIdx ii = i + 1; ii < nStrips; ++ii) {
          if (!stripsLabelled[ii]) {
            nextStripP = srcA + (ii * N_UNITS_PER_STRIP);
            if (!memcmp((const void*) nextStripP, 
                (const void*) currStripP, 
                sizeof(U8) * N_UNITS_PER_STRIP)) {
              stripmapDataA[ii] = stripLabel;  //flipped strip gets same label
              stripsLabelled[ii] = 1;
            }  // if strip is straight-up equal
            // Not identical as-is. Is it identical with flipping?
            else {
              for (StripmapIdx j = N_UNITS_PER_STRIP - 1; j >= 0; --j) {
                if (nextStripP[j] != currStripP[N_UNITS_PER_STRIP - j - 1])
                  goto flipNotEqual;
              }
              flipIdxA[nFlips++] = ii;
              stripmapDataA[ii] = stripLabel;
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
      memset(*ssPP, 0, sizeof(StripsetS));
    }
    else {
      // If it's worth doing, store compressed stuff into colormap profile.
      // Compress data into an inflatable.
      if (!e) {
         e = inflatableNew((void*) stripmapDataA, &(*smPP)->infP);
      }
      if (!e) {
        e = arrayNew((void**) &(*ssPP)->flipset.flipIdxA, sizeof(StripmapIdx), nFlips);
      }
      if (!e) {
        e = _packBits(&stripsetDataA, stripLabel, nUnits, bpu, verbose);
      }
      if (!e) {
        e = inflatableNew((void*) stripsetDataA, &(*ssPP)->infP);
      }
      (*ssPP)->nUnits = nUnits;
      (*ssPP)->nUnitsPerStrip = N_UNITS_PER_STRIP;
      (*ssPP)->nStrips = stripLabel;
      (*ssPP)->bpu = bpu;
      if (e) {
        inflatableDel(&(*ssPP)->infP);
        inflatableDel(&(*smPP)->infP);
        arrayDel((void**) &(*ssPP)->flipset.flipIdxA);
      }
    }
  }
  arrayDel((void**) &stripmapDataA);
  arrayDel((void**) &stripsetDataA);
  if (e) {
    jbFree((void**) ssPP);
    jbFree((void**) smPP);
    printf("Something errored out in stripNew.\n");
  }
  return e;
}

Error writeStripDataInFile(FILE *fP, U8 verbose, char *objNameA, StripsetS *ssP, StripmapS *smP) {
  if (!fP || !ssP || !smP) {
    return E_BAD_ARGS;
  }

  char infName[strlen(objNameA) + strlen("Stripset") + strlen("Inf")];

  // Flip set
  fprintf(fP, "static U16 %sFlipIdxA[] = {\n", objNameA);
  writeRawData16(fP, (U16*) ssP->flipset.flipIdxA, arrayGetNElems(ssP->flipset.flipIdxA));
  fprintf(fP, "};\n\n");
  strcpy(infName, objNameA);
  strcat(infName, "Stripset");
  strcat(infName, "Inf");
  Error e = inflatableAppend(ssP->infP, fP, infName);

  if (!e) {
    fprintf(fP, "\n\n");
    // Strip set 
    fprintf(fP, "StripsetS %sStripset = {\n", objNameA);
    fprintf(fP, "\t.nUnitsPerStrip = %d,\n", N_UNITS_PER_STRIP);  // in case we ever want to give each sprite its own strip length... wuh oh.
    fprintf(fP, "\t.nStrips = %d,\n", ssP->nStrips);
    fprintf(fP, "\t.nUnits  = %d,\n", ssP->nUnits);
    fprintf(fP, "\t.bpu  = %d,\n", ssP->bpu);
    fprintf(fP, "\t.flipset = {\n", objNameA);
    fprintf(fP, "\t\t.nFlips = %d,\n", ssP->flipset.nFlips);
    fprintf(fP, "\t\t.flipIdxA = %sFlipIdxA,\n", objNameA);
    fprintf(fP, "\t},\n");
    fprintf(fP, "\t.infP = &%s,\n", infName);
    fprintf(fP, "};\n\n");
  }
  if (!e) {
    memset(infName, 0, strlen(infName));
    strcpy(infName, objNameA);
    strcat(infName, "Stripmap");
    strcat(infName, "Inf");
    e = inflatableAppend(smP->infP, fP, infName);
  }
  if (!e) {
    fprintf(fP, "\n\n");
    fprintf(fP, "StripmapS %sStripmap = {\n", objNameA);
    fprintf(fP, "\t.nIndices = %d,\n", smP->nIndices);
    fprintf(fP, "\t.infP = &%s\n", infName);
    fprintf(fP, "};\n\n");
  }

  return e;
}

Error writeStripData(char *objNameA, char *OBJ_TYPE, U8 verbose, StripsetS *ssP, StripmapS *smP) {
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
