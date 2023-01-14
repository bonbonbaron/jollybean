#include "strip.h"
#define DEBUG_ 1 

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

  if (verbose) {
    printf("We're packing %d units into strips containing %d units per strip.\n", nUnits, N_UNITS_PER_STRIP);
  }

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
  Error e = arrayNew((void**) &bpsA, sizeof(U32), (nUnits / (8 / bpu)) + ((nUnits & 0x1f) > 0));  // 8 units per U32
  if (!e) {
    U32 nUnitsPerWord = N_BITS_PER_WORD / bpu;
    bpsP = bpsA;
    bpsEndP = bpsP + arrayGetNElems(bpsA);
    // For each output word....
    for (; bpsP < bpsEndP && unitP < unitEndP; ++bpsP) {
      // For each unit you can pack into that output word...
      for (U8 i = 0; i < nUnitsPerWord; ++i) {
        // Pack unit into output word.
        *bpsP |= *(unitP++) << bitIdxA[i];
      }
    }
    U8 nRemainingUnits = countRemainderUnits_(nUnits);  // packs 4-bit values into a word
    for (U8 i = 0; bpsP < bpsEndP && i < nRemainingUnits; ++i, ++bpsP) {
      *bpsP |= *(unitP + i) << bitIdxA[i];  // packs 4-bit values into a word
      //printf("%d", (*bpsP & (0x0f << (i * 4))) >> (i * 4));
    }
  }

  // If no errors, replace stripset with bit-packed one.
  if (!e) {
    if (verbose) {
      printf("before packing data: %d bytes\n", arrayGetNElems(*ssDataAP) * arrayGetElemSz(*ssDataAP));
    }
    arrayDel((void**) ssDataAP);
    *ssDataAP = (U8*) bpsA;
    bpsA = NULL;
    if (verbose) {
      printf("after packing data: %d bytes\n", arrayGetNElems(*ssDataAP) * arrayGetElemSz(*ssDataAP));
    }
  }
  // Otherwise, if something bad happened, clean up.
  else {
    arrayDel((void**) &bpsA);
  }

  assert(bpsA == NULL && *ssDataAP != NULL);
  return e;
}

static inline U32 countWholeBytesOfUnits(U32 nUnits_, U32 bpu) {
 return (nUnits_ / (8 / bpu));
}

// TODO validate unpacker with pre-packed stripset inside stripNew()
// Unpack bits to reconstruct original data
Error stripsetUnpack(Stripset *ssP) {
  if (!ssP || (ssP && !ssP->infP) || (ssP && ssP->infP && !ssP->infP->compressedDataA)) {
    return E_BAD_ARGS;
  }

  // Each UNPACKED unit is one byte in size.
  Error e = arrayNew((void**) &ssP->unpackedDataP, sizeof(U8), ssP->nUnits);

  if (!e) {
    const U32 nWordsPerPackedStrip = (N_UNITS_PER_STRIP * ssP->bpu) / N_BITS_PER_WORD;
    const U32 nRemainderUnits = countRemainderUnits_(ssP->nUnits); 

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
        mask = 0xffffffff;  // TODO Point data to already unpacked inflatedDataP and return here.
        break;
    }

    U32 *packedStripP = ssP->infP->inflatedDataP;
    U32 *unpackedStripP = (U32*) ssP->unpackedDataP;
    U32 *unpackedStripEndP = (U32*) ((U8*) unpackedStripP + ssP->nUnits);
    U32 *unpackedWholeStripEndP = (U32*) ((U8*) unpackedStripEndP - nRemainderUnits);

    //*******************************
    //*******************************
    // Unpack whole strips
    for (; unpackedStripP < unpackedWholeStripEndP; ++packedStripP) {
      for (int j = 0; j < N_BITS_PER_BYTE; j += ssP->bpu) {
        *(unpackedStripP++) =  (*packedStripP >> j) & mask;
      }
    }
    goto skipForNow;
    //*******************************
    // Unpack remainder units
    //*******************************
    U8 *packedByteP = (U8*) packedStripP;
    U8 *packedByteEndP = packedByteP + countWholeBytesOfUnits(nRemainderUnits, ssP->bpu);
    U8 *unpackedByteP = (U8*) unpackedStripP;
    /* Handle all the whole bytes of units. */
    //printf("# whole bytes of remainder data: %d\n", byteEndP - byteP);
    while (packedByteP < packedByteEndP) {
      for (U8 i = 0; i < N_BITS_PER_BYTE; i += ssP->bpu) {
        *(unpackedByteP++) =  (*(packedByteP++) >> i) & mask;
      }
    }
    //*******************************
    /* Handle the last, partial byte of data. */
    //*******************************
    U8 iEnd = countUnitsInPartialByte4BPU_(nRemainderUnits);
    //printf("# partial bytes of remainder data: %d\n", iEnd);
    for (U8 i = 0; i < iEnd; i += ssP->bpu) {
      *(unpackedByteP++) =  (*(packedByteP++) >> i) & mask;
    }
  }

  skipForNow:
  return e;
}

