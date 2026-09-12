#include "tau.h"
#include "data/bitmap.h"

static const UWord SATURATED_WORD = -1;

struct Bitmaps { 
  BasedWord *basedWord1P, *basedWord2P;
  StableBitmap *sbm3P, *sbm4P;  // weird numbering to reflect # bits below
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
  tau->basedWord1P = bmNew( GENERAL );
  tau->basedWord2P = bmNew( GENERAL );
  tau->sbm3P = sbmNew( tau->nBits3, GENERAL );
  tau->sbm4P = sbmNew( tau->nBits4, GENERAL );
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
    wordSetBit( &tau->basedWord1P->bits, BITS_PER_BITFIELD * bmIdx + 3 );
    wordSetBit( &tau->basedWord1P->bits, BITS_PER_BITFIELD * bmIdx + 7 );
    wordSetBit( &tau->basedWord1P->bits, BITS_PER_BITFIELD * bmIdx + 28 );
  }
  for ( int bmIdx = 0; bmIdx <= tau->nBits2 / (sizeof(UWord) * 8); ++bmIdx ) {
    wordSetBit( &tau->basedWord2P->bits, BITS_PER_BITFIELD * bmIdx + 3 );
    wordSetBit( &tau->basedWord2P->bits, BITS_PER_BITFIELD * bmIdx + 7 );
    wordSetBit( &tau->basedWord2P->bits, BITS_PER_BITFIELD * bmIdx + 28 );
  }
  for ( int bmIdx = 0; bmIdx <= tau->nBits3 / (sizeof(UWord) * 8); ++bmIdx ) {
    sbmSetBit( tau->sbm3P, BITS_PER_BITFIELD * bmIdx + 3 );
    sbmSetBit( tau->sbm3P, BITS_PER_BITFIELD * bmIdx + 7 );
    sbmSetBit( tau->sbm3P, BITS_PER_BITFIELD * bmIdx + 28 );
  }
  for ( int bmIdx = 0; bmIdx <= tau->nBits4 / (sizeof(UWord) * 8); ++bmIdx ) {
    sbmSetBit( tau->sbm4P, BITS_PER_BITFIELD * bmIdx + 3 );
    sbmSetBit( tau->sbm4P, BITS_PER_BITFIELD * bmIdx + 7 );
    sbmSetBit( tau->sbm4P, BITS_PER_BITFIELD * bmIdx + 28 );
  }
}

TEST_F_TEARDOWN(Bitmaps) {
  memRst( GENERAL );
}

TEST_F(Bitmaps, getBitmap) {
  // BasedWord from bitmap array 3
  BasedWord* bmPa = sbmGetBasedWord( tau->sbm3P, 14 );
  CHECK_EQ( bmPa->bits, CORRECT_WORD_VAL );

  // BasedWord from bitmap array 4
  BasedWord* bmPb = sbmGetBasedWord( tau->sbm4P, 40 );
  CHECK_EQ( bmPb->bits, CORRECT_WORD_VAL );
}

TEST_F(Bitmaps, getBits) {
  // BasedWord from bitmap array 3
  UWord bits1 = sbmGetBits( tau->sbm3P, 14 );
  CHECK_EQ( bits1, CORRECT_WORD_VAL );

  // BasedWord from bitmap array 4
  UWord bits2 = sbmGetBits( tau->sbm4P, 14 );
  CHECK_EQ( bits2, CORRECT_WORD_VAL );
}

// This sets the bit with both the stable bit-setter and raw bit-setter.
TEST_F(Bitmaps, setBitBothWays) {
  const static UWord EXPECTED_ANSWER = CORRECT_WORD_VAL | ( 1ULL << 31 ) | (1ULL << 1);

  BasedWord* bwP = sbmGetBasedWord( tau->sbm3P, 14 );
  CHECK_EQ( bwP->bits, CORRECT_WORD_VAL );

  wordSetBit( &bwP->bits, 1 );
  sbmSetBit( tau->sbm3P, 31 );
  CHECK_EQ( sbmGetBits( tau->sbm3P, 0 ), EXPECTED_ANSWER );
  CHECK_EQ( bwP->bits, EXPECTED_ANSWER );
}

