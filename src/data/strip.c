#include "strip.h"

#if (__WORDSIZE == 32)
#define MASK_1BPP (0x01010101)
#define MASK_2BPP (0x03030303)
#define MASK_4BPP (0x0f0f0f0f)
#define MASK_8BPP (0xffffffff)
#define FULLY_PACKED_WORD_COUNT_RSHIFT (5)
#elif (__WORDSIZE == 64)
#define MASK_1BPP (0x0101010101010101)
#define MASK_2BPP (0x0303030303030303)
#define MASK_4BPP (0x0f0f0f0f0f0f0f0f)
#define MASK_8BPP (0xffffffffffffffff)
#define FULLY_PACKED_WORD_COUNT_RSHIFT (6)
#else
static_assert(0, "Only 32- and 64-bit systems are supported.");
#endif

#define assert_and_return_( x ) \
  assert( x ); \
  return x

U8* stripGetInput( StripDataS* sdP ) {
  switch( sdP->flags ) {
    // When this was never stripmapped, it's just a raw colormap.
    // However, the colormap may be sourced differently.
    case SD_SKIP_INFLATION_ | SD_SKIP_ASSEMBLY_:
      assert_and_return_( sdP->ss.infP->compressedDataA );
    case SD_SKIP_UNPACKING_ | SD_SKIP_ASSEMBLY_:
      assert_and_return_( sdP->ss.infP->compressedDataA );
    case SD_SKIP_ASSEMBLY_:
      assert_and_return_( sdP->ss.infP->compressedDataA );
    case SD_SKIP_INFLATION_:
      assert_and_return_( sdP->ss.infP->compressedDataA );
    default:  // skipping nothing
      assert_and_return_( sdP->ss.infP->compressedDataA );
  }
}

U8* stripGetOutput( StripDataS* sdP ) {
  switch( sdP->flags ) {
    // When this was never stripmapped, it's just a raw colormap.
    // However, the colormap may be sourced differently.
    case SD_SKIP_INFLATION_ | SD_SKIP_ASSEMBLY_:
      assert_and_return_( sdP->ss.unpackedDataA );
#if 0
    case SD_SKIP_UNPACKING_ | SD_SKIP_ASSEMBLY_:
      break;
    case SD_SKIP_ASSEMBLY_:
      break;
    case SD_SKIP_INFLATION_:
    default:  // skipping nothing
      fillRectFromStripmap( imgP, dstRectP, atlasPixelA, ATLAS_WIDTH );
      break;
#endif
  }
}

// Stripmapped Data
void stripClr(StripDataS *sdP) {
  if (sdP) {
    inflatableClr(sdP->ss.infP);
    inflatableClr(sdP->sm.infP);
    arrayDel((void**) &sdP->ss.unpackedDataA);
    arrayDel((void**) &sdP->assembledDataA);
  }
}

// This is for when I vectorize (again) in the future for the IMPROVED stripset unpacking algorithm.
#if 0
// TODO delete everything below once bug is fixed
// Stripped data inflation
#ifdef __ARM_NEON__
__inline__ static void _unpackStrip4Bpu(size_t **srcStripPP, size_t **dstStripPP) {
  size_t *srcStripP = *srcStripPP;
  size_t *dstStripP = *dstStripPP;
  /* 6 instructions neon VS 40-58 instructions regular */
  /* Although the outer loop appears unnecessary for 1 quadword per 1Bpu strip,
     it safeguards us from changes in the number of units per strip. */
  for (int i = 0; i < N_QUADWORDS_PER_4BPU_STRIP; ++i) {  /* keeping this useless loop here for when I chagne to 128-unit strips. */
    asm volatile inline (
    "vmov.u8 q10, #" #maskByte_ "nt"   /* q10 = mask */
    "vld1.32 {d0-d1}, [%0]!nt"    /* q0 (aka d0-d1) = packed indices */
    : "+r&" (srcStripP)
    );
    for (int j = 0; j < N_4BPU_UNITS_PER_BYTE; ++j) {
      asm volatile inline (
      "vand q1, q0, q10nt"          /* q1 = unpacked indices*/
      "vst1.32 {d2-d3}, [%0]!nt"
      "vshr.u8 q0, #" #Bpu_ "nt"            /* shift q0 over 1*/
      : "+r&" (dstStripP)
      );
    }
  }
}
#endif
#endif

void sdInflate(StripDataS *sdP) {
  if (sdP->flags & SD_SKIP_INFLATION_) {
    return;
  }
  inflatableIni(sdP->ss.infP);
  // Skipping assembly means no map is used.
  if (!(sdP->flags & SD_SKIP_ASSEMBLY_) )  {
    inflatableIni(sdP->sm.infP);
  }
}

