#include "data/bitfield.h"

static_assert( sizeof(U32) == 4, "Woah, ints aren't 4 bytes. That'll break bitfield.c." );

Bitfield* bfNew( const U32 nBits, const PoolId poolId ) {
  return (Bitfield*) memAdd( sizeof( Bitfield ), poolId );
}

BitfieldArray* bfaNew( const U32 nBits, const PoolId poolId ) {
  BitfieldArray* bfaP = (BitfieldArray*) memAdd( sizeof( BitfieldArray ), poolId );
  bfaP->bfA = arrayNew( sizeof( Bitfield ), globalBitToBfIdx_( (nBits - 1) ) + 1, poolId );
  bfaP->maxBitIdx = nBits - 1;
  return bfaP;
}

void bfaSetBit(BitfieldArray* bfaP, const U32 bitIdx ) {
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

void bfaUnsetBit(BitfieldArray* bfaP, const U32 bitIdx ) {
  assert( bfaP );
  assert( bfaP->bfA );
  assert( bitIdx < N_BITS_PER_WORD * arrayGetNElems( bfaP->bfA ) );
  assert( bitIdx <= bfaP->maxBitIdx );
  assert( bfaP->population );
  // Actual logic
  U32 bfIdx = globalBitToBfIdx_( bitIdx );
  assert( bfIdx < arrayGetNElems( bfaP ) );
  bfaP->bfA[bfIdx].bits &= ~globalBitIdxToLocalBit_(bitIdx);
  // Increment the bases of all the bitfields above ours. 
  const U32 nBitfields = arrayGetNElems( bfaP->bfA );
  while (++bfIdx < nBitfields ) {
    --bfaP->bfA[bfIdx].base;
  }
  --bfaP->population;
}

void bfaCopy(const BitfieldArray* srcBfaP, BitfieldArray* dstBfaP ) {
  assert (srcBfaP);
  assert (dstBfaP);
  assert( arrayGetNElems( srcBfaP->bfA ) == arrayGetNElems( dstBfaP->bfA ) );
  assert( arrayGetElemSz( srcBfaP->bfA ) == arrayGetElemSz( dstBfaP->bfA ) );
  memcpy(dstBfaP->bfA, srcBfaP->bfA, arrayGetNElems( srcBfaP->bfA ) * arrayGetElemSz( srcBfaP->bfA ) );
}