TEST_F(Bitmaps, wordUnsetBit) {
  BasedWord* bwP = sbmGetBasedWord( tau->sbm3P, 14 );
  wordUnsetBit( &bwP->bits, 7 );
  const static UWord EXPECTED_ANSWER = CORRECT_WORD_VAL & ~BIT7; // <-- this is correct
  CHECK_EQ( bwP->bits, EXPECTED_ANSWER );
  CHECK_EQ( sbmGetBits( tau->sbm3P, 14 ), EXPECTED_ANSWER );
}

TEST_F(Bitmaps, wordIsBitSet ) {
  for ( int i = 0; i < 32; ++i ) {
    if ( i == 3 || i == 7 || i == 28 ) {
      CHECK_NE( wordIsBitSet( &tau->basedWord1P->bits, i ), 0 );
    }
    else {
      CHECK_EQ( wordIsBitSet( &tau->basedWord1P->bits, i ), 0 );
    }
  }
}

TEST_F(Bitmaps, sbmIsBitSet ) {
  for ( int i = 0; i < 8; i += 2 ) {  // +=2, because i'm not setting bits 3, 7, and 28 for the higher 32 bits.
    for (int j = 0; j < 32; ++j ) {
      // Because I don't want to do extra work:
      UWord expVal = 32 * i + j;
      if ( expVal >= 255 ) {
        goto leaveTestCase1;
      }
      if ( j == 3 || j == 7 || j == 28 ) {
        CHECK_NE( sbmIsBitSet( tau->sbm3P, expVal), 0 );
      }
      else {
        CHECK_EQ( sbmIsBitSet( tau->sbm3P, expVal), 0 );
      }
    }
  }
leaveTestCase1:
}

TEST_F(Bitmaps, sbmIsBitSetEx ) {
  BasedWord* bwP;
  for ( int i = 0; i < 8; i +=2 ) {
    for (int j = 0; j < 32; ++j ) {
      // Because I don't want to do extra work:
      UWord expVal = 32 * i + j;
      if ( expVal >= 255 ) {
        goto leaveTestCase2;
      }
      if ( j == 3 || j == 7 || j == 28 ) {
        CHECK_NE( sbmIsBitSetEx( tau->sbm3P, expVal, &bwP ), 0 );
      }
      else {
        CHECK_EQ( sbmIsBitSetEx( tau->sbm3P, expVal, &bwP ), 0 );
      }
    }
  }
leaveTestCase2:
}

TEST_F(Bitmaps, bmGetFirstZero) {
  CHECK_EQ( bmGetFirstZero( tau->basedWord1P->bits ), 0 );
  wordSetBit( &tau->basedWord1P->bits, 0 );
  CHECK_EQ( bmGetFirstZero( tau->basedWord1P->bits ), 1 );
}

TEST_F(Bitmaps, bmGetFirstOne) {
  CHECK_EQ( bmGetFirstOne( tau->basedWord1P->bits ), 3 );
  wordUnsetBit( &tau->basedWord1P->bits, 3 );
  CHECK_EQ( bmGetFirstOne( tau->basedWord1P->bits ), 7 );
}

TEST_F(Bitmaps, bmSum) {
  CHECK_EQ( bmSum( tau->basedWord1P->bits, tau->basedWord1P->base ), 3 );
  tau->basedWord1P->base = 100;
  CHECK_EQ( bmSum( tau->basedWord1P->bits, tau->basedWord1P->base ), 103 );
}

TEST_F(Bitmaps, sbmUnsetBit) {
  CHECK_FALSE( sbmIsBitSet( tau->sbm3P, 35) ); // it's returning the opposite for 35.
  sbmSetBit( tau->sbm3P, 35 );
  CHECK_TRUE( sbmIsBitSet( tau->sbm3P, 35) );
  sbmUnsetBit( tau->sbm3P, 35 );
  CHECK_FALSE( sbmIsBitSet( tau->sbm3P, 35) );
}


TEST_F(Bitmaps, sbmClone ) {
  StableBitmap* newBfaP = sbmClone( tau->sbm3P, GENERAL );
  for (int i = 0; i < arrayGetNElems( tau->sbm3P->bmA ); ++i ) {
    CHECK_EQ( newBfaP->bmA[i].bits, tau->sbm3P->bmA[i].bits );
    CHECK_EQ( newBfaP->bmA[i].base, tau->sbm3P->bmA[i].base );
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
