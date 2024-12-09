#include "test.h"

// Let's test on real images too.
#include "blehColormap.h"
#include "heckColormap.h"
#include "redColormap.h"

TAU_MAIN()

// In addition to the made-up arrays below,
//   load in the images we made for the SDL surface unit test.

TEST_F_SETUP(Tau) {
  // 4bpp
  tau->raw4bppA = arrayNew(  sizeof(rawData4bpp[0]), sizeof(rawData4bpp) / sizeof(rawData4bpp[0]));
  REQUIRE_TRUE(tau->raw4bppA != NULL);
  REQUIRE_EQ(arrayGetElemSz((void*) tau->raw4bppA), sizeof(rawData4bpp[0]));
  //REQUIRE_EQ(arrayGetNElems((void*) tau->raw4bppA), sizeof(rawData4bpp) / sizeof(rawData4bpp[0]));
  memcpy((void*) tau->raw4bppA, (void*) rawData4bpp, sizeof(rawData4bpp[0]) * sizeof(rawData4bpp) / sizeof(rawData4bpp[0]));

  tau->sd4bppP = stripNew(tau->raw4bppA, 5, 4, 0, 0);
  //stripClr(tau->sd4bppP);
}

TEST_F_TEARDOWN(Tau) {
  arrayDel((void**) &tau->raw4bppA);
  stripDel(&tau->sd4bppP);
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

TEST_F(Tau, stripIni) {
  stripIni(tau->sd4bppP);
  stripClr(tau->sd4bppP);
}

TEST_F(Tau, bleh) {
  stripIni( blehColormap.sdP );
  stripClr( blehColormap.sdP );
}

TEST_F(Tau, red) {
  stripIni( redColormap.sdP );
  stripClr( redColormap.sdP );
}

TEST_F(Tau, heck) {
  stripIni( heckColormap.sdP );
  stripClr( heckColormap.sdP );
}

TEST_F(Tau, heck_with_offset) {
  heckColormap.sdP->ss.offset = 55;
  stripIni( heckColormap.sdP );
  stripClr( heckColormap.sdP );
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
  CHECK_TRUE(tau->sd4bppP->ss.unpackedDataP == NULL);
  CHECK_TRUE(tau->sd4bppP->sm.infP == NULL);
  CHECK_TRUE(tau->sd4bppP->assembledDataA == NULL);
}
