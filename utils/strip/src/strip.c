#include "strip.h"
#define DEBUG_ 0

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

// For debugging
void printStripData(StripDataS *sdP) {
  if (sdP) {
    if (sdP->ss.unpackedDataP) {
      int i = 0, j = 0;
      printf("Stripset data:\n");
      printf("%d strips %d units each\n", sdP->ss.nUnits / sdP->ss.nUnitsPerStrip, sdP->ss.nUnitsPerStrip);
      printf("\n%03d) ", j++);
      U8 *byteP    = sdP->ss.unpackedDataP;
      U8 *byteEndP = byteP + arrayGetNElems(sdP->ss.unpackedDataP);
      for (; byteP < byteEndP; ++byteP) {
        printf("%d", *byteP);
        if (++i >= sdP->ss.nUnitsPerStrip) {
          i = 0;
          if ((byteP + 1) < byteEndP) {
            printf("\n%03d) ", j++);
          }
        }
      }
      printf("\n\n");
    }
    else {
      printf("can't print a null stripset\n");
    }
    if (sdP->sm.infP->inflatedDataP) {
      printf("Stripmap data:\n");
      printf("%d elems\n", sdP->sm.nIndices);
      U16 *shortP    = sdP->sm.infP->inflatedDataP;
      U16 *shortEndP = shortP + sdP->sm.nIndices;
      for (; shortP < shortEndP; ++shortP) {
        printf("%02d ", *shortP);
      }
      printf("\n\n");
    }
    else {
      printf("can't print a null stripmap\n");
    }
  }
  else {
    printf("can't print a null strip data\n");
  }
}

