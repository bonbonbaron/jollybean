#include "data/bitmap.h"

StableBitmap* bmNew( const PoolId poolId ) {
  StableBitmap* bmP = memAdd( sizeof( StableBitmap ), poolId );
  bmP->bits = bmP->base = 0;
  return bmP;
}

StableBitmapArray* bmaNew( const U32 nBits, const PoolId poolId ) {
  StableBitmapArray* bmaP = (StableBitmapArray*) memAdd( sizeof( StableBitmapArray ), poolId );
  U32 nBitmaps = globalBitToBfIdx_( (nBits - 1) ) + 1;
  bmaP->bmA = arrayNew( sizeof( StableBitmap ), nBitmaps, poolId );
  memset( bmaP->bmA, 0, sizeof( StableBitmap ) * nBitmaps );
  bmaP->maxBitIdx = nBits - 1;
  bmaP->population = 0;
  return bmaP;
}

void bmaSetBit(StableBitmapArray* bmaP, const U32 bitIdx ) {
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

void bmaUnsetBit( StableBitmapArray* bmaP, const U32 bitIdx ) {
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
StableBitmapArray* bmaClone( const StableBitmapArray* srcBfaP, const PoolId poolId ) {
  assert (srcBfaP);
  assert (srcBfaP->bmA);
  StableBitmapArray* newBfaP = bmaNew( srcBfaP->maxBitIdx, poolId );
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
    if ( *bmP != SATURATED_WORD ) {
      return bmGetFirstZero( *bmP ) + N_BITS_PER_WORD * ( bmP - vbmA );
    }
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

S32 vbmSetFirstZero( const VolatileBitmap* vbmA ) {
  VolatileBitmap* bmP = (VolatileBitmap*) vbmA;
  const VolatileBitmap* bmEndP = bmP + arrayGetNElems( vbmA );
  for( ; (const VolatileBitmap*) bmP < bmEndP; ++bmP ) {
    if ( *bmP != SATURATED_WORD ) {
      U32 localBit = bmGetFirstZero( *bmP );
      bmSetBit( bmP, localBit );
      return localBit + N_BITS_PER_WORD * ( bmP - vbmA );
    }
  }
  return -1;
}

S32 vbmUnsetFirstOne( const VolatileBitmap* vbmA ) {
  VolatileBitmap* bmP = (VolatileBitmap*) vbmA;
  const VolatileBitmap* bmEndP = bmP + arrayGetNElems( vbmA );
  for( ; (const VolatileBitmap*) bmP < bmEndP; ++bmP ) {
    if ( *bmP != 0 ) {
      U32 localBit = bmGetFirstOne( *bmP );
      bmUnsetBit( bmP, localBit );
      return localBit + N_BITS_PER_WORD * ( bmP - vbmA );
    }
  }
  return -1;
}
