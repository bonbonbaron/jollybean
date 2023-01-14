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
  7, 15, 23, 31
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
static Error _packBits(U8 **ssPackedAP, U8 *ssUnpackedA, U32 nUnits, U32 bpu, U8 verbose) {
  if (!ssPackedAP | !ssUnpackedA || !nUnits || !bpu || (bpu != 1 && bpu !=2 && bpu !=4)) {
    return E_BAD_ARGS;
  }

  U32 nUnitsPerWord = N_BITS_PER_WORD / bpu;
  //U32 nWholeWordUnits = nUnits & 0xfffffffc;
  U32 nWholeWordUnits = nUnits / nUnitsPerWord;
  //U32 nRemainingUnits = nUnits & 0x00000003;  // 4 bytes per word
  U32 nPartialWordUnits = nUnits % nUnitsPerWord;  // 4 bytes per word

  // Pre-pack stats
  if (verbose) {
    U32 compressionRatio = 8 / bpu;
    U32 nWholePackedWords = nUnits / compressionRatio;
    U32 nPartialPackedBytes = nUnits / compressionRatio;
    U32 nRemainderUnpackedBytes = nUnits % compressionRatio;
    U32 nRemainderPackedBytes = (nPartialWordUnits > 0);
    printf("[_packBits] Input data: %d bytes\n", nUnits);
    printf("[_packBits] Compression ratio: %d\n", compressionRatio);
    printf("[_packBits] Packed data has %d whole words ( = %d bytes).\n", nWholeWordUnits, nWholeWordUnits << 2);
    printf("[_packBits] Unpacked data has %d bytes that don't make a word.\n", nUnits & 0x3);
    printf("[_packBits] %d whole packed bytes\n", nWholePackedWords);
    printf("[_packBits] %d remaining packed byte\n\n", nRemainderPackedBytes);
    printf("[_packBits] %d remaining unpacked bytes\n\n", nRemainderUnpackedBytes);
  }

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

  /*   N units   M bits   1 byte
   *   ======= * ====== * ======  = # bytes for output
   *      1       unit    8 bits
   *
   *   In the case of a partial byte, you add one more.
   *   TODO: what about the case of a partial word, which is more relevant here?
   *
   */
  printf("[_packBits] making packed array %d bytes for whole words + %d bytes for remainders\n",
         ((bpu * nUnits) >> 3), (nPartialWordUnits > 0));   
  Error e = arrayNew((void**) ssPackedAP, sizeof(U8), ((bpu * nUnits) >> 3) + (nPartialWordUnits > 0));  
  if (!e) {
    U32 *packedWordP = (U32*) *ssPackedAP;
    U32 *packedWordEndP = packedWordP + nWholeWordUnits;
    U8 *unitP = ssUnpackedA;
    U8 *unitEndP = unitP + nUnits;
    // For each output word....
    for (; packedWordP < packedWordEndP && unitP < unitEndP; ++packedWordP) {
      // For each unit you can pack into that output word...
      for (U8 i = 0; i < nUnitsPerWord; ++i) {
        // Pack unit into output word.
        *packedWordP |= *(unitP++) << bitIdxA[i];
      }
    }
    for (U8 i = 0; packedWordP < packedWordEndP && i < nPartialWordUnits; ++i, ++packedWordP) {
      *packedWordP |= *(unitP + i) << bitIdxA[i];  // packs 4-bit values into a word
    }
    // TODO the above's sus. Is the last packed word going beyond the array bounds?
    // Post-pack stats
    if (verbose) {
      printf("[_packBits] unpacked stripset size: %12d bytes\n", nUnits);
      printf("[_packBits] packed   stripset size: %12d bytes\n", 
          arrayGetNElems(*ssPackedAP) * arrayGetElemSz(*ssPackedAP));
    }
  }
  // Otherwise, if something bad happened, clean up.
  else {
    arrayDel((void**) &ssPackedAP);
  }

  return e;
}