// Pack all the bits in strip set.
static Error _packBits(U8 **ssPackedAP, U8 *ssUnpackedA, U32 nUnits, U32 bpu, U8 verbose) {
  if (!ssPackedAP | !ssUnpackedA || !nUnits || !bpu || (bpu != 1 && bpu !=2 && bpu !=4)) {
    return E_BAD_ARGS;
  }

  U32 nPackedUnitsPerWord     = N_BITS_PER_WORD / bpu;
  U32 nWhollyPackedWords      = nUnits / nPackedUnitsPerWord;
  U32 nUnitsInExtraPackedWord = nUnits % nPackedUnitsPerWord; 

  // Pre-pack stats
  if (verbose) {
    printf("[_packBits] Packing %d bytes to %d bytes (%d full words) + %d bytes (1 partial word)\n" 
        ,nUnits, nWhollyPackedWords * 4, nWhollyPackedWords, nUnitsInExtraPackedWord); 
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
   *
   */
  Error e = arrayNew((void**) ssPackedAP, 
                     sizeof(U8), 
                     4 * (nWhollyPackedWords + (nUnitsInExtraPackedWord > 0))); 
  if (!e) {
    U32 *packedWordP    = (U32*) *ssPackedAP;
    U32 *packedWordEndP = packedWordP + nWhollyPackedWords;
    U8  *inputByteP     = ssUnpackedA;
    U8  *inputByteEndP  = inputByteP + nUnits;
    // For each output word....
    for (; packedWordP < packedWordEndP && inputByteP < inputByteEndP; ++packedWordP) {
      // For each unit you can pack into that output word...
      for (U8 i = 0; i < nPackedUnitsPerWord; ++i) {
        // Pack unit into output word.
        *packedWordP |= *(inputByteP++) << bitIdxA[i];
      }
    }
    // Remainder bytes
    for (U8 i = 0; i < nUnitsInExtraPackedWord; ++i) {
      *packedWordP |= *(inputByteP++) << bitIdxA[i];  // packs 4-bit values into a word
    }
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

static Error _checkMatch(U32 *srcUnpackedWordP, U8 *srcUnpackedDataP, U32 dstUnpackedWord, U32 nUnitsToCompare, U32 nUnits) {
  if (memcmp(srcUnpackedWordP, &dstUnpackedWord, nUnitsToCompare)) {
    printf("Src unpacked word %9d of %9d: %9d != dst %d\n", 
        srcUnpackedWordP - (U32*) srcUnpackedDataP, 
        (nUnits >> 2) - 1,
        *srcUnpackedWordP,
        dstUnpackedWord);
    printf("Bombing out...\n\n");
    return E_BAD_DATA;
  }
  else {
#if 0
    printf("Src unpacked word %9d of %9d: %9d == dst\n", 
        srcUnpackedWordP - (U32*) srcUnpackedDataP, 
        (nUnits >> 2) - 1,  // bytes / 4 = # words
        *srcUnpackedWordP);
#endif
    return SUCCESS;
  }
}


static Error _validateBitPacking(U8 *packedDataA, U8 *srcUnpackedDataP, U32 nUnits, U32 bpu) {
  U32 nPackedUnitsPerWord     = N_BITS_PER_WORD / bpu;
  U32 nWhollyPackedWords      = nUnits / nPackedUnitsPerWord;
  U32 nUnitsInExtraPackedWord = nUnits % nPackedUnitsPerWord; 

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

  U32 *packedWordP         = (U32*) packedDataA;
  U32 *wholePackedWordEndP = packedWordP + nWhollyPackedWords;
  U32 *srcUnpackedWordP    = (U32*) srcUnpackedDataP;
  U32 dstUnpackedWord      = 0;
  U32 j = 0;
  Error e = SUCCESS;

  // Wholly packed words
  for (; !e && packedWordP < wholePackedWordEndP; ++packedWordP) {
    for (j = 0; !e && j < N_BITS_PER_BYTE; j += bpu, ++srcUnpackedWordP) {
      dstUnpackedWord = (*packedWordP >> j) & mask;
      e = _checkMatch(srcUnpackedWordP, srcUnpackedDataP, dstUnpackedWord, 4, nUnits);
    }
  }
  // One partially packed word's remainder units
  if (!e) {
    // While theres at least 4 packed units remaining in word...
    for (j = 0; 
         !e && nUnitsInExtraPackedWord >= 4; 
         nUnitsInExtraPackedWord -= 4, j += bpu, ++srcUnpackedWordP) {
      dstUnpackedWord = (*packedWordP >> j) & mask;
      e = _checkMatch(srcUnpackedWordP, srcUnpackedDataP, dstUnpackedWord, 4, nUnits);
    }
    // Fewer than 4 packed units remaining
    if (!e && nUnitsInExtraPackedWord > 0) {
      dstUnpackedWord = (*packedWordP >> j) & mask;
      e = _checkMatch(srcUnpackedWordP, srcUnpackedDataP, dstUnpackedWord, nUnitsInExtraPackedWord, nUnits);
    }
  }

  if (!e) {
    printf("\nCONGRATULATIONS! We validated your stripmap's bitpacking integrity!\n\n");
  }

  // For each packed remainder byte till it reaches (B),
  //    unpack a byte
  //    see if that unpacked byte equals the original byte
  return e;
}

#if DEBUG_
static Error _validateStripmapping(U8 *ssA, U32 nBytesPerUnpackedStrip, StripmapElem *smA, U8 *srcA) {
  if (!ssA || !smA || !srcA) {
    return E_BAD_ARGS;
  }

  StripmapElem *smElemP = smA;
  StripmapElem *smElemEndP = smElemP + arrayGetNElems(smA);
  U8 *ssElemP = NULL;
  U8 *origElemP = srcA;
  U8 *origElemEndP = origElemP + arrayGetNElems(srcA);

  for (; smElemP < smElemEndP && origElemP < origElemEndP; ++smElemP, origElemP += nBytesPerUnpackedStrip) {
    ssElemP = ssA + (*smElemP * nBytesPerUnpackedStrip); 
    // If unpacked strip doensn't match original data it maps to, bomb out.
    if (memcmp(ssElemP, origElemP, nBytesPerUnpackedStrip)) {
      printf("Found mismatching strip\n");
      return E_BAD_DATA;
    }  // if original strip does not match one in stripset
  }  // for each strip in stripset

  printf("\nCONGRULATIONS! We validated your stripmap's integrity!\n\n");
  return SUCCESS;
}

static Error _validateInflatables(StripDataS *sdP, StripmapElem *smSrcA, U8 *packedStripsetA) {
  if (!sdP || !smSrcA || !packedStripsetA) {
    return E_BAD_ARGS;
  }
  // Initialize inflatables
  Error e = inflatableIni(sdP->ss.infP);
  if (!e) {
    e = inflatableIni(sdP->sm.infP);
  }
  // See if their contents match their sources
  if (!e && memcmp(sdP->ss.infP->inflatedDataP, packedStripsetA, sdP->ss.infP->inflatedLen)) {
    printf("\nSTRIPSET INFLATABLE'S BAD\n");
    e = E_BAD_DATA;
  }
  if (!e && memcmp(sdP->sm.infP->inflatedDataP, smSrcA, sdP->sm.infP->inflatedLen)) {
    printf("\nSTRIPSET INFLATABLE'S BAD\n");
    e = E_BAD_DATA;
  }
  if (!e) {
    printf("\nCONGRATS! We validated your stripset and stripmap inflatables!\n\n");
  }
  inflatableClr(sdP->ss.infP);
  inflatableClr(sdP->sm.infP);
  return e;
}

static Error _validateUnstrippedData(StripDataS *sdP, U8 *srcA) {
  if (!sdP || !sdP->assembledDataA || !sdP->sm.infP || !sdP->sm.infP->inflatedDataP || !srcA) {
    return E_BAD_ARGS;
  }
  // Validate output element size
  if (arrayGetElemSz(sdP->assembledDataA) != arrayGetElemSz(srcA)) {
    printf("[_validateUnstrippedData] Unstripped data's elements size %d doesn't match source data's element size %d.\n");
    return E_BAD_DATA;
  }
  // Validate output element count
  if (arrayGetNElems(sdP->assembledDataA) != arrayGetNElems(srcA)) {
    printf("[_validateUnstrippedData] Unstripped data's %d elements doesn't match source data's %d elements.\n");
    return E_BAD_DATA;
  }

  Error e = SUCCESS;
  if (memcmp(srcA, sdP->assembledDataA, arrayGetElemSz(srcA) * arrayGetNElems(srcA))) {
    printf("[_validateUnstrippedData] Unstripped data doesn't  match original.\n");
    printf("[_validateUnstrippedData] Unstripped data:\n");
    for (U32 i = 0; 
         i < arrayGetElemSz(sdP->assembledDataA) * arrayGetNElems(sdP->assembledDataA); 
         ++i) {
      if (sdP->assembledDataA[i] != srcA[i]) {
        e = E_BAD_DATA;
        StripmapElem *smElemP = (StripmapElem*) sdP->sm.infP->inflatedDataP 
                             + (i / sdP->ss.nUnitsPerStrip);
        U32 maxStripIdx = (sdP->ss.nUnits / sdP->ss.nUnitsPerStrip) - 1;
        printf("[_validateUnstrippedData] Element mismatch: output %3d (from stripset's strip %5d of %5d), source %3d\n", 
            sdP->assembledDataA[i], *smElemP, maxStripIdx, srcA[i]);
        if (*smElemP >= maxStripIdx) {
          printf("Looks like the index is out of bounds. The true last strip is this:\n");
          U8 *lastUnpackedStripP = sdP->ss.unpackedDataP + (sdP->ss.nUnitsPerStrip * (maxStripIdx));
          for (U32 i = 0; i < sdP->ss.nUnitsPerStrip; ++i) {
            printf("%d", lastUnpackedStripP[i]);
          }
          printf("\n\n");
          break;
        }
      }
    }
    printf("\n\n");
    return E_BAD_DATA;
  }

  return e;
}

#endif

Error stripNew(U8 *srcA, const U32 nBytesPerUnpackedStrip, const U8 bitsPerPackedByte, StripDataS **sdPP, U32 flags, U8 verbose) {
  if (!srcA 
      || !sdPP 
      || !nBytesPerUnpackedStrip
      || !bitsPerPackedByte 
      || bitsPerPackedByte > 8
      || ((arrayGetNElems(srcA) * arrayGetElemSz(srcA)) % nBytesPerUnpackedStrip) != 0)
    return E_BAD_ARGS;
  /* Number of strips needed:
   *              unit          strip      8 bits
   * X units    ----------  * -------  *  -----   = 0 strips
   *            1|2|4 bits   32 units     byte        
  */
  // From image's perspective, srcA is the colormap. 
  // Assumes 8 bits per unpacked unit.
  // maximum possible number of strips 
  U32 nStripsInOrigData = (arrayGetNElems(srcA) * arrayGetElemSz(srcA)) 
    / nBytesPerUnpackedStrip;  
  if (verbose) {
    printf("[stripNew] Dividing source data of %d bytes into a MAXIMUM of %d %d-byte strips\n", 
        arrayGetNElems(srcA) * arrayGetElemSz(srcA), 
        nStripsInOrigData, 
        nBytesPerUnpackedStrip);
  }
  // Allocate all these annoying arrays.
  // nStripsInOrigData is the max possible number of strips, so prepare to have up to that many.
  // Strips labelled so far
  U16 stripLabel = 0;
  StripmapElem stripsLabelled[nStripsInOrigData];
  memset(stripsLabelled, 0, sizeof(StripmapElem) * nStripsInOrigData);
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
    // Strip map
    e = arrayNew((void**) &smDataA, sizeof(StripmapElem), nStripsInOrigData);
  }
  if (!e) {
    (*sdPP)->flags = flags;
  }
  if (verbose) {
    printf("Analyzing viability of breaking image into %d strips of %d units each...\n", nStripsInOrigData, nBytesPerUnpackedStrip);
  }

  // Find distinct strips and write an index-mapping to them.
  U8 *currStripP;
  U8 *nextStripP;  
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
        for (StripmapElem ii = i + 1; ii < nStripsInOrigData; ++ii) {
          if (!stripsLabelled[ii]) {
            nextStripP = srcA + (ii * nBytesPerUnpackedStrip);
            // If they match forwards...
            if (!memcmp((const void*) nextStripP, 
                (const void*) currStripP, 
                sizeof(U8) * nBytesPerUnpackedStrip)) {
              smDataA[ii] = stripLabel;  
              stripsLabelled[ii] = 1;
            }  // if strip is straight-up equal
          }  // if this strip isn't labelled...
        }  // for each element after current to look for matches (forward or backwards)...
        ++stripLabel;
      }  // if strip is unlabelled...
    }  // for all strips in original data...
    if (verbose) {
      printf("%d distinct strips out of a maximum possible %d strips\n", stripLabel, nStripsInOrigData);
    }
  }
