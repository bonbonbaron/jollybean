#include "tau.h"
#include "data/bitfield.h"

struct Bitfields { 
  StaticBitfield *bf1P, *bf2P;
  StaticBitfieldArray *bfa3P, *bfa4P;  // weird numbering to reflect # bits below
  VolatileBitfield *vbfA;
  U32 nBits1, nBits2, nBits3, nBits4;
};

static const int N_ELEMS = 100;

TAU_MAIN()

const U32 BIT3 = 1 << 3;
const U32 BIT7 = 1 << 7;
const U32 BIT28 = 1 << 28;
const U32 CORRECT_WORD_VAL =  (1 << 3) | (1 << 7) | (1 << 28);

TEST_F_SETUP(Bitfields) {
  tau->nBits1 = 10;
  tau->nBits2 = 20;
  tau->nBits3 = 255;
  tau->nBits4 = 127;
  tau->bf1P = bfNew( GENERAL );
  tau->bf2P = bfNew( GENERAL );
  tau->bfa3P = bfaNew( tau->nBits3, GENERAL );
  tau->bfa4P = bfaNew( tau->nBits4, GENERAL );
  tau->vbfA = arrayNew( sizeof(U32), 10, GENERAL );
  tau->vbfA[0] = 0xffffffff;  // <-- first 1
  tau->vbfA[1] = 0xffffffff;
  tau->vbfA[2] = 0xfffffffe;  // <-- first 0 (at bit 64)
  tau->vbfA[3] = 0xffffffff;
  tau->vbfA[4] = 0xffffffff;
  tau->vbfA[5] = 0xffffffff;
  tau->vbfA[6] = 0xffffffff;
  tau->vbfA[7] = 0xffffffff;
  tau->vbfA[8] = 0xffffffff;
  tau->vbfA[9] = 0xffffffff;

  const U32 BITS_PER_BITFIELD = 32;
  // Iterate word by word.
  for ( int bfIdx = 0; bfIdx <= tau->nBits1 / (sizeof(U32) * 8); ++bfIdx ) {
    bfSetBit( tau->bf1P, BITS_PER_BITFIELD * bfIdx + 3 );
    bfSetBit( tau->bf1P, BITS_PER_BITFIELD * bfIdx + 7 );
    bfSetBit( tau->bf1P, BITS_PER_BITFIELD * bfIdx + 28 );
  }
  for ( int bfIdx = 0; bfIdx <= tau->nBits2 / (sizeof(U32) * 8); ++bfIdx ) {
    bfSetBit( tau->bf2P, BITS_PER_BITFIELD * bfIdx + 3 );
    bfSetBit( tau->bf2P, BITS_PER_BITFIELD * bfIdx + 7 );
    bfSetBit( tau->bf2P, BITS_PER_BITFIELD * bfIdx + 28 );
  }
  for ( int bfIdx = 0; bfIdx <= tau->nBits3 / (sizeof(U32) * 8); ++bfIdx ) {
    bfaSetBit( tau->bfa3P, BITS_PER_BITFIELD * bfIdx + 3 );
    bfaSetBit( tau->bfa3P, BITS_PER_BITFIELD * bfIdx + 7 );
    bfaSetBit( tau->bfa3P, BITS_PER_BITFIELD * bfIdx + 28 );
  }
  for ( int bfIdx = 0; bfIdx <= tau->nBits4 / (sizeof(U32) * 8); ++bfIdx ) {
    bfaSetBit( tau->bfa4P, BITS_PER_BITFIELD * bfIdx + 3 );
    bfaSetBit( tau->bfa4P, BITS_PER_BITFIELD * bfIdx + 7 );
    bfaSetBit( tau->bfa4P, BITS_PER_BITFIELD * bfIdx + 28 );
  }
}

TEST_F_TEARDOWN(Bitfields) {
  memRst( GENERAL );
}

TEST_F(Bitfields, getBitfield) {
  // StaticBitfield from bitfield array 3
  StaticBitfield* bfPa = bfaGetBitfield( tau->bfa3P, 14 );
  CHECK_EQ( bfPa->bits, CORRECT_WORD_VAL );

  // StaticBitfield from bitfield array 4
  StaticBitfield* bfPb = bfaGetBitfield( tau->bfa4P, 40 );
  CHECK_EQ( bfPb->bits, CORRECT_WORD_VAL );
}

TEST_F(Bitfields, getBits) {
  // StaticBitfield from bitfield array 3
  U32 bits1 = bfaGetBits( tau->bfa3P, 14 );
  CHECK_EQ( bits1, CORRECT_WORD_VAL );

  // StaticBitfield from bitfield array 4
  U32 bits2 = bfaGetBits( tau->bfa4P, 14 );
  CHECK_EQ( bits2, CORRECT_WORD_VAL );
}

TEST_F(Bitfields, setBitBothWays) {
  StaticBitfield* bfP = bfaGetBitfield( tau->bfa3P, 14 );
  bfSetBit( bfP, 1 );
  bfaSetBit( tau->bfa3P, 31 );
  const static U32 EXPECTED_ANSWER = CORRECT_WORD_VAL | ( 1 << 31 ) | (1 << 1);
  CHECK_EQ( bfaGetBits( tau->bfa3P, 0 ), EXPECTED_ANSWER );
  CHECK_EQ( bfP->bits, EXPECTED_ANSWER );
}

