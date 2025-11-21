#include "data/bitmap.h"

StaticBitmap* bmNew( const PoolId poolId ) {
  StaticBitmap* bmP = memAdd( sizeof( StaticBitmap ), poolId );
  bmP->bits = bmP->base = 0;
  return bmP;
}

StaticBitmapArray* bmaNew( const U32 nBits, const PoolId poolId ) {
  StaticBitmapArray* bmaP = (StaticBitmapArray*) memAdd( sizeof( StaticBitmapArray ), poolId );
  U32 nBitmaps = globalBitToBfIdx_( (nBits - 1) ) + 1;
  bmaP->bmA = arrayNew( sizeof( StaticBitmap ), nBitmaps, poolId );
  memset( bmaP->bmA, 0, sizeof( StaticBitmap ) * nBitmaps );
  bmaP->maxBitIdx = nBits - 1;
  bmaP->population = 0;
  return bmaP;
}

void bmaSetBit(StaticBitmapArray* bmaP, const U32 bitIdx ) {
  assert( bmaP );
  assert( bmaP->bmA );
  assert( bitIdx < N_BITS_PER_WORD * arrayGetNElems( bmaP->bmA ) );
  assert( bitIdx <= bmaP->maxBitIdx );
  // Actual logic
  U32 bmIdx = globalBitToBfIdx_( bitIdx );
  assert( bmIdx < arrayGetNElems( bmaP->bmA ) );
  bmaP->bmA[bmIdx].bits |= globalBitIdxToLocalBit_(bitIdx);
  // Increment the bases of all the bitfields above ours. 
  const U32 nBitmaps = arrayGetNElems( bmaP->bmA );
  while (++bmIdx < nBitmaps ) {
    ++bmaP->bmA[bmIdx].base;
  }
  ++bmaP->population;
  assert(bmaP->population <= ( bmaP->maxBitIdx + 1 ));
}

void bmaUnsetBit( StaticBitmapArray* bmaP, const U32 bitIdx ) {
  assert( bmaP );
  assert( bmaP->bmA );
  assert( bitIdx < N_BITS_PER_WORD * arrayGetNElems( bmaP->bmA ) );
  assert( bitIdx <= bmaP->maxBitIdx );
  assert( bmaP->population );
  // Actual logic
  U32 bmIdx = globalBitToBfIdx_( bitIdx );
  assert( bmIdx < arrayGetNElems( bmaP->bmA ) );
  bmaP->bmA[bmIdx].bits &= ~globalBitIdxToLocalBit_(bitIdx);
  // Increment the bases of all the bitfields above ours. 
  const U32 nBitmaps = arrayGetNElems( bmaP->bmA );
  while (++bmIdx < nBitmaps ) {
    --bmaP->bmA[bmIdx].base;
  }
  --bmaP->population;
}

// The reason we don't allocate the bitfield array for the developer is because they must 
// be responsible for which memory pool their destinaton BFA is in.
StaticBitmapArray* bmaClone( const StaticBitmapArray* srcBfaP, const PoolId poolId ) {
  assert (srcBfaP);
  assert (srcBfaP->bmA);
  StaticBitmapArray* newBfaP = bmaNew( srcBfaP->maxBitIdx, poolId );
  memcpy(newBfaP->bmA, srcBfaP->bmA, arrayGetNElems( srcBfaP->bmA ) * arrayGetElemSz( srcBfaP->bmA ) );
  return newBfaP;
}

#if __WORDSIZE == 32
#define SATURATED_WORD (0xffffffff)
#elif __WORDSIZE == 64
#define SATURATED_WORD (0xffffffffffffffff)
#else
static_assert(0, "Jollybean only supports 32- and 64-bit architectures.");
#endif

S32 vbmGetFirstZero( const VolatileBitmap* vbmA ) {
  VolatileBitmap* bmP = (VolatileBitmap*) vbmA;
  const VolatileBitmap* bmEndP = bmP + arrayGetNElems( vbmA );
  for( ; (const VolatileBitmap*) bmP < bmEndP; ++bmP ) {
    if ( *bmP == SATURATED_WORD ) {
      continue;
    }
    return bmGetFirstZero( *bmP ) + N_BITS_PER_WORD * ( bmP - vbmA );
  }
  return -1;
}

S32 vbmGetFirstOne( const VolatileBitmap* vbmA ) {
  VolatileBitmap* bmP = (VolatileBitmap*) vbmA;
  const VolatileBitmap* bmEndP = bmP + arrayGetNElems( vbmA );
  for( ; (const VolatileBitmap*) bmP < bmEndP; ++bmP ) {
    if ( *bmP == 0 ) {
      continue;
    }
    return bmGetFirstOne( *bmP ) + N_BITS_PER_WORD * ( bmP - vbmA );
  }
  return -1;
}