// This is for UNPACKED strips, so units are in full  bytes.
/* EXAMPLE:
   ========
 *
 * with 3 used units out of 8:
 * normal         flipped
 * ======         ========
 * 14300000  -->  34100000
 */
static void _flipStrip(U8 *unflippedStripP, U32 nUsedBytesPerUnpackedStrip, U32 nTotalBytesPerUnpackedStrip, U8 *flippedStripP) {
  memset(flippedStripP, 0, nTotalBytesPerUnpackedStrip);
  U8 *flipUnitP = flippedStripP;
  U8 *unflippedUnitP = unflippedStripP + nUsedBytesPerUnpackedStrip - 1;
  for (; unflippedUnitP >= unflippedStripP; --unflippedUnitP) {
    *(flipUnitP++) = *unflippedUnitP;
  }
}

#if DEBUG_
Error validateUnpackedStripset(U8 *ssA, U32 nUsedBytesPerUnpackedStrip, 
                               U32 nTotalBytesPerUnpackedStrip, StripmapElem *smA, 
                               StripmapElem *flipsetA, U32 nFlips, U8 *origDataA) {
  if (!ssA || !smA || !origDataA) {
    return E_BAD_ARGS;
  }

  StripmapElem *smElemP = smA;
  StripmapElem *smElemEndP = smElemP + arrayGetNElems(smA);
  U8 *ssElemP = NULL;
  U8 *origElemP = origDataA;
  U8 *origElemEndP = origElemP + arrayGetNElems(origDataA);

  U8 flippedStrip[nTotalBytesPerUnpackedStrip];

  for (; smElemP < smElemEndP && origElemP < origElemEndP; ++smElemP, origElemP += N_BYTES_PER_UNPACKED_STRIP) {
    ssElemP = ssA + (*smElemP * N_BYTES_PER_UNPACKED_STRIP); 
    // If unpacked strip doensn't match original data it maps to, see if it's in flipset.
    if (memcmp(ssElemP, origElemP, nUsedBytesPerUnpackedStrip)) {
      // If strip doesn't match, see if it matches when flipped.
      _flipStrip(ssElemP, nUsedBytesPerUnpackedStrip, nTotalBytesPerUnpackedStrip, flippedStrip);
      // If strip matches when reversed, make sure it's in flipset too!
      if (!memcmp(flippedStrip, origElemP, nUsedBytesPerUnpackedStrip)) {
        U32 smElemNumber = smElemP - smA;
        printf("Stripset's %dth strip matches source data when flipped!\n", smElemNumber);
        for (U32 i = 0; i < nFlips; ++i) {
          printf("\tTrying flipset[%d] = %d...\n", i, flipsetA[i]);
          if (*smElemP == flipsetA[i]) {
            printf("\tMatching flipset index found.\n");
            goto matchFound;
          }
        }
        // If it doesn't exist in flipset, 
        printf("But stripset index %d doesn't exist in flipset. Bombing out...", *smElemP);
        return E_BAD_DATA;
      }  // if flipped strip matches current
      else {
        printf("\n\nUnpacked stripset doesn't represent original data correctly! Bombing out...\n\n");
        U32 smElemNumber = smElemP - smA;
        printf("Original data's %dth strip:\n", smElemNumber);
        for (U32 i = 0; i < N_BYTES_PER_UNPACKED_STRIP; ++i) {
          printf("%d", origElemP[i]);
        }
        printf("\n\nStripset's %dth strip:\n", *smElemP);
        for (U32 i = 0; i < N_BYTES_PER_UNPACKED_STRIP; ++i) {
          printf("%d", ssElemP[i]);
        }
        return E_BAD_DATA;
      }
    }  // if original strip does not match one in stripset
    matchFound:
    continue;
  }  // for each strip in stripset
  return SUCCESS;
}
#endif