#if DEBUG_
  if (!e) {
    e = _validateStripmapping(ssUnpackedA, nBytesPerUnpackedStrip, smDataA, srcA);
  }
#endif
  // Slight compression isn't worth the trouble to inflate; keep as is.
  if (!e) {
    U32 nUnitsInStripset = stripLabel * nBytesPerUnpackedStrip;
    // If it's worth doing, store compressed stuff into colormap profile.
    // Compress data into an inflatable.
    if (!e) {
      e = _packBits(&ssPackedA, ssUnpackedA, nUnitsInStripset, bitsPerPackedByte, verbose);
    }
#if DEBUG_
    if (!e) {
      e = _validateBitPacking(ssPackedA, ssUnpackedA, nUnitsInStripset, bitsPerPackedByte);
    }
#endif

    // Fill in empty information.
    // Stripset
    if (!e) {
      (*sdPP)->ss.nUnits = nUnitsInStripset;
      (*sdPP)->ss.nUnitsPerStrip = nBytesPerUnpackedStrip;
      (*sdPP)->ss.bpu = bitsPerPackedByte;
      e = inflatableNew((void*) ssPackedA, &(*sdPP)->ss.infP);
    }
    // Stripmap
    if (!e) {
      (*sdPP)->sm.nIndices = nStripsInOrigData;
       e = inflatableNew((void*) smDataA, &(*sdPP)->sm.infP);
    }
    if (!e && verbose) {
      printf("[stripNew] Compressed size: %d bytes\n", 
          (sizeof(Inflatable) * 2)
        + (*sdPP)->ss.infP->compressedLen
        + (*sdPP)->sm.infP->compressedLen);
    }
#if DEBUG_
    if (!e && verbose) {
      e = _validateInflatables(*sdPP, smDataA, ssPackedA);
    }
#endif
    if (!e) {
      e =  stripIni(*sdPP);
    }
#if DEBUG_
    if (!e) {
      e = _validateUnstrippedData(*sdPP, srcA);
    }
#endif
  }
  else {
    inflatableDel(&(*sdPP)->ss.infP);
    inflatableDel(&(*sdPP)->sm.infP);
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

  char smInfName[strlen(objNameA) + strlen("Stripmap") + strlen("Inf")];
  memset(smInfName, 0, strlen(smInfName));
  strcpy(smInfName, objNameA);
  strcat(smInfName, "Stripmap");
  strcat(smInfName, "Inf");
  char ssInfName[strlen(objNameA) + strlen("Stripset") + strlen("Inf")];
  memset(ssInfName, 0, strlen(ssInfName));
  strcpy(ssInfName, objNameA);
  strcat(ssInfName, "Stripset");
  strcat(ssInfName, "Inf");


  // Stripset inflatable
  Error e = inflatableAppend(sdP->ss.infP, fP, ssInfName);
  if (!e) {
    // Stripmap inflatable
    e = inflatableAppend(sdP->sm.infP, fP, smInfName);
  }
  if (!e) {
    //Stripmap
    fprintf(fP, "\n\n");
    fprintf(fP, "StripDataS %sStripData = {\n", objNameA);
    // Stripmap
    fprintf(fP, "\t.sm = {\n");
    fprintf(fP, "\t\t.nIndices = %d,\n", sdP->sm.nIndices);
    fprintf(fP, "\t\t.infP = &%s\n", smInfName);
    fprintf(fP, "\t},\n");
    // Stripset
    fprintf(fP, "\t.ss = {\n");
    fprintf(fP, "\t\t.nUnitsPerStrip = %d,\n", sdP->ss.nUnitsPerStrip);  // in case we ever want to give each sprite its own strip length... wuh oh.
    fprintf(fP, "\t\t.nUnits  = %d,\n", sdP->ss.nUnits);
    fprintf(fP, "\t\t.bpu  = %d,\n", sdP->ss.bpu);
    fprintf(fP, "\t\t.infP = &%s,\n", ssInfName);
    fprintf(fP, "\t},\n");
    // Unstripped data
    fprintf(fP, "\t.assembledDataA = NULL\n");
    fprintf(fP, "};\n\n");
  }

  return e;
}  // writeStripDataInFile
