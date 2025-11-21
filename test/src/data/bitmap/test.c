#include "tau.h"
#include "data/bitmap.h"

static const UWord SATURATED_WORD = -1;

struct Bitmaps { 
  StableBitmap *bm1P, *bm2P;
  StableBitmapArray *bma3P, *bma4P;  // weird numbering to reflect # bits below
  VolatileBitmap *vbmA;
  UWord nBits1, nBits2, nBits3, nBits4;
};

static const int N_ELEMS = 100;

TAU_MAIN()

const UWord BIT3 = 1 << 3;
const UWord BIT7 = 1 << 7;
const UWord BIT28 = 1 << 28;
const UWord CORRECT_WORD_VAL =  BIT3 | BIT7 | BIT28;

TEST_F_SETUP(Bitmaps) {
  tau->nBits1 = 10;
  tau->nBits2 = 20;
  tau->nBits3 = 255;
  tau->nBits4 = 127;
  tau->bm1P = bmNew( GENERAL );
  tau->bm2P = bmNew( GENERAL );
  tau->bma3P = bmaNew( tau->nBits3, GENERAL );
  tau->bma4P = bmaNew( tau->nBits4, GENERAL );
  tau->vbmA = arrayNew( sizeof(UWord), 10, GENERAL );
  tau->vbmA[0] = SATURATED_WORD;  // <-- first 1
  tau->vbmA[1] = SATURATED_WORD;
  tau->vbmA[2] = SATURATED_WORD & ~1;  // <-- first 0 (at bit 64)
  tau->vbmA[3] = SATURATED_WORD;
  tau->vbmA[4] = SATURATED_WORD;
  tau->vbmA[5] = SATURATED_WORD;
  tau->vbmA[6] = SATURATED_WORD;
  tau->vbmA[7] = SATURATED_WORD;
  tau->vbmA[8] = SATURATED_WORD;
  tau->vbmA[9] = SATURATED_WORD;

  const UWord BITS_PER_BITFIELD = __WORDSIZE;
  // Iterate word by word.
  for ( int bmIdx = 0; bmIdx <= tau->nBits1 / (sizeof(UWord) * 8); ++bmIdx ) {
    bmSetBit( tau->bm1P, BITS_PER_BITFIELD * bmIdx + 3 );
    bmSetBit( tau->bm1P, BITS_PER_BITFIELD * bmIdx + 7 );
    bmSetBit( tau->bm1P, BITS_PER_BITFIELD * bmIdx + 28 );
  }
  for ( int bmIdx = 0; bmIdx <= tau->nBits2 / (sizeof(UWord) * 8); ++bmIdx ) {
    bmSetBit( tau->bm2P, BITS_PER_BITFIELD * bmIdx + 3 );
    bmSetBit( tau->bm2P, BITS_PER_BITFIELD * bmIdx + 7 );
    bmSetBit( tau->bm2P, BITS_PER_BITFIELD * bmIdx + 28 );
  }
  for ( int bmIdx = 0; bmIdx <= tau->nBits3 / (sizeof(UWord) * 8); ++bmIdx ) {
    bmaSetBit( tau->bma3P, BITS_PER_BITFIELD * bmIdx + 3 );
    bmaSetBit( tau->bma3P, BITS_PER_BITFIELD * bmIdx + 7 );
    bmaSetBit( tau->bma3P, BITS_PER_BITFIELD * bmIdx + 28 );
  }
  for ( int bmIdx = 0; bmIdx <= tau->nBits4 / (sizeof(UWord) * 8); ++bmIdx ) {
    bmaSetBit( tau->bma4P, BITS_PER_BITFIELD * bmIdx + 3 );
    bmaSetBit( tau->bma4P, BITS_PER_BITFIELD * bmIdx + 7 );
    bmaSetBit( tau->bma4P, BITS_PER_BITFIELD * bmIdx + 28 );
  }
}

TEST_F_TEARDOWN(Bitmaps) {
  memRst( GENERAL );
}

TEST_F(Bitmaps, getBitmap) {
  // StableBitmap from bitmap array 3
  StableBitmap* bmPa = bmaGetBitmap( tau->bma3P, 14 );
  CHECK_EQ( bmPa->bits, CORRECT_WORD_VAL );

  // StableBitmap from bitmap array 4
  StableBitmap* bmPb = bmaGetBitmap( tau->bma4P, 40 );
  CHECK_EQ( bmPb->bits, CORRECT_WORD_VAL );
}

TEST_F(Bitmaps, getBits) {
  // StableBitmap from bitmap array 3
  UWord bits1 = bmaGetBits( tau->bma3P, 14 );
  CHECK_EQ( bits1, CORRECT_WORD_VAL );

  // StableBitmap from bitmap array 4
  UWord bits2 = bmaGetBits( tau->bma4P, 14 );
  CHECK_EQ( bits2, CORRECT_WORD_VAL );
}

TEST_F(Bitmaps, setBitBothWays) {
  StableBitmap* bmP = bmaGetBitmap( tau->bma3P, 14 );
  bmSetBit( bmP, 1 );
  bmaSetBit( tau->bma3P, 31 );
  const static UWord EXPECTED_ANSWER = CORRECT_WORD_VAL | ( 1 << 31 ) | (1 << 1);
  CHECK_EQ( bmaGetBits( tau->bma3P, 0 ), EXPECTED_ANSWER );
  CHECK_EQ( bmP->bits, EXPECTED_ANSWER );
}

TEST_F(Bitmaps, bmUnsetBit) {
  StableBitmap* bmP = bmaGetBitmap( tau->bma3P, 14 );
  bmUnsetBit( bmP, 7 );
  const static UWord EXPECTED_ANSWER = CORRECT_WORD_VAL & ~BIT7; // <-- this is correct
  CHECK_EQ( bmP->bits, EXPECTED_ANSWER );
  CHECK_EQ( bmaGetBits( tau->bma3P, 14 ), EXPECTED_ANSWER );
}

