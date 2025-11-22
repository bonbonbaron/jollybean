// TODO make volatile bitmaps whose prefix sums change too frequently for updates to be practical.
#ifndef BITMAP_H
#define BITMAP_H
#include "data/mem.h"
#include "data/array.h"

#if __WORDSIZE == 32
#define globalBitToBfIdx_(bitIdx) (bitIdx >> 5)  // ">> 5" is the same as "/ 32"
#define LOCAL_BIT_MASK (0x1f)
#elif __WORDSIZE == 64
#define globalBitToBfIdx_(bitIdx) (bitIdx >> 6)  // ">> 6" is the same as "/ 64"
#define LOCAL_BIT_MASK (0x3f)
#else
  static_assert("Jollybean only supports 32- and 64-bit architectures.");
#endif

#define globalBitIdxToLocalBit_(bitIdx) (1ULL << (bitIdx & LOCAL_BIT_MASK))  // transforms a global bit (bit in an array of fields) to a local one (bit within its specific field)

typedef UWord VolatileBitmap; // This doesn't get a special array struct since it can have a raw array.

// Static bitmaps are those which you set once, and then you *at least _almost_* never touch it again.
typedef struct BasedWord {
  UWord bits;
  UWord base;
} BasedWord;

typedef struct StableBitmap {
  BasedWord* bmA;
  U32 maxBitIdx;
  U32 population;
} StableBitmap;

BasedWord* bmNew( const PoolId poolId );
StableBitmap* sbmNew( const U32 nBits, const PoolId poolId );

inline BasedWord* sbmGetBasedWord( const StableBitmap* sbmP, const U32 globalBitIdx ) {
  assert( sbmP );
  assert( sbmP->bmA );
  assert( globalBitIdx <= sbmP->maxBitIdx );
  assert( globalBitToBfIdx_( globalBitIdx ) < arrayGetNElems( sbmP->bmA ) );
  return &sbmP->bmA[ globalBitToBfIdx_( globalBitIdx ) ];
}

inline UWord sbmGetBits( const StableBitmap* sbmP, const U32 globalBitIdx ) {
  return sbmGetBasedWord( sbmP, globalBitIdx )->bits;
}

inline void wordSetBit(UWord *wordP, const U32 bitIdx ) {
  assert( bitIdx < N_BITS_PER_WORD );
  *wordP |= ( 1ULL << bitIdx );
}

inline void wordUnsetBit(UWord *wordP, const U32 bitIdx ) {
  assert( bitIdx < N_BITS_PER_WORD );
  *wordP &= ~( 1ULL << bitIdx );
}

inline UWord wordIsBitSet( const UWord* wordP, const U32 bitIdx ) {
  assert( bitIdx < N_BITS_PER_WORD );
  return *wordP & ( 1ULL << bitIdx );
}

UWord sbmIsBitSetEx( const StableBitmap* sbmP,  const U32 globalBitIdx, BasedWord** basedWordPP);

inline UWord sbmIsBitSet( const StableBitmap* sbmP, const U32 globalBitIdx ) {
  assert( sbmP );
  assert( sbmP->bmA );
  assert( globalBitIdx <= sbmP->maxBitIdx );
  assert( globalBitToBfIdx_( globalBitIdx ) < arrayGetNElems( sbmP->bmA ) );
  return sbmGetBasedWord( sbmP, globalBitIdx )->bits & globalBitIdxToLocalBit_( globalBitIdx );
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
inline UWord bmSum( const UWord bits, const UWord base ) {
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
void sbmSetBit(StableBitmap* sbmP, const U32 bitIdx );
void sbmUnsetBit(StableBitmap* sbmP, const U32 bitIdx );
StableBitmap* sbmClone( const StableBitmap* srcBfaP, const PoolId poolId );
S32 vbmGetFirstZero( const VolatileBitmap* vbmA );
S32 vbmGetFirstOne( const VolatileBitmap* vbmA );
S32 vbmSetFirstZero( const VolatileBitmap* vbmA );
S32 vbmUnsetFirstOne( const VolatileBitmap* vbmA );
#endif