TEST_F(Bitfields, bfUnsetBit) {
  StaticBitfield* bfP = bfaGetBitfield( tau->bfa3P, 14 );
  bfUnsetBit( bfP, 7 );
  const static U32 EXPECTED_ANSWER = CORRECT_WORD_VAL & ~BIT7; // <-- this is correct
  CHECK_EQ( bfP->bits, EXPECTED_ANSWER );
  CHECK_EQ( bfaGetBits( tau->bfa3P, 14 ), EXPECTED_ANSWER );
}

TEST_F(Bitfields, bfIsBitSet ) {
  for ( int i = 0; i < 32; ++i ) {
    if ( i == 3 || i == 7 || i == 28 ) {
      CHECK_NE( bfIsBitSet( tau->bf1P, i ), 0 );
    }
    else {
      CHECK_EQ( bfIsBitSet( tau->bf1P, i ), 0 );
    }
  }
}

TEST_F(Bitfields, bfaIsBitSet ) {
  for ( int i = 0; i < 8; ++i ) {
    for (int j = 0; j < 32; ++j ) {
      // Because I don't want to do extra work:
      U32 expVal = 32 * i + j;
      if ( expVal >= 255 ) {
        goto leaveTestCase1;
      }
      if ( j == 3 || j == 7 || j == 28 ) {
        CHECK_NE( bfaIsBitSet( tau->bfa3P, expVal), 0 );
      }
      else {
        CHECK_EQ( bfaIsBitSet( tau->bfa3P, expVal), 0 );
      }
    }
  }
leaveTestCase1:
}

TEST_F(Bitfields, bfaIsBitSetEx ) {
  StaticBitfield* bfP;
  for ( int i = 0; i < 8; ++i ) {
    for (int j = 0; j < 32; ++j ) {
      // Because I don't want to do extra work:
      U32 expVal = 32 * i + j;
      if ( expVal >= 255 ) {
        goto leaveTestCase2;
      }
      if ( j == 3 || j == 7 || j == 28 ) {
        CHECK_NE( bfaIsBitSetEx( tau->bfa3P, expVal, &bfP ), 0 );
      }
      else {
        CHECK_EQ( bfaIsBitSetEx( tau->bfa3P, expVal, &bfP ), 0 );
      }
    }
  }
leaveTestCase2:
}

TEST_F(Bitfields, bfGetFirstZero) {
  CHECK_EQ( bfGetFirstZero( tau->bf1P->bits ), 0 );
  bfSetBit( tau->bf1P, 0 );
  CHECK_EQ( bfGetFirstZero( tau->bf1P->bits ), 1 );
}

TEST_F(Bitfields, bfGetFirstOne) {
  CHECK_EQ( bfGetFirstOne( tau->bf1P->bits ), 3 );
  bfUnsetBit( tau->bf1P, 3 );
  CHECK_EQ( bfGetFirstOne( tau->bf1P->bits ), 7 );
}

TEST_F(Bitfields, bfSum) {
  CHECK_EQ( bfSum( tau->bf1P->bits, tau->bf1P->base ), 3 );
  tau->bf1P->base = 100;
  CHECK_EQ( bfSum( tau->bf1P->bits, tau->bf1P->base ), 103 );
}

TEST_F(Bitfields, bfaUnsetBit) {
  CHECK_TRUE( bfaIsBitSet( tau->bfa3P, 35) );
  bfaUnsetBit( tau->bfa3P, 35 );
  CHECK_FALSE( bfaIsBitSet( tau->bfa3P, 35) );
}


TEST_F(Bitfields, bfaClone ) {
  StaticBitfieldArray* newBfaP = bfaClone( tau->bfa3P, GENERAL );
  for (int i = 0; i < arrayGetNElems( tau->bfa3P->bfA ); ++i ) {
    CHECK_EQ( newBfaP->bfA[i].bits, tau->bfa3P->bfA[i].bits );
    CHECK_EQ( newBfaP->bfA[i].base, tau->bfa3P->bfA[i].base );
  }
}

TEST_F(Bitfields, rawBitCount ) {
  for ( U32 bf = 0, i = 0; i < 32; ++i ) {
    CHECK_EQ( rawBitCount(bf), i );
    bf |= 1 << i;
    CHECK_EQ( rawBitCount(bf), i + 1);
  }
}

TEST_F(Bitfields, vbfSetBit) {
  CHECK_EQ( tau->vbfA[2], 0xfffffffe );
  vbfSetBit( tau->vbfA, 64 );
  CHECK_EQ( tau->vbfA[2], 0xffffffff );
}

TEST_F(Bitfields, vbfUnsetBit) {
  CHECK_EQ( tau->vbfA[7], 0xffffffff );
  vbfUnsetBit( tau->vbfA, 255 );
  CHECK_EQ( tau->vbfA[7], 0x7fffffff );
}

TEST_F(Bitfields, vbfGetFirstZero) {
  CHECK_EQ( vbfGetFirstZero( (const VolatileBitfield*) tau->vbfA ), 64);
  vbfSetBit( tau->vbfA, 64 );
  CHECK_EQ( vbfGetFirstZero( (const VolatileBitfield*) tau->vbfA ), -1);
}

TEST_F(Bitfields, vbfGetFirstOne) {
  CHECK_EQ( vbfGetFirstOne( (const VolatileBitfield*) tau->vbfA ), 0);
  tau->vbfA[0] = 0;
  CHECK_EQ( vbfGetFirstOne( (const VolatileBitfield*) tau->vbfA ), 32);
  for (int i = 0; i < 10; ++i ) {
    tau->vbfA[i] = 0;
  }
  CHECK_EQ( vbfGetFirstOne( (const VolatileBitfield*) tau->vbfA ), -1);
}