// It's desirable to keep nTotalBytesPerUnpackedStrip as a power of two.
// Because the unpacker simultaneously unloads packed bytes into multiple unpacked ones.
// If it's not a power of two, the code gets slow and complicated.
Error stripNew(U8 *srcA, const U32 nTotalBytesPerUnpackedStrip, const U32 nUsedBytesPerUnpackedStrip, const U8 bitsPerPackedByte, StripDataS **sdPP, U8 verbose) {
  if (!srcA || !sdPP || !nTotalBytesPerUnpackedStrip || !nUsedBytesPerUnpackedStrip || nUsedBytesPerUnpackedStrip > nTotalBytesPerUnpackedStrip || !bitsPerPackedByte || bitsPerPackedByte > 8)
    return E_BAD_ARGS;
  /* Number of strips needed:
   *              unit          strip      8 bits
   * X units    ----------  * -------  *  -----   = 0 strips
   *            1|2|4 bits   32 units     byte        
  */
  // From image's perspective, srcA is the colormap. 
  // Assumes 8 bits per unpacked unit.
  U32 nStripsInOrigData = (arrayGetNElems(srcA) * arrayGetElemSz(srcA)) / nUsedBytesPerUnpackedStrip + 1;  // maximum possible number of strips + 1 for remainders, asssuming 8bpp at first
  if (verbose) {
    printf("[stripNew] Dividing source data of %d bytes into a MAXIMUM of %d %d-byte strips\n", 
        arrayGetNElems(srcA) * arrayGetElemSz(srcA), 
        nStripsInOrigData, 
        nUsedBytesPerUnpackedStrip);
  }
  U16 stripLabel = 0;
  // Allocate all these annoying arrays.
  // nStripsInOrigData is the max possible number of strips, so prepare to have up to that many.
  // Strips labelled so far
  StripmapElem stripsLabelled[nStripsInOrigData];
  memset(stripsLabelled, 0, sizeof(StripmapElem) * nStripsInOrigData);
  // Flip set
  StripmapElem flipIdxA[nStripsInOrigData];
  // Flipped strip used to search for other strips that match when flipped
  U8 flippedCurrStripP[nTotalBytesPerUnpackedStrip];  // may have unused bytes; that's fine/expected
  // Strip set
  U8 *ssDataA = NULL;  // stripset placeholder till we compress it into output's inflatable
  // Strip map
  StripmapElem *smDataA = NULL;  // ditto

  // Allocate output stripmap and stripset
  *sdPP = NULL;
  Error e = jbAlloc((void**) sdPP, sizeof(StripDataS), 1);

  if (!e ) {
    e = arrayNew((void**) &ssDataA, sizeof(U8), nStripsInOrigData * nTotalBytesPerUnpackedStrip);
  }
  if (!e) {
    memset(ssDataA, 0, sizeof(U8) * nStripsInOrigData * nTotalBytesPerUnpackedStrip);
    // Array of Index of strips to flip
    // This gets copied to a properly sized array at the end.
    memset(flipIdxA, 0, sizeof(StripmapElem) * nStripsInOrigData);
    // Strip map
    e = arrayNew((void**) &smDataA, sizeof(StripmapElem), nStripsInOrigData);
  }
  if (verbose) {
    printf("Analyzing viability of breaking image into %d strips of %d units each...\n", nStripsInOrigData, N_UNITS_PER_STRIP);
  }

  // Find distinct strips and write an index-mapping to them.
  U8 *currStripP;
  U8 *nextStripP;  
  U32 nFlips = 0;
  StripmapElem lastFlipChange = 0;
  if (!e) {
    for (U32 i = 0; i < nStripsInOrigData; ++i) {
      // This strip hasn't been labelled yet. 
      // So give it its own label and add it to the set of unique strips.
      if (!stripsLabelled[i]) {
        memcpy((void*) &ssDataA[stripLabel * nTotalBytesPerUnpackedStrip], 
                       &srcA[i * nUsedBytesPerUnpackedStrip], 
                       nUsedBytesPerUnpackedStrip);
        stripsLabelled[i] = 1;  // If it's not labeled, you're labelling it now.
        smDataA[i] = stripLabel;
        /* All strips up to current "i" should be labelled, so search for strips
         * beyond that that match the current, newly labelled strip. */
        currStripP = srcA + (nUsedBytesPerUnpackedStrip * i);
        // Flip this strip only once to speed up the search for flipped matches.
        _flipStrip(currStripP, nUsedBytesPerUnpackedStrip, nTotalBytesPerUnpackedStrip, flippedCurrStripP);
        for (StripmapElem ii = i + 1; ii < nStripsInOrigData; ++ii) {
          if (lastFlipChange != flipIdxA[0]) {
            printf("stripLabel: %d\tflipset[0] = %d\n", stripLabel, flipIdxA[0]);
            lastFlipChange = flipIdxA[0];
          }
          if (!stripsLabelled[ii]) {
            nextStripP = srcA + (ii * nUsedBytesPerUnpackedStrip);
            // If they match forwards...
            if (!memcmp((const void*) nextStripP, 
                (const void*) currStripP, 
                sizeof(U8) * nUsedBytesPerUnpackedStrip)) {
              smDataA[ii] = stripLabel;  //flipped strip gets same label
              stripsLabelled[ii] = 1;
            }  // if strip is straight-up equal
            // Not identical as-is. Is it identical with flipping?
            else if (!memcmp((const void*) nextStripP, 
                (const void*) flippedCurrStripP, 
                sizeof(U8) * nUsedBytesPerUnpackedStrip)) {
              // Flip index points to strip in unpacked stripset that should be flipped.
              printf("Flip found: flipIdxA[%d] = %d\n", nFlips, stripLabel);
              flipIdxA[nFlips++] = stripLabel;
              printf("Flip idx inserted: %d\n", flipIdxA[nFlips - 1]);
              smDataA[ii] = stripLabel;
              stripsLabelled[ii] = 1;
            }  // if strip is straight-up equal
          }
        }
        ++stripLabel;
      }  // if strip is unlabelled...
    }  // for all strips in original data...
    if (verbose) {
      printf("%d distinct strips out of a maximum possible %d strips\n", stripLabel, nStripsInOrigData);
    }
  }
#if DEBUG_
  if (!e) {
    printf("All flipmap elements: ");
    for (U32 i = 0; i < nFlips; ++i) {
      printf("%d, ", flipIdxA[i]);
    }
    printf("\n");
  }
  if (!e) {
    e = validateUnpackedStripset(ssDataA, nUsedBytesPerUnpackedStrip,
                                 nTotalBytesPerUnpackedStrip, smDataA, 
                                 flipIdxA, nFlips, srcA);
  }
#endif
  // Slight compression isn't worth the trouble to inflate; keep as is.
  U32 nUnitsInStripset = 0;
  if (!e) {
    U32 nRemainderUnits = 0;
    if (nRemainderUnits = arrayGetNElems(srcA) & UNWHOLE_STRIP_MASK) {
      nUnitsInStripset = (stripLabel * (N_UNITS_PER_STRIP - 1)) + nRemainderUnits;
    }
    else {
      nUnitsInStripset = (stripLabel * N_UNITS_PER_STRIP);
    }
    // If it's worth doing, store compressed stuff into colormap profile.
    // Compress data into an inflatable.
    if (!e) {
       e = inflatableNew((void*) smDataA, &(*sdPP)->sm.infP);
    }
    if (!e) {
      e = arrayNew((void**) &(*sdPP)->ss.flipset.flipIdxA, sizeof(StripmapElem), nFlips);
    }
    if (!e) {
      e = _packBits(&ssDataA, stripLabel, nUnitsInStripset, bitsPerPackedByte, verbose);
    }
    if (!e) {
      e = inflatableNew((void*) ssDataA, &(*sdPP)->ss.infP);
    }
    if (!e) {
      (*sdPP)->ss.nUnits = nUnitsInStripset;
      (*sdPP)->ss.nUnitsPerStrip = N_UNITS_PER_STRIP;
      (*sdPP)->ss.nStrips = stripLabel;
      (*sdPP)->ss.bpu = bitsPerPackedByte;
    }
    if (e) {
      inflatableDel(&(*sdPP)->ss.infP);
      inflatableDel(&(*sdPP)->sm.infP);
      arrayDel((void**) &(*sdPP)->ss.flipset.flipIdxA);
    }
  }
  arrayDel((void**) &smDataA);
  arrayDel((void**) &ssDataA);
  if (e) {
    jbFree((void**) sdPP);
    printf("Something errored out in stripNew.\n");
  }
  return e;
}

