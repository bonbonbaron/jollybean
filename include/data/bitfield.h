#ifndef BITFIELD_H
#define BITFIELD_H
#include "data/common.h"
#include <assert.h>

#define BITS_PER_INT (32)

typedef struct Bitfield {
  U32 bits;
  U32 base;
} Bitfield;

typedef struct BitfieldArray {
  Bitfield* bfA;
  U32 maxBitIdx;
} BitfieldArray;

inline void bfSetBit(Bitfield* bfP, const U32 bitIdx ) {
  assert( bitIdx < BITS_PER_INT );
  bfP->bits |= ( 1 << bitIdx );
}

inline void bfUnsetBit(Bitfield* bfP, const U32 bitIdx ) {
  assert( bitIdx < BITS_PER_INT );
  bfP->bits &= ~( 1 << bitIdx );
}

inline U32 bfIsBitSet( const Bitfield* bfP, const U32 bitIdx ) {
  assert( bitIdx < BITS_PER_INT );
  return bfP->bits & ( 1 << bitIdx );
}

inline U32 bfGetFirstZeroSingle( const Bitfield* bfP ) {
  return __builtin_ctz(~bfP->bits);
}

inline U32 bfGetFirstOneSingle( const Bitfield* bfP ) {
  return __builtin_ctz(bfP->bits);
}

inline U32 bfGetFirstZero( const Bitfield* bfP ) {
  return __builtin_ctz(~(bfP->bits)) + bfP->base;
}

inline U32 bfGetFirstOne( const Bitfield* bfP ) {
  return __builtin_ctz(bfP->bits) + bfP->base;
}

#endif
