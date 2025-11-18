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
  return (Bitfield*) arrayNew( sizeof( Bitfield ), nBits >> BITS_PER_INT, poolId );
}

void bfArraySetBit(Bitfield* bfP, const U32 bitIdx ) {
  U32 byteIdx = byteIdx_(key);
  mapP->flagA[byteIdx].flags |= bitFlag_(key);  /* flagNum & 0x07 gives you # of bits in the Nth byte */
  /* Increment all prevBitCounts in bytes above affected one. */
  // TODO vectorize the below if it's available
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
Bln bfArrayIsBitSet( const Bitfield* bP, const bitIdx ) {}
