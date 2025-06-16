#include "sd.h"
#define DEBUG_ 1

// These three arrays describe where each consecutive unit
// is located in the current size_t word. We stagger them like this
// so we can inflate 4 units at a time in xRender.c.

#if ( __WORDSIZE == 32 )

const static U8 bitIdx1Bpu[] = {
  0,  8, 16, 24,
  1,  9, 17, 25,
  2, 10, 18, 26,
  3, 11, 19, 27,
  4, 12, 20, 28,
  5, 13, 21, 29,
  6, 14, 22, 30,
  7, 15, 23, 31
};

const static U8 bitIdx2Bpu[] = {
  0,  8, 16, 24,
  2, 10, 18, 26,
  4, 12, 20, 28,
  6, 14, 22, 30,
};

#define MASK_1BPP (0x01010101)
#define MASK_2BPP (0x03030303)
#define MASK_4BPP (0x0f0f0f0f)

const static U8 bitIdx4Bpu[] = {
  0, 8, 16, 24,
  4, 12, 20, 28,
};

#define FULLY_PACKED_WORD_COUNT_MASK (0xffffffc0)
#define FULLY_PACKED_WORD_COUNT_RSHIFT (5)

#elif ( __WORDSIZE == 64 )

const static U8 bitIdx1Bpu[] = {
  0,  8, 16, 24, 32, 40, 48, 56,
  1,  9, 17, 25, 33, 41, 49, 57,
  2, 10, 18, 26, 34, 42, 50, 58, 
  3, 11, 19, 27, 35, 43, 51, 59, 
  4, 12, 20, 28, 36, 44, 52, 60, 
  5, 13, 21, 29, 37, 45, 53, 61, 
  6, 14, 22, 30, 38, 46, 54, 62, 
  7, 15, 23, 31, 39, 47, 55, 63 
};

const static U8 bitIdx2Bpu[] = {
  0,  8, 16, 24, 32, 40, 48, 56,
  2, 10, 18, 26, 34, 42, 50, 58,
  4, 12, 20, 28, 36, 44, 52, 60,
  6, 14, 22, 30, 38, 46, 54, 62
};

const static U8 bitIdx4Bpu[] = {
  0,  8, 16, 24, 32, 40, 48, 56,
  4, 12, 20, 28, 36, 44, 52, 60
};

#define MASK_1BPP (0x0101010101010101)
#define MASK_2BPP (0x0303030303030303)
#define MASK_4BPP (0x0f0f0f0f0f0f0f0f)

#define FULLY_PACKED_WORD_COUNT_MASK (0xffffffffffffffc0)
#define FULLY_PACKED_WORD_COUNT_RSHIFT (6)

#else
static_assert( 0, "Jollybean only supports 32- and 64-bit machines.");
#endif