static Error _validateBitPacking(U8 *packedDataA, U8 *srcUnpackedDataP, U32 nUnits, U32 bpu) {
  U32 nUnitsPerWord = N_BITS_PER_WORD / bpu;
  U32 nWholePackedWords = nUnits / nUnitsPerWord;
  U32 nRemainingPackedBytes =  (nUnits / (nUnitsPerWord >> 2)) & 0x00000003;  /* units per byte */

  U32 mask = 0;
  switch(bpu) {
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
      printf("[_validateBitPacking] This data isn't packed. No validation to do.\n");
      return SUCCESS;
  }

  U32 *wholePackedWordP = (U32*) packedDataA;
  U32 *srcUnpackedWordP    = (U32*) srcUnpackedDataP;

  U32 *wholePackedWordEndP = wholePackedWordP + nWholePackedWords;
  U32 dstUnpackedWord = 0;


  for (; wholePackedWordP < wholePackedWordEndP; ++wholePackedWordP) {
    for (U32 j = 0; j < N_BITS_PER_BYTE; j += bpu, ++srcUnpackedWordP) {
      dstUnpackedWord = (*wholePackedWordP >> j) & mask;
      if (*srcUnpackedWordP != dstUnpackedWord) {
        printf("Src unpacked word %9d of %9d: %9d != dst %d\n", 
            srcUnpackedWordP - (U32*) srcUnpackedDataP, 
            nUnits >> 2,
            *srcUnpackedWordP,
            dstUnpackedWord);
        printf("Bombing out...\n\n");
        return E_BAD_DATA;
      }
      else {
        printf("Src unpacked word %9d of %9d: %9d == dst\n", 
            srcUnpackedWordP - (U32*) srcUnpackedDataP, 
            nUnits >> 2,  // bytes / 4 = # words
            *srcUnpackedWordP);
      }
    }
  }

  printf("\n\nCONGRATULATIONS! We validated your stripmap's bitpacking integrity!\n\n");

  // For each packed remainder byte till it reaches (B),
  //    unpack a byte
  //    see if that unpacked byte equals the original byte
  return SUCCESS;
}

static inline U32 countWholeBytesOfUnits(U32 nUnits_, U32 bpu) {
 return (nUnits_ / (8 / bpu));
}

// Unpack bits to reconstruct original data (uesd for debugging img.c)
static Error _stripsetUnpack(Stripset *ssP) {
  if (!ssP || (ssP && !ssP->infP) || (ssP && ssP->infP && !ssP->infP->compressedDataA)) {
    return E_BAD_ARGS;
  }
  // start copy
  Error e = arrayNew((void**) &ssP->unpackedDataP, sizeof(U8), ssP->nUnits);
  if (e) {
    return e;
  }

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
      printf("[_stripsetUnpack] This data isn't packed. No validation to do.\n");
      return SUCCESS;
  }

  U32 *wholePackedWordP    = (U32*) ssP->infP->inflatedDataP;
  U32 *wholePackedWordEndP = (U32*) (ssP->infP->inflatedDataP + ssP->infP->inflatedLen);
  U32 *dstUnpackedWordP    = (U32*) ssP->unpackedDataP;
  U32 stripNum = 0;

  for (; wholePackedWordP < wholePackedWordEndP; ++wholePackedWordP) {
    U32 *dstBookmarkP = dstUnpackedWordP;
    for (U32 j = 0; j < N_BITS_PER_BYTE; j += ssP->bpu) {
      *(dstUnpackedWordP++) = (*wholePackedWordP >> j) & mask;
    }
    printf("[_stripsetUnpack] strip %5d: ", stripNum++);
    for (U32 i = 0; i < ssP->nUnitsPerStrip; ++i) {
      printf("%d", dstBookmarkP[i]);
    }
    printf("\n");
  }

  return e;
}

