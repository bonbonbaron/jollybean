#include "data/bitfield.h"
#include "data/array.h"

#define SATURATED_INT (0xffffffff)

static_assert( sizeof(U32) == 4, "Woah, ints aren't 4 bytes. That'll break bitfield.c." );

#define bfIdx_(bitIdx) (bitIdx >> 5)  // ">> 5" is the same as "/ 32"
#define bitFlag_(key) (1 << ((key - 1) & 0x07))

Bitfield* bfNew( const U32 nBits, const PoolId poolId ) {
  return (Bitfield*) memAdd( sizeof( Bitfield ), poolId );
}

Bitfield* bfArrayNew( const U32 nBits, const PoolId poolId ) {
  BitfieldArray* bfaP = (BitfieldArray*) memAdd( sizeof( BitfieldArray ), 1, poolId );
  bfaP->bfA = arrayNew( sizeof( Bitfield ), ( nBits >> 5 ) + 1, poolId );
  bfaP->maxBitIdx = nBits - 1;
  return bfaP;
}

void bfArraySetBit(Bitfield* bfP, const U32 bitIdx ) {
  assert( bfP );
  assert( bitIdx < N_BITS_PER_WORD * arrayGetNElems( bfP ) );
  U32 bfIdx = bfIdx_(key);
  assert( bfIdx < arrayGetNElems( bfP ) );
  bfP[bfIdx]flags |= bitFlag_(key);  /* flagNum & 0x07 gives you # of bits in the Nth byte */
  /* Increment all prevBitCounts in bytes above affected one. */
  const U32 nBitfields = arrayGetNElems( bfA );
  while (++byteIdx < N_FLAG_BYTES) {
    ++mapP->flagA[byteIdx].prevBitCount;
  }
}

void bfArrayUnsetBit(Bitfield* bfP, const U32 bitIdx ) {
  U32 byteIdx = byteIdx_(key);
  mapP->flagA[byteIdx].flags |= bitFlag_(key);  /* flagNum & 0x07 gives you # of bits in the Nth byte */
  /* Increment all prevBitCounts in bytes above affected one. */
  // TODO vectorize the below if it's available
  while (++byteIdx < N_FLAG_BYTES) {
    ++mapP->flagA[byteIdx].prevBitCount;
  }
}

U32 bfArrayIsBitSet( const Bitfield* bP, const bitIdx ) {

}