// For debugging
void printStripData(StripDataS *sdP) {
  if (sdP) {
    if (sdP->ss.unpackedDataA) {
      int i = 0, j = 0;
      printf("Stripset data:\n");
      printf("%d strips %d units each\n", sdP->ss.nUnits / sdP->ss.nUnitsPerStrip, sdP->ss.nUnitsPerStrip);
      printf("\n%03d) ", j++);
      U8 *byteP    = sdP->ss.unpackedDataA;
      U8 *byteEndP = byteP + arrayGetNElems(sdP->ss.unpackedDataA);
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
    // If we're assembling, that implies we have stripmap data to look at.
    if ( !( sdP->flags & SD_SKIP_ASSEMBLY_ ) ) {
      assert( sdP->sm.infP );
      assert( sdP->sm.infP->inflatedDataP );
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
static size_t* _packBits(U8 *ssUnpackedA, size_t nUnits, size_t bpu, U8 verbose) {
  assert (ssUnpackedA && nUnits && bpu && (bpu == 1 || bpu ==2 || bpu ==4));
  if (verbose) {
    printf("nUnits: %lx\nbpu: %lx\n", nUnits, bpu);
  }

  const size_t nPackedUnitsPerWord     = __WORDSIZE / bpu;
  const size_t nWhollyPackedWords      = ( ( nUnits * bpu ) >> FULLY_PACKED_WORD_COUNT_RSHIFT );
  const size_t nUnitsInExtraPackedWord = nUnits % nPackedUnitsPerWord; 

  // Pre-pack stats
  if (verbose) {
    printf("[_packBits] Packing %ld bytes into %ld bytes (%ld full words) + %ld bytes (%d partial word)\n" 
        , nUnits, nWhollyPackedWords * sizeof(size_t), nWhollyPackedWords, (nUnitsInExtraPackedWord * bpu + 4) / 8, nUnitsInExtraPackedWord > 0 ? 1 : 0); 
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
      if (verbose) {
        printf("[_packBits] no packing to be done.\n");
      }
      return NULL;
  }

  /*   N units   M bits   1 byte
   *   ======= * ====== * ======  = # bytes for output
   *      1       unit    8 bits
   *
   *   In the case of a partial byte, you add one more.
   *
   */
  size_t* ssPackedA = arrayNew(  
      sizeof(size_t), 
      (nWhollyPackedWords + (( nUnitsInExtraPackedWord > 0 ? 1 : 0) )), TEMPORARY); 
  memset(ssPackedA, 0, arrayGetNElems( ssPackedA ) * arrayGetElemSz( ssPackedA ) );

  if (verbose) {
    printf("There are %d packed words.\n", arrayGetNElems( ssPackedA ) );
  }
  size_t *packedWordP    = (size_t*) ssPackedA;
  size_t *fullyPackedWordEndP = packedWordP + nWhollyPackedWords;
  U8  *inputByteP     = ssUnpackedA;
  U8  *inputByteEndP  = inputByteP + nUnits;
  // Stagger the input bits across each output word
  //   so that they occur in an order convenient for 
  //   masking + shifting into place.
  size_t nWordsSoFar = 1;
  for (; packedWordP < fullyPackedWordEndP; ++packedWordP, ++nWordsSoFar) {
    // For each unit you can pack into that output word...
    for (U8 i = 0; i < nPackedUnitsPerWord; ++i) {
      if ( verbose && i % sizeof( size_t ) == 0 ) {  // 8 because 
        printf("%3ld / %3ld) packing 0x%016lx\n", nWordsSoFar, nWhollyPackedWords + (nUnitsInExtraPackedWord > 0 ? 1 : 0 ), *( (size_t*) inputByteP ));
      }
      // Pack unit into output word.
      *packedWordP |= (size_t) *(inputByteP++) << bitIdxA[i];
    }
    if (verbose) {
      printf(" into 0x%016lx\n", *packedWordP);
    }
  }
  if (verbose) {
    printf("We filled %ld whole words.\n", packedWordP - ssPackedA);
  }
  // Remainder bytes
  if (verbose && nUnitsInExtraPackedWord) {
    printf("\e[95mNow we're filling in the last one.\e[0m\n");
  }
  // First, zero-out all bytes in the extra packed word for consistency of results.
  if (verbose) {
    printf("%3ld / %3ld) packing 0x", nWordsSoFar, nWhollyPackedWords + (nUnitsInExtraPackedWord > 0 ? 1 : 0 ));
  }
  size_t inputWord = 0;  // to make printouts of the input words cleaner
  for (U8 i = 0; i < nUnitsInExtraPackedWord; ++i) {
    if (verbose && i % sizeof( size_t ) == 0 ) {
      printf("%3ld / %3ld) packing 0x%016lx\n", nWordsSoFar, nWhollyPackedWords + (nUnitsInExtraPackedWord > 0 ? 1 : 0 ), inputWord);
    }
    if (verbose) {
      printf("%02x", *inputByteP);
    }
    // *packedWordP |= *(inputByteP++) << bitIdxA[i];  
    *packedWordP |= (size_t) *(inputByteP++) << bitIdxA[i];
  }
  if (verbose) {
    printf(" into 0x%016lx\n", *packedWordP);
  }

  assert( inputByteP == inputByteEndP );

  // Post-pack stats
  if (verbose) {
    printf("[_packBits] unpacked stripset size: %12ld bytes\n", nUnits);
    printf("[_packBits] packed   stripset size: %12d bytes\n", 
        arrayGetNElems(ssPackedA) * arrayGetElemSz(ssPackedA));
  }
  // Otherwise, if something bad happened, clean up.
  return ssPackedA;
}  // _packBits()

inline static void _checkMatch(size_t srcUnpackedWord, size_t dstUnpackedWord, size_t nUnitsToCompare) {
  assert(!memcmp(&srcUnpackedWord, &dstUnpackedWord, nUnitsToCompare));
#if 0
  printf("Src unpacked word %9d of %9d: %9d != dst %d\n", 
      srcUnpackedWordP - (size_t*) srcUnpackedDataP, 
      (nUnits / sizeof(size_t) ) - 1,
      *srcUnpackedWordP,
      dstUnpackedWord);
  printf("Bombing out...\n\n");
  else {
    printf("Src unpacked word %9d of %9d: %9d == dst\n", 
        srcUnpackedWordP - (size_t*) srcUnpackedDataP, 
        (nUnits / sizeof(size_t) ) - 1,  // bytes / (bytes/word) = # words
        *srcUnpackedWordP);
  }
#endif
}


#if DEBUG_
static void _validateBitPacking(size_t *packedDataA, U8 *srcUnpackedDataP, size_t nUnits, size_t bpu, U8 verbose) {
  const size_t nPackedUnitsPerWord     = __WORDSIZE / bpu;
  const size_t nWhollyPackedWords      = ( ( nUnits * bpu ) >> FULLY_PACKED_WORD_COUNT_RSHIFT );
  size_t nUnitsInExtraPackedWord = nUnits % nPackedUnitsPerWord; 

  size_t mask = 0;
  switch(bpu) {
    case 1:
      mask = MASK_1BPP;
      break;
    case 2:
      mask = MASK_2BPP;
      break;
    case 4:
      mask = MASK_4BPP;
      break;
    default:
      if (verbose) {
        printf("[_validateBitPacking] This data isn't packed. No validation to do.\n");
      }
      return;
  }

  size_t *packedWordP         = packedDataA;
  size_t *wholePackedWordEndP = packedWordP + nWhollyPackedWords;
  size_t *srcUnpackedWordP    = (size_t*) srcUnpackedDataP;
  size_t dstUnpackedWord      = 0;
  size_t j = 0;
  size_t nWordsSoFar = 1;

  // Wholly packed words
  for (; packedWordP < wholePackedWordEndP; ++packedWordP, ++nWordsSoFar) {
    for (j = 0; j < N_BITS_PER_BYTE; j += bpu, ++srcUnpackedWordP) {
      dstUnpackedWord = (*packedWordP >> j) & mask;
      if (verbose) {
        printf("%2ld / %2ld) unpacking 0x%016lx into 0x%016lx, ?= orig 0x%016lx\n", nWordsSoFar, nWhollyPackedWords + (nUnitsInExtraPackedWord > 0 ? 1 : 0 ), *packedWordP, dstUnpackedWord, *srcUnpackedWordP);
      }
      _checkMatch(*srcUnpackedWordP, dstUnpackedWord, sizeof(dstUnpackedWord));
    }
  }
  // One partially packed word's remainder units
  // While theres at least 4 packed units remaining in word...
  for (j = 0; 
      nUnitsInExtraPackedWord >= sizeof(size_t); 
      nUnitsInExtraPackedWord -= sizeof(size_t), j += bpu, ++srcUnpackedWordP) {
    dstUnpackedWord = (*packedWordP >> j) & mask;
    if (verbose) {
      printf("%2ld / %2ld) unpacking 0x%016lx into 0x%016lx, ?= orig 0x%016lx\n", nWordsSoFar, nWhollyPackedWords + (nUnitsInExtraPackedWord > 0 ? 1 : 0 ), *packedWordP, dstUnpackedWord, *srcUnpackedWordP);
    }
    _checkMatch(*srcUnpackedWordP, dstUnpackedWord, sizeof(dstUnpackedWord));
  }
  // Fewer than (# bytes / word) packed units remaining
  if (nUnitsInExtraPackedWord > 0) {
    dstUnpackedWord = (*packedWordP >> j) & mask;
    _checkMatch(*srcUnpackedWordP, dstUnpackedWord, nUnitsInExtraPackedWord);
  }

  if (verbose) {
    printf("\n\e[92mCONGRATULATIONS! We validated your bitpacking integrity!\e[0m\n\n");
  }
} // _validateBitPacking()
#endif

#if DEBUG_
static void _validateStripmapping(U8 *ssA, size_t nBytesPerUnpackedStrip, StripmapElem *smA, U8 *srcA, U8 verbose) {
  assert (ssA && smA && srcA);

  StripmapElem *smElemP = smA;
  StripmapElem *smElemEndP = smElemP + arrayGetNElems(smA);
  U8 *ssElemP = NULL;
  U8 *origElemP = srcA;
  U8 *origElemEndP = origElemP + arrayGetNElems(srcA);

  for (; smElemP < smElemEndP && origElemP < origElemEndP; ++smElemP, origElemP += nBytesPerUnpackedStrip) {
    ssElemP = ssA + (*smElemP * nBytesPerUnpackedStrip); 
    // If unpacked strip doensn't match original data it maps to, bomb out.
    assert (!memcmp(ssElemP, origElemP, nBytesPerUnpackedStrip));
  }  // for each strip in stripset

  if (verbose) {
    printf("\n\e[92mCONGRATULATIONS! We validated your stripmap's integrity!\e[0m\n\n");
  }
}

#if 0
static void _validateInflatables(StripDataS *sdP, StripmapElem *smSrcA, U8 *packedStripsetA) {
  assert (sdP && smSrcA && packedStripsetA);
  // Initialize inflatables
  inflatableIni(sdP->ss.infP);
  inflatableIni(sdP->sm.infP);
  // See if their contents match their sources
  assert (!memcmp(sdP->ss.infP->inflatedDataP, packedStripsetA, sdP->ss.infP->inflatedLen));
  assert(!memcmp(sdP->sm.infP->inflatedDataP, smSrcA, sdP->sm.infP->inflatedLen));
  printf("\nCONGRATS! We validated your stripset and stripmap inflatables!\n\n");
}
#endif

static void _validateAssembledData(StripDataS *sdP, U8 *srcA, U8 verbose) {
  assert(sdP);
  assert(sdP->assembledDataA);
  assert(sdP->sm.infP);
  assert(sdP->sm.infP->inflatedDataP);
  assert(srcA);
  // Validate output element size
  assert (arrayGetElemSz(sdP->assembledDataA) == arrayGetElemSz(srcA));
  // Validate output element count
  assert (arrayGetNElems(sdP->assembledDataA) == arrayGetNElems(srcA));

  if (memcmp(srcA, sdP->assembledDataA, arrayGetElemSz(srcA) * arrayGetNElems(srcA))) {
    if (verbose) {
      printf("[_validateAssembledData] Assembled data doesn't  match original.\n");
      printf("[_validateAssembledData] Assembled data:\n");
    }
    size_t i;
    for (i = 0; 
        i < arrayGetElemSz(sdP->assembledDataA) * arrayGetNElems(sdP->assembledDataA); 
        ++i) {
      if (sdP->assembledDataA[i] != srcA[i]) {
        StripmapElem *smElemP = (StripmapElem*) sdP->sm.infP->inflatedDataP 
          + (i / sdP->ss.nUnitsPerStrip);
        size_t maxStripIdx = (sdP->ss.nUnits / sdP->ss.nUnitsPerStrip) - 1;
        if (verbose) {
          printf("[_validateAssembledData] Element mismatch: output %3d (from stripset's strip %5d of %5ld), source %3d\n", sdP->assembledDataA[i], *smElemP, maxStripIdx, srcA[i]);
        }
        if (*smElemP >= maxStripIdx) {
          if (verbose) {
            printf("Looks like the index is out of bounds. The true last strip is this:\n");
          }
          U8 *lastUnpackedStripP = sdP->ss.unpackedDataA + (sdP->ss.nUnitsPerStrip * (maxStripIdx));
          if (verbose) {
            for (size_t i = 0; i < sdP->ss.nUnitsPerStrip; ++i) {
              printf("%d", lastUnpackedStripP[i]);
            }
            printf("\n\n");
          }
          break;
        }
      }
    }

    // Bomb out if it failed in the above for-loop.
    assert (i == arrayGetElemSz(sdP->assembledDataA) * arrayGetNElems(sdP->assembledDataA));

    if (verbose) {
      printf("\n\n");
    }
  }
}

#endif

StripDataS* stripNew(U8 *srcA, const size_t nBytesPerUnpackedStrip, const U8 bpu,  size_t flags, U8 verbose) {
  assert (srcA);
  assert (nBytesPerUnpackedStrip);
  assert ( bpu == 1 || bpu == 2 || bpu == 4 || bpu == 8 );
  assert (((arrayGetNElems(srcA) * arrayGetElemSz(srcA)) % nBytesPerUnpackedStrip) == 0);
  /* Number of strips needed:
   *              unit          strip      8 bits
   * X units    ----------  * -------  *  -----   = 0 strips
   *            1|2|4 bits   32 units     byte        
   */
  // From image's perspective, srcA is the colormap. 
  // Assumes 8 bits per unpacked unit.
  // maximum possible number of strips 
  size_t nStripsInOrigData = (arrayGetNElems(srcA) * arrayGetElemSz(srcA)) 
    / nBytesPerUnpackedStrip;  
  if (verbose) {
    printf("[stripNew] Dividing source data of %d bytes into a MAXIMUM of %ld %ld-byte strips\n", 
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

  // Allocate output stripmap and stripset
  StripDataS *sdP = memAdd(sizeof(StripDataS), TEMPORARY );
  memset(sdP, 0, sizeof(StripDataS));
  U8* ssUnpackedA = arrayNew( sizeof(U8), nStripsInOrigData * nBytesPerUnpackedStrip, TEMPORARY);
  memset(ssUnpackedA, 0, arrayGetNElems( ssUnpackedA ) * arrayGetElemSz( ssUnpackedA ) );
  // Strip map
  StripmapElem* smDataA = arrayNew( sizeof(StripmapElem), nStripsInOrigData, TEMPORARY);
  sdP->flags = flags;
  if (verbose) {
    printf("Analyzing viability of breaking image into %ld strips of %ld units each...\n", nStripsInOrigData, nBytesPerUnpackedStrip);
  }

  // Find distinct strips and write an index-mapping to them.
  U8 *currStripP;
  U8 *nextStripP;  
  for (size_t i = 0; i < nStripsInOrigData; ++i) {
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
    printf("%d distinct strips out of a maximum possible %ld strips\n", stripLabel, nStripsInOrigData);
  }
#if DEBUG_
  if (verbose) {
    _validateStripmapping(ssUnpackedA, nBytesPerUnpackedStrip, smDataA, srcA, verbose);
  }
#endif
  size_t nUnitsInStripset = stripLabel * nBytesPerUnpackedStrip;
  // If it's worth doing, store compressed stuff into colormap profile.
  // Compress data into an inflatable.
  size_t* ssPackedA = _packBits(ssUnpackedA, nUnitsInStripset, bpu, verbose);
#if DEBUG_
  if (verbose) {
    _validateBitPacking(ssPackedA, ssUnpackedA, nUnitsInStripset, bpu, verbose);
  }
#endif

  // One last thing: Pack the raw, non-stripped data to see if it can contend with the strip/zips.
  size_t* packedRawA =  _packBits(srcA, arrayGetNElems(srcA), bpu, verbose );
#if DEBUG_
  if (verbose) {
    _validateBitPacking(packedRawA, srcA, nUnitsInStripset, bpu, verbose);
  }
#endif
  // Oh, and also compress the raw as well so we can compare that too.
  Inflatable* rawInfP = inflatableNew( srcA, TEMPORARY);
  // OH YEAH, annnnd compress the *packed* raw too.
  Inflatable *packedInfP =  inflatableNew( packedRawA, TEMPORARY );
  // Go ahead and act like you're going to use stripset and stripmap for now.
  // We'll see if we really want them after we compare their sizes to the other compresion methods.
  // Stripset
  sdP->ss.nUnits = nUnitsInStripset;
  sdP->ss.nUnitsPerStrip = nBytesPerUnpackedStrip;
  sdP->ss.bpu = bpu;
  sdP->ss.infP = inflatableNew((void*) ssPackedA, TEMPORARY);
  if (verbose) {
    printf("[stripNew] SS Compressed size: %ld bytes\n", 
        sdP->ss.infP->compressedLen);
  }
  // Stripmap
  sdP->sm.nIndices = nStripsInOrigData;
  sdP->sm.infP = inflatableNew((void*) smDataA, TEMPORARY );
  // First, figure out what the best compression method to use is based on our results.
  /*
   * 5 contenders;
   * =============
   *   packed
   *   zipped
   *   packed + zipped
   *   packed + stripped
   *   packed + stripped + zipped
   *
   * Since all three de-compression methods are so fast, I vote we just go for the smallest one no matter what. 
   * Nothng fancy. */

  // Figure out the smallest compression.
  size_t packedSz = arrayGetElemSz(packedRawA) * arrayGetNElems(packedRawA);
  size_t zippedSz = rawInfP->compressedLen + sizeof(Inflatable);
  size_t pakZipSz = packedInfP->compressedLen + sizeof(Inflatable);
  size_t pkStrpSz = sdP->ss.infP->inflatedLen   + sdP->sm.infP->inflatedLen   + 2 * sizeof(Inflatable);
  size_t pkSpZpSz = sdP->ss.infP->compressedLen + sdP->sm.infP->compressedLen + 2 * sizeof(Inflatable);

  if (verbose) {
    printf("\e[33mraw: %d bytes\n", arrayGetNElems(srcA) * arrayGetElemSz(srcA));
    printf("\e[33mpacked: %ld bytes\n", packedSz);
    printf("zipped: %ld bytes\n", zippedSz);
    printf("packed & zipped: %ld bytes\n", pakZipSz);
    printf("packed & stripped: %ld bytes\n", pkStrpSz);
    printf("packed & stripped & zipped: %ld bytes\e[0m\n", pkSpZpSz);
  }

  size_t smallestSz = ( packedSz < zippedSz ) ? packedSz : zippedSz;
  if ( smallestSz > pakZipSz ) smallestSz = pakZipSz;
  if ( smallestSz > pkStrpSz ) smallestSz = pkStrpSz;
  if ( smallestSz > pkSpZpSz ) smallestSz = pkSpZpSz;

  // if using only packed data...
  if ( smallestSz == packedSz ) {
    assert( sdP->ss.infP != NULL );
    //free( sdP->ss.infP->compressedDataA );  // this is a raw array.. sigh
    sdP->ss.nUnits = arrayGetNElems( srcA );
    sdP->ss.infP->compressedDataA = memAdd( arrayGetElemSz( packedRawA ) * arrayGetNElems( packedRawA ), TEMPORARY );
    memcpy( sdP->ss.infP->compressedDataA, packedRawA, arrayGetElemSz( packedRawA ) * arrayGetNElems( packedRawA ) );
    // Flags
    sdP->flags = SD_SKIP_INFLATION_ | SD_SKIP_ASSEMBLY_;
    if (verbose) {
      printf("[stripNew] best size is \e[92mpack-only\e[0m (skip inflation and  assembly).\n");
    }
  }
  // if using only zipped raw data...
  else if ( smallestSz == zippedSz ) {
    sdP->ss.bpu = bpu;
    sdP->ss.nUnits = arrayGetNElems( srcA );
    sdP->ss.infP = rawInfP;
    rawInfP = NULL;
    // Stripmap
    memset( &sdP->sm, 0, sizeof( Stripmap ) );
    // Flags
    sdP->flags = SD_SKIP_UNPACKING_ | SD_SKIP_ASSEMBLY_;
    if (verbose) {
      printf("[stripNew] best size is \e[92mzip-only\e[0m (skip unpacking and  assembly).\n");
    }
  }
  // if using zipped-up packed data...
  else if ( smallestSz == pakZipSz ) {
    // Stripset
    sdP->ss.bpu = bpu;
    sdP->ss.nUnits = arrayGetNElems( srcA );
    sdP->ss.infP = packedInfP;
    packedInfP = NULL;
    // Stripmap
    memset( &sdP->sm, 0, sizeof( Stripmap ) );
    // Flags
    sdP->flags = SD_SKIP_ASSEMBLY_;
    if (verbose) {
      printf("[stripNew] best size is \e[92mzip-and-pack\e[0m (skip assembly).\n");
    }
    // Clean the rest of the stuff
  }
  // if using stripped packed data...
  else if ( smallestSz == pkStrpSz ) {
    // Populate uncompressed (yet packed) stripset.
    // Allocate stripset.
    sdP->ss.infP->compressedDataA = memAdd(  arrayGetElemSz( ssPackedA ) * arrayGetNElems( ssPackedA ), TEMPORARY );
    // Allocate stripmap.
    sdP->sm.infP->compressedDataA = memAdd(  sizeof(StripmapElem) * sdP->sm.nIndices, TEMPORARY);
    // Copy into stripset.
    memcpy( sdP->ss.infP->compressedDataA, ssPackedA, arrayGetElemSz( ssPackedA ) * arrayGetNElems( ssPackedA ) );
    // Copy into stripmap.
    memcpy( sdP->sm.infP->compressedDataA, smDataA, arrayGetElemSz( smDataA ) * arrayGetNElems( smDataA ) );
    // Keep the rest of the data as-is.
    // Flags
    sdP->flags = SD_SKIP_INFLATION_;
    // if using zipped-up stripped & packed data...
    if (verbose) {
      printf("[stripNew] best size is \e[92mpack-and-assembly\e[0m (skip zip).\n");
    }
  }
  else if ( smallestSz == pkSpZpSz ) {
    // No need to do anything. We have the inflatables already here.
    // Keep the rest of the data as-is.
    // Flags
    sdP->flags = 0;
    if (verbose) {
      printf("[stripNew] best size is \e[92mpack-and-assembly-and-zip\e[0m.\n");
    }
  }
  else {
    assert(0); // well, it's gotta compress SOMEhow!
  }
  // TODO free everything allocated in the new lines.
#if DEBUG_
  if (verbose) {
    //_validateInflatables(sdP, smDataA, ssPackedA);
  }
#endif
  stripIni(sdP, TEMPORARY);  // Make sure it's fully populated after this point.
#if DEBUG_
  if ( verbose && !( sdP->flags & SD_SKIP_ASSEMBLY_ ) ) {
    _validateAssembledData(sdP, srcA, verbose);
    // Why is sdP not assembled at this point?
    assert( sdP->assembledDataA );
  }
#endif

  return sdP;
}  // stripNew()

void writeStripDataInFile(FILE *fP, U8 verbose, char *objNameA, StripDataS *sdP) {
  assert (fP && sdP && objNameA);

  const int smInfNameLen = strlen(objNameA) + strlen("Stripmap") + strlen("Inf") + 1;
  char* smInfName = memAdd(sizeof(char) * smInfNameLen, TEMPORARY);
  memset(smInfName, 0, smInfNameLen);
  strcpy(smInfName, objNameA);
  strcat(smInfName, "Stripmap");
  strcat(smInfName, "Inf");
  const int ssInfNameLen = strlen(objNameA) + strlen("Stripset") + strlen("Inf") + 1;
  char* ssInfName = memAdd(sizeof(char) * ssInfNameLen, TEMPORARY);
  memset(ssInfName, 0, strlen(ssInfName));
  strcpy(ssInfName, objNameA);
  strcat(ssInfName, "Stripset");
  strcat(ssInfName, "Inf");

  // TODO: figure out all the cases things will vary by in here.
  //       -- NO STRIPMAPS --
  //       SD_SKIP_INFLATION_ | SD_SKIP_ASSEMBLY_  -> ss.infP->compressedDataA  ( packing only )
  //       SD_SKIP_UNPACKING_ | SD_SKIP_ASSEMBLY_  -> ss.infP  ( zipping only )
  //       SD_SKIP_ASSEMBLY_ -> ss.infP ( packing + zipping )
  //
  //       -- WITH STRIPMAPS --
  //       
  //       

  // Stripset inflatable
  inflatableAppend(sdP->ss.infP, fP, ssInfName);
  if (!( sdP->flags & SD_SKIP_ASSEMBLY_ ) ) {
    // Stripmap inflatable
    inflatableAppend(sdP->sm.infP, fP, smInfName);
  }
  //Stripmap
  fprintf(fP, "\n\n");
  fprintf(fP, "StripDataS %sStripData = {\n", objNameA);
  // Flags
  fprintf(fP, "\t.flags = %d,\n", sdP->flags);
  // Stripmap
  fprintf(fP, "\t.sm = {\n");
  fprintf(fP, "\t\t.nIndices = %d,\n", sdP->sm.nIndices);
  if (!( sdP->flags & SD_SKIP_ASSEMBLY_ ) ) {
    fprintf(fP, "\t\t.infP = &%s\n", smInfName);
  }
  else {
    fprintf(fP, "\t\t.infP = NULL\n");
  }
  fprintf(fP, "\t},\n");
  // Stripset
  fprintf(fP, "\t.ss = {\n");
  fprintf(fP, "\t\t.nUnitsPerStrip = %d,\n", sdP->ss.nUnitsPerStrip);  // in case we ever want to give each sprite its own strip length... wuh oh.
  fprintf(fP, "\t\t.nUnits  = %d,\n", sdP->ss.nUnits);
  fprintf(fP, "\t\t.bpu  = %d,\n", sdP->ss.bpu);
  fprintf(fP, "\t\t.infP = &%s,\n", ssInfName);
  fprintf(fP, "\t},\n");
  // Assembled data
  fprintf(fP, "\t.assembledDataA = NULL\n");
  fprintf(fP, "};\n\n");

}  // writeStripDataInFile