Error writeStripDataInFile(FILE *fP, U8 verbose, char *objNameA, StripDataS *sdP) {
  if (!fP || !sdP) {
    return E_BAD_ARGS;
  }

  char infName[strlen(objNameA) + strlen("Stripset") + strlen("Inf")];

  // Flip set
  fprintf(fP, "static U16 %sFlipIdxA[] = {\n", objNameA);
  writeRawData16(fP, (U16*) sdP->ss.flipset.flipIdxA, arrayGetNElems(sdP->ss.flipset.flipIdxA));
  fprintf(fP, "};\n\n");
  strcpy(infName, objNameA);
  strcat(infName, "Stripset");
  strcat(infName, "Inf");
  Error e = inflatableAppend(sdP->ss.infP, fP, infName);

  if (!e) {
    fprintf(fP, "\n\n");
    // Strip set 
    fprintf(fP, "Stripset %sStripset = {\n", objNameA);
    fprintf(fP, "\t.nUnitsPerStrip = %d,\n", N_UNITS_PER_STRIP);  // in case we ever want to give each sprite its own strip length... wuh oh.
    fprintf(fP, "\t.nStrips = %d,\n", sdP->ss.nStrips);
    fprintf(fP, "\t.nUnits  = %d,\n", sdP->ss.nUnits);
    fprintf(fP, "\t.bpu  = %d,\n", sdP->ss.bpu);
    fprintf(fP, "\t.flipset = {\n", objNameA);
    fprintf(fP, "\t\t.nFlips = %d,\n", sdP->ss.flipset.nFlips);
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
    e = inflatableAppend(sdP->sm.infP, fP, infName);
  }
  if (!e) {
    fprintf(fP, "\n\n");
    fprintf(fP, "Stripmap %sStripmap = {\n", objNameA);
    fprintf(fP, "\t.nIndices = %d,\n", sdP->sm.nIndices);
    fprintf(fP, "\t.infP = &%s\n", infName);
    fprintf(fP, "};\n\n");
  }

  return e;
}
