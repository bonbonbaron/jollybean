#include "test.h"
#include "interface/interface.h"

extern Image redImg; 
extern Image blehImg; 
extern Image heckImg;

TAU_MAIN()

// In addition to the made-up arrays below,
//   load in the images we made for the SDL surface unit test.

const static U8 verbose = 0;

TEST_F_SETUP(Tau) {
  // Make arrays, because inflatableNew() expects Jollybean arrays.
  tau->raw1bppA = arrayNew(  sizeof(rawData1bpp[0]), sizeof(rawData1bpp) / sizeof(rawData1bpp[0]), GENERAL);
  memcpy((void*) tau->raw1bppA, (void*) rawData1bpp, sizeof(rawData1bpp[0]) * sizeof(rawData1bpp) / sizeof(rawData1bpp[0]));
  if (verbose) printf("\e[91m1bpp data\e[0m\n");
  tau->sd1bppP = stripNew(tau->raw1bppA, 3, 1, 0, verbose);

  tau->raw2bppA = arrayNew(  sizeof(rawData2bpp[0]), sizeof(rawData2bpp) / sizeof(rawData2bpp[0]), GENERAL);
  memcpy((void*) tau->raw2bppA, (void*) rawData2bpp, sizeof(rawData2bpp[0]) * sizeof(rawData2bpp) / sizeof(rawData2bpp[0]));
  if (verbose) printf("\e[91m2bpp data\e[0m\n");
  tau->sd2bppP = stripNew(tau->raw2bppA, 3, 2, 0, verbose);

  tau->raw4bppA = arrayNew(  sizeof(rawData4bpp[0]), sizeof(rawData4bpp) / sizeof(rawData4bpp[0]), GENERAL);
  memcpy((void*) tau->raw4bppA, (void*) rawData4bpp, sizeof(rawData4bpp[0]) * sizeof(rawData4bpp) / sizeof(rawData4bpp[0]));
  if (verbose) printf("\e[91m4bpp data\e[0m\n");
  tau->sd4bppP = stripNew(tau->raw4bppA, 9, 4, 0, verbose);
}

TEST_F_TEARDOWN(Tau) {
  memRst( GENERAL );
}


TEST_F(Tau, stripClr) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, sdUnpack) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, sdAssemble) {
  REQUIRE_TRUE(1);
}

TEST_F(Tau, bleh) {
  stripIni( blehImg.cmP->sdP, GENERAL );
}

TEST_F(Tau, stripIni) {
  stripIni(tau->sd1bppP, GENERAL);
  stripIni(tau->sd2bppP, GENERAL);
  stripIni(tau->sd4bppP, GENERAL);
}

TEST_F(Tau, red) {
  stripIni( redImg.cmP->sdP, GENERAL );
}

TEST_F(Tau, heck) {
  stripIni( heckImg.cmP->sdP, GENERAL );
}

TEST_F(Tau, heck_with_offset) {
  heckImg.cmP->sdP->ss.offset = 55;
  stripIni( heckImg.cmP->sdP, GENERAL );
}

TEST_F(Tau, stripIni_4bpp_expectOnlyInflation) {
  CHECK_EQ( tau->sd4bppP->flags, ( SD_SKIP_ASSEMBLY_ | SD_SKIP_UNPACKING_ ) );
  // we store packed data in infP->compressedDataA even if we don't inflate it later.
  // Because we need it there for the cases where we DO inflate it.
  REQUIRE_TRUE(tau->sd4bppP != NULL);
  REQUIRE_TRUE(tau->sd4bppP->ss.infP != NULL);
  stripIni(tau->sd4bppP, GENERAL);
  // Strip set should be non-empty, since that's where its data goes.
  CHECK_TRUE(tau->sd4bppP->ss.infP->inflatedDataP != NULL);
  // Shouldn't be unpacked or assembled.
  CHECK_TRUE(tau->sd4bppP->ss.unpackedDataA == NULL);
  CHECK_TRUE(tau->sd4bppP->sm.infP == NULL);
  CHECK_TRUE(tau->sd4bppP->assembledDataA == NULL);
}