static Error _pieceTogetherMappedStrips(StripDataS *sdP) {
  if (!sdP || !sdP->ss.unpackedDataP || !sdP->sm.infP->inflatedDataP || !sdP->unmappedDataA) {
    return E_BAD_ARGS;
  }
  // Piece together strips
  for (StripmapElem *smElemP = sdP->sm.infP->inflatedDataP;
       smElemP < (StripmapElem*) sdP->sm.infP->inflatedDataP + sdP->sm.nIndices;
       ++smElemP) {
    memcpy(sdP->unmappedDataA + (*smElemP * sdP->ss.nUnitsPerStrip),
           sdP->ss.unpackedDataP,
           sdP->ss.nUnitsPerStrip);
  }

  return SUCCESS;
}

Error stripDataIni(StripDataS *sdP) {
  Error e = inflatableIni(sdP->ss.infP);
  if (!e) {
    e = inflatableIni(sdP->sm.infP);
  }
  if (!e) {
    e = _stripsetUnpack(&sdP->ss);
  }
  if (!e) {
    e = arrayNew((void**) &sdP->unmappedDataA, 
                 sizeof(U8), 
                 sdP->sm.nIndices * sdP->ss.nUnitsPerStrip);
  }
  if (!e) {
    e = _pieceTogetherMappedStrips(sdP);
  }
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
static void _flipStrip(U8 *unflippedStripP, U32 nBytesPerUnpackedStrip, U8 *flippedStripP) {
  memset(flippedStripP, 0, nBytesPerUnpackedStrip);
  U8 *flipUnitP = flippedStripP;
  U8 *unflippedUnitP = unflippedStripP + nBytesPerUnpackedStrip - 1;
  for (; unflippedUnitP >= unflippedStripP; --unflippedUnitP) {
    *(flipUnitP++) = *unflippedUnitP;
  }
}

#if DEBUG_
static Error _validateStripmapping(U8 *ssA, U32 nBytesPerUnpackedStrip, StripmapElem *smA, 
                                   StripmapElem *flipsetA, U32 nFlips, U8 *origDataA) {
  if (!ssA || !smA || !origDataA) {
    return E_BAD_ARGS;
  }

  StripmapElem *smElemP = smA;
  StripmapElem *smElemEndP = smElemP + arrayGetNElems(smA);
  U8 *ssElemP = NULL;
  U8 *origElemP = origDataA;
  U8 *origElemEndP = origElemP + arrayGetNElems(origDataA);

  U8 flippedStrip[nBytesPerUnpackedStrip];

  for (; smElemP < smElemEndP && origElemP < origElemEndP; ++smElemP, origElemP += nBytesPerUnpackedStrip) {
    ssElemP = ssA + (*smElemP * nBytesPerUnpackedStrip); 
    // If unpacked strip doensn't match original data it maps to, see if it's in flipset.
    if (memcmp(ssElemP, origElemP, nBytesPerUnpackedStrip)) {
      // If strip doesn't match, see if it matches when flipped.
      _flipStrip(ssElemP, nBytesPerUnpackedStrip, flippedStrip);
      // If strip matches when reversed, make sure it's in flipset too!
      if (!memcmp(flippedStrip, origElemP, nBytesPerUnpackedStrip)) {
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
        printf("Original data's %dth strip: ", smElemNumber);
        for (U32 i = 0; i < nBytesPerUnpackedStrip; ++i) {
          printf("%d", origElemP[i]);
        }
        printf("\n\nStripset's %dth strip: ", *smElemP);
        for (U32 i = 0; i < nBytesPerUnpackedStrip; ++i) {
          printf("%d", ssElemP[i]);
        }
        printf("\n\n");
        return E_BAD_DATA;
      }
    }  // if original strip does not match one in stripset
    else {
      U32 smElemNumber = smElemP - smA;
      printf("sm Idx %5d = src strip %5d: ", *smElemP, smElemNumber);
      for (U32 i = 0; i < nBytesPerUnpackedStrip; ++i) {
        printf("%d", origElemP[i]);
      }
      printf("\n");
    }
    matchFound:
    continue;
  }  // for each strip in stripset

  printf("\nCONGRULATIONS! We validated your stripmap's integrity!\n\n");
  return SUCCESS;
}
#endif

// It's desirable to keep nTotalBytesPerUnpackedStrip as a power of two.
// Because the unpacker simultaneously unloads packed bytes into multiple unpacked ones.
// If it's not a power of two, the code gets slow and complicated.
Error stripNew(U8 *srcA, const U32 nBytesPerUnpackedStrip, const U8 bitsPerPackedByte, StripDataS **sdPP, U8 verbose) {
  if (!srcA || !sdPP || !nBytesPerUnpackedStrip || !bitsPerPackedByte || bitsPerPackedByte > 8)
    return E_BAD_ARGS;
  /* Number of strips needed:
   *              unit          strip      8 bits
   * X units    ----------  * -------  *  -----   = 0 strips
   *            1|2|4 bits   32 units     byte        
  */
  // From image's perspective, srcA is the colormap. 
  // Assumes 8 bits per unpacked unit.
  U32 nStripsInOrigData = (arrayGetNElems(srcA) * arrayGetElemSz(srcA)) / nBytesPerUnpackedStrip + 1;  // maximum possible number of strips + 1 for remainders, asssuming 8bpp at first
  if (verbose) {
    printf("[stripNew] Dividing source data of %d bytes into a MAXIMUM of %d %d-byte strips\n", 
        arrayGetNElems(srcA) * arrayGetElemSz(srcA), 
        nStripsInOrigData, 
        nBytesPerUnpackedStrip);
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
  U8 flippedCurrStripP[nBytesPerUnpackedStrip];  // may have unused bytes; that's fine/expected
  // Strip set
  U8 *ssUnpackedA = NULL;  // stripset placeholder till we compress it into output's inflatable
  U8 *ssPackedA = NULL;  // stripset placeholder till we compress it into output's inflatable
  // Strip map
  StripmapElem *smDataA = NULL;  // ditto

  // Allocate output stripmap and stripset
  *sdPP = NULL;
  Error e = jbAlloc((void**) sdPP, sizeof(StripDataS), 1);
  if (!e ) {
    memset(*sdPP, 0, sizeof(StripDataS));
    e = arrayNew((void**) &ssUnpackedA, sizeof(U8), nStripsInOrigData * nBytesPerUnpackedStrip);
  }
  if (!e) {
    memset(ssUnpackedA, 0, sizeof(U8) * nStripsInOrigData * nBytesPerUnpackedStrip);
    // Array of Index of strips to flip
    // This gets copied to a properly sized array at the end.
    memset(flipIdxA, 0, sizeof(StripmapElem) * nStripsInOrigData);
    // Strip map
    e = arrayNew((void**) &smDataA, sizeof(StripmapElem), nStripsInOrigData);
  }
  if (verbose) {
    printf("Analyzing viability of breaking image into %d strips of %d units each...\n", nStripsInOrigData, nBytesPerUnpackedStrip);
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
        memcpy((void*) &ssUnpackedA[stripLabel * nBytesPerUnpackedStrip], 
                       &srcA[i * nBytesPerUnpackedStrip], 
                       nBytesPerUnpackedStrip);
        stripsLabelled[i] = 1;  // If it's not labeled, you're labelling it now.
        smDataA[i] = stripLabel;
        /* All strips up to current "i" should be labelled, so search for strips
         * beyond that that match the current, newly labelled strip. */
        currStripP = srcA + (nBytesPerUnpackedStrip * i);
        // Flip this strip only once to speed up the search for flipped matches.
        _flipStrip(currStripP, nBytesPerUnpackedStrip, flippedCurrStripP);
        for (StripmapElem ii = i + 1; ii < nStripsInOrigData; ++ii) {
          if (lastFlipChange != flipIdxA[0]) {
            lastFlipChange = flipIdxA[0];
          }
          if (!stripsLabelled[ii]) {
            nextStripP = srcA + (ii * nBytesPerUnpackedStrip);
            // If they match forwards...
            if (!memcmp((const void*) nextStripP, 
                (const void*) currStripP, 
                sizeof(U8) * nBytesPerUnpackedStrip)) {
              smDataA[ii] = stripLabel;  //flipped strip gets same label
              stripsLabelled[ii] = 1;
            }  // if strip is straight-up equal
            // Not identical as-is. Is it identical with flipping?
            else if (!memcmp((const void*) nextStripP, 
                (const void*) flippedCurrStripP, 
                sizeof(U8) * nBytesPerUnpackedStrip)) {
              // Flip index points to strip in unpacked stripset that should be flipped.
              flipIdxA[nFlips++] = stripLabel;
              smDataA[ii] = stripLabel;
              stripsLabelled[ii] = 1;
            }  // if strip is straight-up equal
          }
        }
        ++stripLabel;
      }  // if strip is unlabelled...
    }  // for all strips in original data...
    --stripLabel;  // last increment is erroneous
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
    e = _validateStripmapping(ssUnpackedA, nBytesPerUnpackedStrip, smDataA, flipIdxA, nFlips, srcA);
  }
#endif
  // Slight compression isn't worth the trouble to inflate; keep as is.
  U32 nUnitsInStripset = 0;
  if (!e) {
    U32 nRemainderUnits = 0;
    if (nRemainderUnits = (arrayGetNElems(srcA) * arrayGetElemSz(srcA)) 
                          % nBytesPerUnpackedStrip) {
      nUnitsInStripset = (stripLabel * (nBytesPerUnpackedStrip - 1)) + nRemainderUnits;
    }
    else {
      nUnitsInStripset = (stripLabel * nBytesPerUnpackedStrip);
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
      memcpy((*sdPP)->ss.flipset.flipIdxA, 
             flipIdxA, 
             nFlips * arrayGetElemSz((*sdPP)->ss.flipset.flipIdxA));
      (*sdPP)->ss.flipset.nFlips = nFlips;
    }
    if (!e) {
      e = _packBits(&ssPackedA, ssUnpackedA, nUnitsInStripset, bitsPerPackedByte, verbose);
    }
#if DEBUG_
    if (!e) {
      e = _validateBitPacking(ssPackedA, ssUnpackedA, nUnitsInStripset, bitsPerPackedByte);
    }
#endif
    if (!e) {
      e = inflatableNew((void*) ssPackedA, &(*sdPP)->ss.infP);
    }
    if (!e && verbose) {
      printf("[stripNew] Compressed size: %d bytes\n", 
          (sizeof(Inflatable) * 2)
        + (*sdPP)->ss.infP->compressedLen
        + (*sdPP)->sm.infP->compressedLen
        );
    }

    // Fill in empty information.
    if (!e) {
      (*sdPP)->ss.nUnits = nUnitsInStripset;
      (*sdPP)->ss.nUnitsPerStrip = nBytesPerUnpackedStrip;
      (*sdPP)->ss.nStrips = stripLabel;
      (*sdPP)->ss.bpu = bitsPerPackedByte;
      (*sdPP)->sm.nIndices = nStripsInOrigData - 1;
    }
    else {
      inflatableDel(&(*sdPP)->ss.infP);
      inflatableDel(&(*sdPP)->sm.infP);
      arrayDel((void**) &(*sdPP)->ss.flipset.flipIdxA);
    }
  }
  arrayDel((void**) &smDataA);
  arrayDel((void**) &ssUnpackedA);
  if (e) {
    jbFree((void**) sdPP);
  }
  return e;
}  // stripNew()

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
    fprintf(fP, "\t.nUnitsPerStrip = %d,\n", sdP->ss.nUnitsPerStrip);  // in case we ever want to give each sprite its own strip length... wuh oh.
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
}  // writeStripDataInFile
