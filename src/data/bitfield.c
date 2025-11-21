#include "data/bitfield.h"

static_assert( sizeof(U32) == 4, "Woah, ints aren't 4 bytes. That'll break bitfield.c." );

StaticBitfield* bfNew( const PoolId poolId ) {
  StaticBitfield* bfP = memAdd( sizeof( StaticBitfield ), poolId );
  bfP->bits = bfP->base = 0;
  return bfP;
}

StaticBitfieldArray* bfaNew( const U32 nBits, const PoolId poolId ) {
  StaticBitfieldArray* bfaP = (StaticBitfieldArray*) memAdd( sizeof( StaticBitfieldArray ), poolId );
  U32 nBitfields = globalBitToBfIdx_( (nBits - 1) ) + 1;
  bfaP->bfA = arrayNew( sizeof( StaticBitfield ), nBitfields, poolId );
  memset( bfaP->bfA, 0, sizeof( StaticBitfield ) * nBitfields );
  bfaP->maxBitIdx = nBits - 1;
  bfaP->population = 0;
  return bfaP;
}

void bfaSetBit(StaticBitfieldArray* bfaP, const U32 bitIdx ) {
  assert( bfaP );
  assert( bfaP->bfA );
  assert( bitIdx < N_BITS_PER_WORD * arrayGetNElems( bfaP->bfA ) );
  assert( bitIdx <= bfaP->maxBitIdx );
  // Actual logic
  U32 bfIdx = globalBitToBfIdx_( bitIdx );
  assert( bfIdx < arrayGetNElems( bfaP->bfA ) );
  bfaP->bfA[bfIdx].bits |= globalBitIdxToLocalBit_(bitIdx);
  // Increment the bases of all the bitfields above ours. 
  const U32 nBitfields = arrayGetNElems( bfaP->bfA );
  while (++bfIdx < nBitfields ) {
    ++bfaP->bfA[bfIdx].base;
  }
  ++bfaP->population;
  assert(bfaP->population <= ( bfaP->maxBitIdx + 1 ));
}

void bfaUnsetBit( StaticBitfieldArray* bfaP, const U32 bitIdx ) {
  assert( bfaP );
  assert( bfaP->bfA );
  assert( bitIdx < N_BITS_PER_WORD * arrayGetNElems( bfaP->bfA ) );
  assert( bitIdx <= bfaP->maxBitIdx );
  assert( bfaP->population );
  // Actual logic
  U32 bfIdx = globalBitToBfIdx_( bitIdx );
  assert( bfIdx < arrayGetNElems( bfaP->bfA ) );
  bfaP->bfA[bfIdx].bits &= ~globalBitIdxToLocalBit_(bitIdx);
  // Increment the bases of all the bitfields above ours. 
  const U32 nBitfields = arrayGetNElems( bfaP->bfA );
  while (++bfIdx < nBitfields ) {
    --bfaP->bfA[bfIdx].base;
  }
  --bfaP->population;
}

// The reason we don't allocate the bitfield array for the developer is because they must 
// be responsible for which memory pool their destinaton BFA is in.
StaticBitfieldArray* bfaClone( const StaticBitfieldArray* srcBfaP, const PoolId poolId ) {
  assert (srcBfaP);
  assert (srcBfaP->bfA);
  StaticBitfieldArray* newBfaP = bfaNew( srcBfaP->maxBitIdx, poolId );
  memcpy(newBfaP->bfA, srcBfaP->bfA, arrayGetNElems( srcBfaP->bfA ) * arrayGetElemSz( srcBfaP->bfA ) );
  return newBfaP;
}

#define SATURATED_INT (0xffffffff)
S32 vbfGetFirstZero( const VolatileBitfield* vbfA ) {
  U32* bfP = (U32*) vbfA;
  const U32* bfEndP = bfP + arrayGetNElems( vbfA );
  for( ; (const U32*) bfP < bfEndP; ++bfP ) {
    if ( *bfP == SATURATED_INT ) {
      continue;
    }
    return bfGetFirstZero( *bfP ) + N_BITS_PER_INT * ( bfP - vbfA );
  }
  return -1;
}

S32 vbfGetFirstOne( const VolatileBitfield* vbfA ) {
  U32* bfP = (U32*) vbfA;
  const U32* bfEndP = bfP + arrayGetNElems( vbfA );
  for( ; (const U32*) bfP < bfEndP; ++bfP ) {
    if ( *bfP == 0 ) {
      continue;
    }
    return bfGetFirstOne( *bfP ) + N_BITS_PER_INT * ( bfP - vbfA );
  }
  return -1;
}
