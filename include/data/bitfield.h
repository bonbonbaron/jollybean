// TODO make volatile bitfields whose prefix sums change too frequently for updates to be practical.
#ifndef BITFIELD_H
#define BITFIELD_H
#include "data/mem.h"
#include "data/array.h"

#define LOCAL_BIT_MASK (0x1f)
#define N_BITS_PER_INT (32)
#define globalBitToBfIdx_(bitIdx) (bitIdx >> 5)  // ">> 5" is the same as "/ 32"
#define globalBitIdxToLocalBit_(bitIdx) (1 << (bitIdx & LOCAL_BIT_MASK))  // transforms a global bit (bit in an array of fields) to a local one (bit within its specific field)

typedef U32 VolatileBitfield; // This doesn't get a special array struct since it can have a raw array.

// Static bitfields are those which you set once, and then you *at least _almost_* never touch it again.
typedef struct StaticBitfield {
  U32 bits;
  U32 base;
} StaticBitfield;

typedef struct StaticBitfieldArray {
  StaticBitfield* bfA;
  U32 maxBitIdx;
  U32 population;
} StaticBitfieldArray;

StaticBitfield* bfNew( const PoolId poolId );
StaticBitfieldArray* bfaNew( const U32 nBits, const PoolId poolId );

inline StaticBitfield* bfaGetBitfield( const StaticBitfieldArray* bfaP, const U32 globalBitIdx ) {
  assert( bfaP );
  assert( bfaP->bfA );
  assert( globalBitIdx <= bfaP->maxBitIdx );
  assert( globalBitToBfIdx_( globalBitIdx ) < arrayGetNElems( bfaP->bfA ) );
  return &bfaP->bfA[ globalBitToBfIdx_( globalBitIdx ) ];
}

inline U32 bfaGetBits( const StaticBitfieldArray* bfaP, const U32 globalBitIdx ) {
  return bfaGetBitfield( bfaP, globalBitIdx )->bits;
}

inline void bfSetBit(StaticBitfield* bfP, const U32 bitIdx ) {
  assert( bitIdx < N_BITS_PER_INT );
  bfP->bits |= ( 1ul << bitIdx );
}

inline void bfUnsetBit(StaticBitfield* bfP, const U32 bitIdx ) {
  assert( bitIdx < N_BITS_PER_INT );
  bfP->bits &= ~( 1ul << bitIdx );
}

inline U32 bfIsBitSet( const StaticBitfield* bfP, const U32 bitIdx ) {
  assert( bitIdx < N_BITS_PER_INT );
  return bfP->bits & ( 1ul << bitIdx );
}

inline U32 bfaIsBitSet( const StaticBitfieldArray* bfaP, const U32 globalBitIdx ) {
  assert( bfaP );
  assert( bfaP->bfA );
  assert( globalBitIdx <= bfaP->maxBitIdx );
  assert( globalBitToBfIdx_( globalBitIdx ) < arrayGetNElems( bfaP->bfA ) );
  return bfaGetBitfield( bfaP, globalBitIdx )->bits & globalBitIdxToLocalBit_( globalBitIdx );
}

inline U32 bfaIsBitSetEx( const StaticBitfieldArray* bfaP,  const U32 globalBitIdx, StaticBitfield** bfPP) {
  assert( bfaP );
  assert( bfPP );
  assert( bfaP->bfA );
  assert( globalBitIdx <= bfaP->maxBitIdx );
  assert( globalBitToBfIdx_( globalBitIdx ) < arrayGetNElems( bfaP->bfA ) );
  *bfPP = &bfaP->bfA[ globalBitToBfIdx_( globalBitIdx ) ];
  return (*bfPP)->bits & globalBitIdxToLocalBit_( globalBitIdx );
}

inline U32 bfGetFirstZero( const U32 bits ) {
  return __builtin_ctz(~bits);
}

inline U32 bfGetFirstOne( const U32 bits ) {
  return __builtin_ctz(bits);
}

#define rawBitCount __builtin_popcount

// This allows flexible sums of the local and base fields.
inline U32 bfSum( const U32 localBits, const U32 base ) {
  return __builtin_popcount(localBits) + base;
}

inline void vbfSetBit( VolatileBitfield* vbfA, const U32 globalBitIdx ) {
  assert( vbfA );
  assert( globalBitToBfIdx_( globalBitIdx ) < arrayGetNElems( vbfA ) );
  assert( globalBitIdx < N_BITS_PER_INT * arrayGetNElems( vbfA ) ); 
  vbfA[ globalBitToBfIdx_( globalBitIdx ) ] |= globalBitIdxToLocalBit_( globalBitIdx );
}

inline void vbfUnsetBit( VolatileBitfield* vbfA, const U32 globalBitIdx ) {
  assert( vbfA );
  assert( globalBitToBfIdx_( globalBitIdx ) < arrayGetNElems( vbfA ) );
  assert( globalBitIdx < N_BITS_PER_INT * arrayGetNElems( vbfA ) ); 
  vbfA[ globalBitToBfIdx_( globalBitIdx ) ] &= ~globalBitIdxToLocalBit_( globalBitIdx );
}

void bfaSetBit(StaticBitfieldArray* bfaP, const U32 bitIdx );
void bfaUnsetBit(StaticBitfieldArray* bfaP, const U32 bitIdx );
StaticBitfieldArray* bfaClone( const StaticBitfieldArray* srcBfaP, const PoolId poolId );
S32 vbfGetFirstZero( const VolatileBitfield* vbfA );
S32 vbfGetFirstOne( const VolatileBitfield* vbfA );
#endif
