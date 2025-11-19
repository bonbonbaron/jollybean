// TODO make volatile bitfields whose prefix sums change too frequently for updates to be practical.
#ifndef BITFIELD_H
#define BITFIELD_H
#include "data/mem.h"
#include "data/array.h"

#define LOCAL_BIT_MASK (0x1f)
#define BITS_PER_INT (32)
#define globalBitToBfIdx_(bitIdx) (bitIdx >> 5)  // ">> 5" is the same as "/ 32"
#define globalBitIdxToLocalBit_(bitIdx) (1 << (bitIdx & LOCAL_BIT_MASK))  // transforms a global bit (bit in an array of fields) to a local one (bit within its specific field)

typedef struct Bitfield {
  U32 bits;
  U32 base;
} Bitfield;

typedef struct BitfieldArray {
  Bitfield* bfA;
  U32 maxBitIdx;
  U32 population;
} BitfieldArray;

Bitfield* bfNew( const U32 nBits, const PoolId poolId );
BitfieldArray* bfaNew( const U32 nBits, const PoolId poolId );

inline Bitfield* bfaGetBitfield( const BitfieldArray* bfaP, const U32 globalBitIdx ) {
  assert( bfaP );
  assert( bfaP->bfA );
  assert( globalBitIdx <= bfaP->maxBitIdx );
  assert( globalBitToBfIdx_( globalBitIdx ) < arrayGetNElems( bfaP->bfA ) );
  return &bfaP->bfA[ globalBitToBfIdx_( globalBitIdx ) ];
}

inline U32 bfaGetBits( const BitfieldArray* bfaP, const U32 globalBitIdx ) {
  return bfaGetBitfield( bfaP, globalBitIdx )->bits;
}

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

inline U32 bfaIsBitSet( const BitfieldArray* bfaP, const U32 globalBitIdx ) {
  assert( bfaP );
  assert( bfaP->bfA );
  assert( globalBitIdx <= bfaP->maxBitIdx );
  assert( globalBitToBfIdx_( globalBitIdx ) < arrayGetNElems( bfaP->bfA ) );
  return bfaP->bfA[ globalBitToBfIdx_( globalBitIdx ) ].bits & globalBitIdxToLocalBit_( globalBitIdx );
}

inline U32 bfaIsBitSetEx( const BitfieldArray* bfaP,  const U32 globalBitIdx, Bitfield** bfPP) {
  assert( bfaP );
  assert( bfPP );
  assert( bfaP->bfA );
  assert( globalBitIdx <= bfaP->maxBitIdx );
  assert( globalBitToBfIdx_( globalBitIdx ) < arrayGetNElems( bfaP->bfA ) );
  *bfPP = &bfaP->bfA[ globalBitToBfIdx_( globalBitIdx ) ];
  return (*bfPP)->bits & globalBitIdxToLocalBit_( globalBitIdx );
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

// This allows flexible sums of the local and base fields.
inline U32 bfSum( const U32 localBits, const U32 base ) {
  return __builtin_popcount(localBits) + base;
}


void bfaSetBit(BitfieldArray* bfaP, const U32 bitIdx );
void bfaUnsetBit(BitfieldArray* bfaP, const U32 bitIdx );
void bfaCopy(const BitfieldArray* srcBfaP, BitfieldArray* dstBfaP );

#endif