TEST_F(Bitmaps, bmIsBitSet ) {
  for ( int i = 0; i < 32; ++i ) {
    if ( i == 3 || i == 7 || i == 28 ) {
      CHECK_NE( bmIsBitSet( tau->bm1P, i ), 0 );
    }
    else {
      CHECK_EQ( bmIsBitSet( tau->bm1P, i ), 0 );
    }
  }
}

TEST_F(Bitmaps, bmaIsBitSet ) {
  for ( int i = 0; i < 8; ++i ) {
    for (int j = 0; j < 32; ++j ) {
      // Because I don't want to do extra work:
      UWord expVal = 32 * i + j;
      if ( expVal >= 255 ) {
        goto leaveTestCase1;
      }
      if ( j == 3 || j == 7 || j == 28 ) {
        CHECK_NE( bmaIsBitSet( tau->bma3P, expVal), 0 );
      }
      else {
        CHECK_EQ( bmaIsBitSet( tau->bma3P, expVal), 0 );
      }
    }
  }
leaveTestCase1:
}

TEST_F(Bitmaps, bmaIsBitSetEx ) {
  StableBitmap* bmP;
  for ( int i = 0; i < 8; ++i ) {
    for (int j = 0; j < 32; ++j ) {
      // Because I don't want to do extra work:
      UWord expVal = 32 * i + j;
      if ( expVal >= 255 ) {
        goto leaveTestCase2;
      }
      if ( j == 3 || j == 7 || j == 28 ) {
        CHECK_NE( bmaIsBitSetEx( tau->bma3P, expVal, &bmP ), 0 );
      }
      else {
        CHECK_EQ( bmaIsBitSetEx( tau->bma3P, expVal, &bmP ), 0 );
      }
    }
  }
leaveTestCase2:
}

TEST_F(Bitmaps, bmGetFirstZero) {
  CHECK_EQ( bmGetFirstZero( tau->bm1P->bits ), 0 );
  bmSetBit( tau->bm1P, 0 );
  CHECK_EQ( bmGetFirstZero( tau->bm1P->bits ), 1 );
}

TEST_F(Bitmaps, bmGetFirstOne) {
  CHECK_EQ( bmGetFirstOne( tau->bm1P->bits ), 3 );
  bmUnsetBit( tau->bm1P, 3 );
  CHECK_EQ( bmGetFirstOne( tau->bm1P->bits ), 7 );
}

TEST_F(Bitmaps, bmSum) {
  CHECK_EQ( bmSum( tau->bm1P->bits, tau->bm1P->base ), 3 );
  tau->bm1P->base = 100;
  CHECK_EQ( bmSum( tau->bm1P->bits, tau->bm1P->base ), 103 );
}

TEST_F(Bitmaps, bmaUnsetBit) {
  CHECK_TRUE( bmaIsBitSet( tau->bma3P, 35) );
  bmaUnsetBit( tau->bma3P, 35 );
  CHECK_FALSE( bmaIsBitSet( tau->bma3P, 35) );
}


TEST_F(Bitmaps, bmaClone ) {
  StableBitmapArray* newBfaP = bmaClone( tau->bma3P, GENERAL );
  for (int i = 0; i < arrayGetNElems( tau->bma3P->bmA ); ++i ) {
    CHECK_EQ( newBfaP->bmA[i].bits, tau->bma3P->bmA[i].bits );
    CHECK_EQ( newBfaP->bmA[i].base, tau->bma3P->bmA[i].base );
  }
}

TEST_F(Bitmaps, rawBitCount ) {
  for ( UWord bm = 0, i = 0; i < 31; ++i ) {
    CHECK_EQ( rawBitCount(bm), i );
    bm |= 1 << i;
    CHECK_EQ( rawBitCount(bm), i + 1);
  }
}

TEST_F(Bitmaps, vbmSetBit) {
  static const U32 BIT_NUMBER = 2 * __WORDSIZE;
  CHECK_EQ( tau->vbmA[2], SATURATED_WORD & ~(1ULL) );  // 0xf...fe  for both 32- and 64-bit
  vbmSetBit( tau->vbmA, BIT_NUMBER );
  CHECK_EQ( tau->vbmA[2], SATURATED_WORD );
}

TEST_F(Bitmaps, vbmUnsetBit) {
  CHECK_EQ( tau->vbmA[7], SATURATED_WORD );
  vbmUnsetBit( tau->vbmA, 7 * __WORDSIZE );
  CHECK_EQ( tau->vbmA[7], SATURATED_WORD - 1 );
}

TEST_F(Bitmaps, vbmGetFirstZero) {
  CHECK_EQ( vbmGetFirstZero( (const VolatileBitmap*) tau->vbmA ), 2 * __WORDSIZE);
  vbmSetBit( tau->vbmA, 2 * __WORDSIZE);
  CHECK_EQ( vbmGetFirstZero( (const VolatileBitmap*) tau->vbmA ), -1);
}

TEST_F(Bitmaps, vbmGetFirstOne) {
  CHECK_EQ( vbmGetFirstOne( (const VolatileBitmap*) tau->vbmA ), 0);
  tau->vbmA[0] = 0;
  CHECK_EQ( vbmGetFirstOne( (const VolatileBitmap*) tau->vbmA ), __WORDSIZE);
  for (int i = 0; i < 10; ++i ) {
    tau->vbmA[i] = 0;
  }
  CHECK_EQ( vbmGetFirstOne( (const VolatileBitmap*) tau->vbmA ), -1);
}