// Unpack bits to reconstruct original data (uesd for debugging img.c)
void sdUnpack(StripDataS *sdP) {
  assert(sdP && (sdP->ss.infP && sdP->ss.infP->compressedDataA));
  if (sdP->flags & SD_SKIP_UNPACKING_) {
    return;
  }

  Stripset *ssP = &sdP->ss;
  // Packed data is all whole words. Unpacked may not be.
  // Only way to tell is by looking at the number of units.
  const size_t nPackedUnitsPerWord     = __WORDSIZE / ssP->bpu;  
  size_t nUnitsInExtraPackedWord = ssP->nUnits % nPackedUnitsPerWord;   
  const size_t nWhollyPackedWords      = ( ( ssP->nUnits * ssP->bpu ) >> FULLY_PACKED_WORD_COUNT_RSHIFT );

  // start copy
  ssP->unpackedDataA = arrayNew(sizeof(U8), ssP->nUnits); // 9 bytes allocated here

  const size_t mask = 
    (ssP->bpu == 1) ? MASK_1BPP :
    (ssP->bpu == 2) ? MASK_2BPP :
    (ssP->bpu == 4) ? MASK_4BPP : MASK_8BPP;

  assert( mask != MASK_8BPP );   // No point in unpacking if there's nothing to unpack.

// The offset is the number we're going to add to all units. This applies, for instance,
// if we're appending all color palettes to a global color palette and need to update
// the color palette indices in the colormap. (An old version of jollybean does that, but
// I eventually decided raw color data is better as it's faster for the GPU to process.)
#if (__WORDSIZE == 32)
  const size_t offset = (sdP->ss.offset << 24) |  
                        (sdP->ss.offset << 16) |
                        (sdP->ss.offset <<  8) |
                        (sdP->ss.offset      );
#elif (__WORDSIZE == 64)
  // You have to cast the U8 to a size_t in order for the compiler to
  // be okay with you left-shifting it by more than 31 bits.
  const size_t offset = ( sdP->ss.offset ) ?
      ((size_t) sdP->ss.offset << 56) |
      ((size_t) sdP->ss.offset << 48) |
      ((size_t) sdP->ss.offset << 40) |
      ((size_t) sdP->ss.offset << 32) |
      ((size_t) sdP->ss.offset << 24) |
      ((size_t) sdP->ss.offset << 16) |
      ((size_t) sdP->ss.offset <<  8) |
      ((size_t) sdP->ss.offset      ) : 0;
#else
    static_assert(0, "Only 32- and 64-bit systems are supported.");
#endif

  size_t *packedWordP;  
  // If the data was never compressed, then we're going to pull it from the "compressed" field.
  // It's just a safe place to put data that otherwise would get deleted by stripClr().
  if ( sdP->flags & SD_SKIP_INFLATION_ ) {
    packedWordP = (size_t*) ssP->infP->compressedDataA;  // storing here is a trick to avoid new data fields
  }
  else {  // But if it really IS inflated, pull the inflated data.
    packedWordP = (size_t*) ssP->infP->inflatedDataP;
  }
  const size_t* fullyPackedWordEndP = packedWordP + nWhollyPackedWords;
  size_t *dstUnpackedWordP = (size_t*) ssP->unpackedDataA;
  size_t j;
  // Unpack fully packed words.
  for (; packedWordP < fullyPackedWordEndP; ++packedWordP) {
    for (j = 0; j < N_BITS_PER_BYTE; j += ssP->bpu) {
      *(dstUnpackedWordP++) = ((*packedWordP >> j) & mask) + offset;
    }
  }
  // Unpack partly packed words with a whole mask.
  for (j = 0; 
       nUnitsInExtraPackedWord >= sizeof(size_t);   
       nUnitsInExtraPackedWord -= sizeof(size_t), j += ssP->bpu) {  
    *(dstUnpackedWordP++) = ((*packedWordP >> j) & mask) + offset;
  }
  // Unpack the remaining bytes of a partly packed word that don't fill a whole mask.
  if (nUnitsInExtraPackedWord > 0) {
    size_t lastUnpackedWord = ((*packedWordP >> j) & mask) + offset;
    memcpy((void*) dstUnpackedWordP, &lastUnpackedWord, nUnitsInExtraPackedWord);
  }
}  // sdUnpack() 

void sdAssemble(StripDataS *sdP) {
  assert (sdP );

  if ( sdP->flags & SD_SKIP_ASSEMBLY_) {
    return;
  }

  assert (sdP->ss.unpackedDataA && sdP->sm.infP->inflatedDataP && !sdP->assembledDataA
      && sdP->ss.nUnitsPerStrip > 0);

  sdP->assembledDataA = arrayNew( sizeof(U8), sdP->sm.nIndices * sdP->ss.nUnitsPerStrip);

  // Piece together strips
  // If the data was never compressed, then we're going to pull it from the "compressed" field.
  // It's just a safe place to put data that otherwise would get deleted by stripClr().
#if 0
  if ( sdP->flags & SD_SKIP_INFLATION_ ) {
    packedWordP = (size_t*) ssP->infP->compressedDataA;  // storing here is a trick to avoid new data fields
  }
  else {  // But if it really IS inflated, pull the inflated data.
    packedWordP = (size_t*) ssP->infP->inflatedDataP;
  }
#endif
  StripmapElem *smElemP = sdP->sm.infP->inflatedDataP;
  StripmapElem *smElemEndP = smElemP + sdP->sm.nIndices;
  U8 *assembledDataP = sdP->assembledDataA;
  for (; smElemP < smElemEndP; ++smElemP, assembledDataP += sdP->ss.nUnitsPerStrip) {
    memcpy(assembledDataP,
        sdP->ss.unpackedDataA + (*smElemP * sdP->ss.nUnitsPerStrip),
        sdP->ss.nUnitsPerStrip);
  }
}

// This is the single-threaded version of inflating stripd data.
void stripIni(StripDataS *sdP) {
  sdInflate(sdP);
  sdUnpack(sdP);
  sdAssemble(sdP);
}


