#include "tau.h"
#include "data/bitfield.h"

struct Bitfields { 
  Bitfield *bf1P, *bf2P;
  BitfieldArray *bfa3P, *bfa4P;  // weird numbering to reflect # bits below
  U32 nBits1, nBits2, nBits3, nBits4;
};

static const int N_ELEMS = 100;

TAU_MAIN()

TEST_F_SETUP(Bitfields) {
  tau->nBits1 = 10;
  tau->nBits2 = 20;
  tau->nBits3 = 200;
  tau->nBits4 = 100;
  tau->bf1P = bfNew( tau->nBits1, GENERAL );
  tau->bf2P = bfNew( tau->nBits2, GENERAL );
  tau->bfa3P = bfaNew( tau->nBits3, GENERAL );
  tau->bfa4P = bfaNew( tau->nBits4, GENERAL );
}

TEST_F_TEARDOWN(Bitfields) {
  memRst( GENERAL );
}

TEST_F(Bitfields, listAppendWhenEmpty) {

}
