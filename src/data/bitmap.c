#include "data/bitmap.h"

BasedWord* bmNew( const PoolId poolId ) {
  BasedWord* bmP = memAdd( sizeof( BasedWord ), poolId );
  bmP->bits = bmP->base = 0;
  return bmP;
}

StableBitmap* sbmNew( const U32 nBits, const PoolId poolId ) {
  StableBitmap* sbmP = (StableBitmap*) memAdd( sizeof( StableBitmap ), poolId );
  U32 nBitmaps = globalBitToBfIdx_( (nBits - 1) ) + 1;
  sbmP->bmA = arrayNew( sizeof( BasedWord ), nBitmaps, poolId );
  memset( sbmP->bmA, 0, sizeof( BasedWord ) * nBitmaps );
  sbmP->maxBitIdx = nBits - 1;
  sbmP->population = 0;
  return sbmP;
}

void sbmSetBit(StableBitmap* sbmP, const U32 bitIdx ) {
  assert( sbmP );
  assert( sbmP->bmA );
  assert( bitIdx < N_BITS_PER_WORD * arrayGetNElems( sbmP->bmA ) );
  assert( bitIdx <= sbmP->maxBitIdx );
  // Actual logic
  U32 bmIdx = globalBitToBfIdx_( bitIdx );
  assert( bmIdx < arrayGetNElems( sbmP->bmA ) );
  sbmP->bmA[bmIdx].bits |= globalBitIdxToLocalBit_(bitIdx);
  // Increment the bases of all the bitfields above ours. 
  const U32 nBitmaps = arrayGetNElems( sbmP->bmA );
  while (++bmIdx < nBitmaps ) {
    ++sbmP->bmA[bmIdx].base;
  }
  ++sbmP->population;
  assert(sbmP->population <= ( sbmP->maxBitIdx + 1 ));
}

void sbmUnsetBit( StableBitmap* sbmP, const U32 bitIdx ) {
  assert( sbmP );
  assert( sbmP->bmA );
  assert( bitIdx < N_BITS_PER_WORD * arrayGetNElems( sbmP->bmA ) );
  assert( bitIdx <= sbmP->maxBitIdx );
  assert( sbmP->population );
  // Actual logic
  U32 bmIdx = globalBitToBfIdx_( bitIdx );
  assert( bmIdx < arrayGetNElems( sbmP->bmA ) );
  sbmP->bmA[bmIdx].bits &= ~globalBitIdxToLocalBit_(bitIdx);
  // Increment the bases of all the bitfields above ours. 
  const U32 nBitmaps = arrayGetNElems( sbmP->bmA );
  while (++bmIdx < nBitmaps ) {
    --sbmP->bmA[bmIdx].base;
  }
  --sbmP->population;
}

// The reason we don't allocate the bitfield array for the developer is because they must 
// be responsible for which memory pool their destinaton SBM is in.
StableBitmap* sbmClone( const StableBitmap* srcSbmP, const PoolId poolId ) {
  assert (srcSbmP);
  assert (srcSbmP->bmA);
  StableBitmap* newSbmP = sbmNew( srcSbmP->maxBitIdx, poolId );
  memcpy(newSbmP->bmA, srcSbmP->bmA, arrayGetNElems( srcSbmP->bmA ) * arrayGetElemSz( srcSbmP->bmA ) );
  return newSbmP;
}

#if __WORDSIZE == 32
#define SATURATED_WORD (0xffffffff)
#elif __WORDSIZE == 64
#define SATURATED_WORD (0xffffffffffffffff)
#else
static_assert(0, "Jollybean only supports 32- and 64-bit architectures.");
#endif

S32 vbmGetFirstZero( const VolatileBitmap* vbmA ) {
  UWord* wordP = (UWord*) vbmA;
  const UWord* bmEndP = wordP + arrayGetNElems( vbmA );
  for( ; (const UWord*) wordP < bmEndP; ++wordP ) {
    if ( *wordP != SATURATED_WORD ) {
      return bmGetFirstZero( *wordP ) + N_BITS_PER_WORD * ( wordP - vbmA );
    }
  }
  return -1;
}

S32 vbmGetFirstOne( const UWord* vbmA ) {
  UWord* wordP = (UWord*) vbmA;
  const UWord* bmEndP = wordP + arrayGetNElems( vbmA );
  for( ; (const UWord*) wordP < bmEndP; ++wordP ) {
    if ( *wordP == 0 ) {
      continue;
    }
    return bmGetFirstOne( *wordP ) + N_BITS_PER_WORD * ( wordP - vbmA );
  }
  return -1;
}

S32 vbmSetFirstZero( const VolatileBitmap* vbmA ) {
  UWord* wordP = (UWord*) vbmA;
  const UWord* bmEndP = wordP + arrayGetNElems( vbmA );
  for( ; (const UWord*) wordP < bmEndP; ++wordP ) {
    if ( *wordP != SATURATED_WORD ) {
      U32 localBit = bmGetFirstZero( *wordP );
      bmSetBit( wordP, localBit );
      return localBit + N_BITS_PER_WORD * ( wordP - vbmA );
    }
  }
  return -1;
}

S32 vbmUnsetFirstOne( const VolatileBitmap* vbmA ) {
  UWord* wordP = (UWord*) vbmA;
  const UWord* bmEndP = wordP + arrayGetNElems( vbmA );
  for( ; (const UWord*) wordP < bmEndP; ++wordP ) {
    if ( *wordP != 0 ) {
      U32 localBit = bmGetFirstOne( *wordP );
      bmUnsetBit( wordP, localBit );
      return localBit + N_BITS_PER_WORD * ( wordP - vbmA );
    }
  }
  return -1;
}
