// TODO make volatile bitmaps whose prefix sums change too frequently for updates to be practical.
#ifndef BITMAP_H
#define BITMAP_H
#include "data/mem.h"
#include "data/array.h"

#if __WORDSIZE == 32
#define globalBitToBfIdx_(bitIdx) (bitIdx >> 5)  // ">> 5" is the same as "/ 32"
#define LOCAL_BIT_MASK (0x1f)
#elif __WORDSIZE == 64
#define globalBitToBfIdx_(bitIdx) (bitIdx >> 6)  // ">> 5" is the same as "/ 32"
#define LOCAL_BIT_MASK (0x3f)
#else
  static_assert("Jollybean only supports 32- and 64-bit architectures.");
#endif

#define globalBitIdxToLocalBit_(bitIdx) (1 << (bitIdx & LOCAL_BIT_MASK))  // transforms a global bit (bit in an array of fields) to a local one (bit within its specific field)

typedef UWord VolatileBitmap; // This doesn't get a special array struct since it can have a raw array.

// Static bitmaps are those which you set once, and then you *at least _almost_* never touch it again.
typedef struct StableBitmap {
  UWord bits;
  UWord base;
} StableBitmap;

typedef struct StableBitmapArray {
  StableBitmap* bmA;
  U32 maxBitIdx;
  U32 population;
} StableBitmapArray;

StableBitmap* bmNew( const PoolId poolId );
StableBitmapArray* bmaNew( const U32 nBits, const PoolId poolId );

inline StableBitmap* bmaGetBitmap( const StableBitmapArray* bmaP, const U32 globalBitIdx ) {
  assert( bmaP );
  assert( bmaP->bmA );
  assert( globalBitIdx <= bmaP->maxBitIdx );
  assert( globalBitToBfIdx_( globalBitIdx ) < arrayGetNElems( bmaP->bmA ) );
  return &bmaP->bmA[ globalBitToBfIdx_( globalBitIdx ) ];
}

inline UWord bmaGetBits( const StableBitmapArray* bmaP, const U32 globalBitIdx ) {
  return bmaGetBitmap( bmaP, globalBitIdx )->bits;
}

inline void bmSetBit(StableBitmap* bmP, const U32 bitIdx ) {
  assert( bitIdx < N_BITS_PER_WORD );
  bmP->bits |= ( 1ULL << bitIdx );
}

inline void bmUnsetBit(StableBitmap* bmP, const U32 bitIdx ) {
  assert( bitIdx < N_BITS_PER_WORD );
  bmP->bits &= ~( 1ULL << bitIdx );
}

inline U32 bmIsBitSet( const StableBitmap* bmP, const U32 bitIdx ) {
  assert( bitIdx < N_BITS_PER_WORD );
  return bmP->bits & ( 1ULL << bitIdx );
}

inline U32 bmaIsBitSet( const StableBitmapArray* bmaP, const U32 globalBitIdx ) {
  assert( bmaP );
  assert( bmaP->bmA );
  assert( globalBitIdx <= bmaP->maxBitIdx );
  assert( globalBitToBfIdx_( globalBitIdx ) < arrayGetNElems( bmaP->bmA ) );
  return bmaGetBitmap( bmaP, globalBitIdx )->bits & globalBitIdxToLocalBit_( globalBitIdx );
}

inline U32 bmaIsBitSetEx( const StableBitmapArray* bmaP,  const U32 globalBitIdx, StableBitmap** bmPP) {
  assert( bmaP );
  assert( bmPP );
  assert( bmaP->bmA );
  assert( globalBitIdx <= bmaP->maxBitIdx );
  assert( globalBitToBfIdx_( globalBitIdx ) < arrayGetNElems( bmaP->bmA ) );
  *bmPP = &bmaP->bmA[ globalBitToBfIdx_( globalBitIdx ) ];
  return (*bmPP)->bits & globalBitIdxToLocalBit_( globalBitIdx );
}

inline U32 bmGetFirstZero( const UWord bits ) {
#if __WORDSIZE == 32
  return __builtin_ctz(~bits);
#elif __WORDSIZE == 64
  return __builtin_ctzll(~bits);
#else 
  static_assert("Jollybean only supports 32- and 64-bit architectures.");
#endif
}

inline U32 bmGetFirstOne( const UWord bits ) {
#if __WORDSIZE == 32
  return __builtin_ctz(bits);
#elif __WORDSIZE == 64
  return __builtin_ctzll(bits);
#else 
  static_assert("Jollybean only supports 32- and 64-bit architectures.");
#endif
}

#if __WORDSIZE == 32
#define rawBitCount __builtin_popcount
#elif __WORDSIZE == 64
#define rawBitCount __builtin_popcountll
#else 
  static_assert("Jollybean only supports 32- and 64-bit architectures.");
#endif

// This allows flexible sums of the local and base fields.
inline U32 bmSum( const UWord bits, const U32 base ) {
  return rawBitCount(bits) + base;
}

// Volatile bitmaps
inline void vbmSetBit( VolatileBitmap* vbmA, const U32 globalBitIdx ) {
  assert( vbmA );
  assert( globalBitToBfIdx_( globalBitIdx ) < arrayGetNElems( vbmA ) );
  assert( globalBitIdx < N_BITS_PER_WORD * arrayGetNElems( vbmA ) ); 
  vbmA[ globalBitToBfIdx_( globalBitIdx ) ] |= globalBitIdxToLocalBit_( globalBitIdx );
}
inline void vbmUnsetBit( VolatileBitmap* vbmA, const U32 globalBitIdx ) {
  assert( vbmA );
  assert( globalBitToBfIdx_( globalBitIdx ) < arrayGetNElems( vbmA ) );
  assert( globalBitIdx < N_BITS_PER_WORD * arrayGetNElems( vbmA ) ); 
  vbmA[ globalBitToBfIdx_( globalBitIdx ) ] &= ~globalBitIdxToLocalBit_( globalBitIdx );
}
void bmaSetBit(StableBitmapArray* bmaP, const U32 bitIdx );
void bmaUnsetBit(StableBitmapArray* bmaP, const U32 bitIdx );
StableBitmapArray* bmaClone( const StableBitmapArray* srcBfaP, const PoolId poolId );
S32 vbmGetFirstZero( const VolatileBitmap* vbmA );
S32 vbmGetFirstOne( const VolatileBitmap* vbmA );
S32 vbmSetFirstZero( const VolatileBitmap* vbmA );
S32 vbmUnsetFirstOne( const VolatileBitmap* vbmA );
#endif
