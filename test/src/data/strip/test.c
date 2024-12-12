#include "test.h"

// Let's test on real images too.
#include "blehColormap.h"
#include "heckColormap.h"
#include "redColormap.h"

TAU_MAIN()

// In addition to the made-up arrays below,
//   load in the images we made for the SDL surface unit test.

const static U8 verbose = 0;

TEST_F_SETUP(Tau) {
  // Make arrays, because inflatableNew() expects Jollybean arrays.
  tau->raw1bppA = arrayNew(  sizeof(rawData1bpp[0]), sizeof(rawData1bpp) / sizeof(rawData1bpp[0]));
  memcpy((void*) tau->raw1bppA, (void*) rawData1bpp, sizeof(rawData1bpp[0]) * sizeof(rawData1bpp) / sizeof(rawData1bpp[0]));
  if (verbose) printf("\e[91m1bpp data\e[0m\n");
  tau->sd1bppP = stripNew(tau->raw1bppA, 3, 1, 0, verbose);

  tau->raw2bppA = arrayNew(  sizeof(rawData2bpp[0]), sizeof(rawData2bpp) / sizeof(rawData2bpp[0]));
  memcpy((void*) tau->raw2bppA, (void*) rawData2bpp, sizeof(rawData2bpp[0]) * sizeof(rawData2bpp) / sizeof(rawData2bpp[0]));
  if (verbose) printf("\e[91m2bpp data\e[0m\n");
  tau->sd2bppP = stripNew(tau->raw2bppA, 3, 2, 0, verbose);

  tau->raw4bppA = arrayNew(  sizeof(rawData4bpp[0]), sizeof(rawData4bpp) / sizeof(rawData4bpp[0]));
  memcpy((void*) tau->raw4bppA, (void*) rawData4bpp, sizeof(rawData4bpp[0]) * sizeof(rawData4bpp) / sizeof(rawData4bpp[0]));
  if (verbose) printf("\e[91m4bpp data\e[0m\n");
  tau->sd4bppP = stripNew(tau->raw4bppA, 9, 4, 0, verbose);
  
  stripClr(tau->sd1bppP);
  stripClr(tau->sd2bppP);
  stripClr(tau->sd4bppP);
}

TEST_F_TEARDOWN(Tau) {
  stripDel(&tau->sd1bppP);
  arrayDel((void**) &tau->raw1bppA);

  stripDel(&tau->sd2bppP);
  arrayDel((void**) &tau->raw2bppA);

  stripDel(&tau->sd4bppP);
  arrayDel((void**) &tau->raw4bppA);

  stripClr( blehColormap.sdP );
  stripClr( redColormap.sdP );
  stripClr( heckColormap.sdP );
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
  stripIni( blehColormap.sdP );
}

TEST_F(Tau, stripIni) {
  stripIni(tau->sd1bppP);
  stripIni(tau->sd2bppP);
  stripIni(tau->sd4bppP);
}

TEST_F(Tau, red) {
  stripIni( redColormap.sdP );
}

TEST_F(Tau, heck) {
  stripIni( heckColormap.sdP );
}

TEST_F(Tau, heck_with_offset) {
  heckColormap.sdP->ss.offset = 55;
  stripIni( heckColormap.sdP );
}

TEST_F(Tau, stripIni_4bpp_expectOnlyInflation) {
  CHECK_EQ( tau->sd4bppP->flags, ( SD_SKIP_ASSEMBLY_ | SD_SKIP_UNPACKING_ ) );
  // we store packed data in infP->compressedDataA even if we don't inflate it later.
  // Because we need it there for the cases where we DO inflate it.
  REQUIRE_TRUE(tau->sd4bppP != NULL);
  REQUIRE_TRUE(tau->sd4bppP->ss.infP != NULL);
  REQUIRE_TRUE(tau->sd4bppP->ss.infP->inflatedDataP == NULL);
  stripIni(tau->sd4bppP);
  // Strip set should be non-empty, since that's where its data goes.
  CHECK_TRUE(tau->sd4bppP->ss.infP->inflatedDataP != NULL);
  // Shouldn't be unpacked or assembled.
  CHECK_TRUE(tau->sd4bppP->ss.unpackedDataA == NULL);
  CHECK_TRUE(tau->sd4bppP->sm.infP == NULL);
  CHECK_TRUE(tau->sd4bppP->assembledDataA == NULL);
}
