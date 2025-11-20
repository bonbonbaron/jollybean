#include "tau.h"
#include "data/bitfield.h"

struct Bitfields { 
  Bitfield *bf1P, *bf2P;
  BitfieldArray *bfa3P, *bfa4P;  // weird numbering to reflect # bits below
  U32 nBits1, nBits2, nBits3, nBits4;
};

static const int N_ELEMS = 100;

TAU_MAIN()

const U32 CORRECT_WORD_VAL =  (1 << 3) | (1 << 7) | (1 << 28);

TEST_F_SETUP(Bitfields) {
  tau->nBits1 = 10;
  tau->nBits2 = 20;
  tau->nBits3 = 255;
  tau->nBits4 = 127;
  tau->bf1P = bfNew( tau->nBits1, GENERAL );
  tau->bf2P = bfNew( tau->nBits2, GENERAL );
  tau->bfa3P = bfaNew( tau->nBits3, GENERAL );
  tau->bfa4P = bfaNew( tau->nBits4, GENERAL );

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
  // TODO guard against overflowing wiht maxBitIdx in bitfield.
  //      Tried to do 220 when bfIdx allowed it to only have 200 bits.
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
  // Bitfield from bitfield array 3
  Bitfield* bfPa = bfaGetBitfield( tau->bfa3P, 14 );
  CHECK_EQ( bfPa->bits, CORRECT_WORD_VAL );

  // Bitfield from bitfield array 4
  Bitfield* bfPb = bfaGetBitfield( tau->bfa4P, 40 );
  CHECK_EQ( bfPb->bits, CORRECT_WORD_VAL );
}

TEST_F(Bitfields, getBits) {
  // Bitfield from bitfield array 3
  U32 bits1 = bfaGetBits( tau->bfa3P, 14 );
  CHECK_EQ( bits1, CORRECT_WORD_VAL );

  // Bitfield from bitfield array 4
  U32 bits2 = bfaGetBits( tau->bfa4P, 14 );
  CHECK_EQ( bits2, CORRECT_WORD_VAL );
}
