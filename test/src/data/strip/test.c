#include "tau/tau.h"
#include "strip.h"
#include "sd.h"

// TODO rename strip util to not clash
// TODO include strip util to pack stuff to make this easier and portable

TAU_MAIN()

typedef struct Tau {
  U8 *raw1bppA;
  U8 *raw2bppA;
  U8 *raw4bppA;
  StripDataS *sd1bppP;
  StripDataS *sd2bppP;
  StripDataS *sd4bppP;
} Tau;

TEST_F_SETUP(Tau) {
  U8 rawData1bpp[] = {1, 0, 0, 1, 0, 0, 0, 0, 1};
  U8 rawData2bpp[] = {1, 2, 0, 3, 0, 2, 3, 2, 1};
  U8 rawData4bpp[] = {
    1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
    1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
    1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
    1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
    1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
    1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
    1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
    1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
    1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
    1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
    1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
    1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
    1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
    1, 15, 12, 2, 0, 10, 8, 9, 1, 0,
    1, 15, 12, 2, 0, 10, 8, 9, 1, 0};
  // 1bpp
  tau->raw1bppA = arrayNew(  sizeof(rawData1bpp[0]), sizeof(rawData1bpp) / sizeof(rawData1bpp[0]));
  REQUIRE_TRUE(tau->raw1bppA != NULL);
  REQUIRE_EQ(arrayGetElemSz((void*) tau->raw1bppA), (U32) sizeof(rawData1bpp[0]));
  REQUIRE_EQ(arrayGetNElems((void*) tau->raw1bppA), sizeof(rawData1bpp) / sizeof(rawData1bpp[0]));
  memcpy((void*) tau->raw1bppA, (void*) rawData1bpp, sizeof(rawData1bpp[0]) * sizeof(rawData1bpp) / sizeof(rawData1bpp[0]));
  // 2bpp
  tau->raw2bppA = arrayNew(  sizeof(rawData2bpp[0]), sizeof(rawData2bpp) / sizeof(rawData2bpp[0]));
  REQUIRE_TRUE(tau->raw2bppA != NULL);
  REQUIRE_EQ(arrayGetElemSz((void*) tau->raw2bppA), sizeof(rawData2bpp[0]));
  REQUIRE_EQ(arrayGetNElems((void*) tau->raw2bppA), sizeof(rawData2bpp) / sizeof(rawData2bpp[0]));
  memcpy((void*) tau->raw2bppA, (void*) rawData2bpp, sizeof(rawData2bpp[0]) * sizeof(rawData2bpp) / sizeof(rawData2bpp[0]));
  // 4bpp
  tau->raw4bppA = arrayNew(  sizeof(rawData4bpp[0]), sizeof(rawData4bpp) / sizeof(rawData4bpp[0]));
  REQUIRE_TRUE(tau->raw4bppA != NULL);
  REQUIRE_EQ(arrayGetElemSz((void*) tau->raw4bppA), sizeof(rawData4bpp[0]));
  //REQUIRE_EQ(arrayGetNElems((void*) tau->raw4bppA), sizeof(rawData4bpp) / sizeof(rawData4bpp[0]));
  memcpy((void*) tau->raw4bppA, (void*) rawData4bpp, sizeof(rawData4bpp[0]) * sizeof(rawData4bpp) / sizeof(rawData4bpp[0]));

  // Make strip data with random units per strips
  tau->sd1bppP = stripNew(tau->raw1bppA, 3, 1, 0, 0);
  tau->sd2bppP = stripNew(tau->raw2bppA, 3, 2, 0, 0);
  tau->sd4bppP = stripNew(tau->raw4bppA, 5, 4, 0, 0);
  stripClr(tau->sd1bppP);
  stripClr(tau->sd2bppP);
  stripClr(tau->sd4bppP);
}

TEST_F_TEARDOWN(Tau) {
  arrayDel((void**) &tau->raw1bppA);
  arrayDel((void**) &tau->raw2bppA);
  arrayDel((void**) &tau->raw4bppA);
  stripDel(&tau->sd1bppP);
  stripDel(&tau->sd2bppP);
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
  stripIni(tau->sd1bppP);
  stripIni(tau->sd2bppP);
  stripIni(tau->sd4bppP);
}

TEST_F(Tau, stripIni_SkipInflation) {
  tau->sd1bppP->flags |= SD_SKIP_INFLATION_;
  sdInflate(tau->sd1bppP);
  CHECK_TRUE(tau->sd1bppP->ss.infP->inflatedDataP == NULL);
  CHECK_TRUE(tau->sd1bppP->sm.infP->inflatedDataP == NULL);
}

TEST_F(Tau, stripIni_SkipUnpacking) {
  tau->sd1bppP->flags |= SD_SKIP_UNPACKING_;
  sdInflate(tau->sd1bppP);
  CHECK_TRUE(tau->sd1bppP->ss.infP->inflatedDataP == NULL);
  CHECK_TRUE(tau->sd1bppP->sm.infP->inflatedDataP == NULL);
  sdUnpack(tau->sd1bppP);
  CHECK_TRUE(tau->sd1bppP->ss.unpackedDataP == NULL);
  // Now make it unpack.
  tau->sd1bppP->flags &= ~SD_SKIP_UNPACKING_;
  sdUnpack(tau->sd1bppP);
  CHECK_TRUE(tau->sd1bppP->ss.unpackedDataP != NULL);
}

TEST_F(Tau, stripIni_SkipAssembly) {
  tau->sd1bppP->flags |= SD_SKIP_ASSEMBLY_;
  sdInflate(tau->sd1bppP);
  CHECK_TRUE(tau->sd1bppP->ss.infP->inflatedDataP == NULL);
  CHECK_TRUE(tau->sd1bppP->sm.infP->inflatedDataP == NULL);
  sdUnpack(tau->sd1bppP);
  CHECK_TRUE(tau->sd1bppP->ss.unpackedDataP != NULL);
  sdAssemble(tau->sd1bppP);
  CHECK_TRUE(tau->sd1bppP->assembledDataA == NULL);
}

TEST_F(Tau, stripIni_4bpp_expectOnlyUnzipping) {
  CHECK_TRUE(tau->sd4bppP->ss.infP->inflatedDataP == NULL);
  stripIni(tau->sd4bppP);
  // Strip set should be non-empty, since that's where its data goes.
  CHECK_TRUE(tau->sd4bppP->ss.infP->inflatedDataP != NULL);
  // Usual stripset and stripmap stuff should be empty.
  CHECK_TRUE(tau->sd4bppP->ss.unpackedDataP == NULL);
  CHECK_TRUE(tau->sd4bppP->sm.infP == NULL);
  // Strip data
  CHECK_TRUE(tau->sd4bppP->assembledDataA == NULL);
}
