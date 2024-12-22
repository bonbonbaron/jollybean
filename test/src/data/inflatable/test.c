#include "tau.h"
#include "inf.h"
// #include "inflatable.h"
#include "array.h"

/*
functions to test:
==================
  inflatableIni
  inflatableClr
==================
*/

TAU_MAIN()

TEST(InflatableSuite, OnlyTest) {
  Inflatable *infP = NULL;

  // Make dummy data.
  U32 *dummyDataA = NULL;
  static const U32 MAX_N_ELEMS = 1000;
  for (U32 j = 1; j < MAX_N_ELEMS; ++j) {
    //printf("%d\n", j);
    dummyDataA = arrayNew(sizeof(U32), j);
    CHECK_EQ(arrayGetNElems(dummyDataA), j);
    CHECK_EQ(arrayGetElemSz(dummyDataA), sizeof(U32));

    for (U32 i = 0; i < j; ++i) {
      dummyDataA[i] = i;
    }

    // Create inflatable.
    CHECK_NOT_NULL(dummyDataA);
    infP = inflatableNew((void*) dummyDataA);  
    REQUIRE_TRUE(infP != NULL);
    REQUIRE_TRUE(infP->inflatedDataP == NULL);

    // Delete array inflatable comes from.
    arrayDel((void**) &dummyDataA);
    
    // Inflate inflatable.
    inflatableIni(infP);
    REQUIRE_TRUE(infP->inflatedDataP != NULL);

    // Verify information is still correct.
    REQUIRE_EQ(infP->inflatedLen, j * sizeof(U32)); 
    for (U32 i = 0; i < j; ++i) {
      CHECK_TRUE(((U32*) infP->inflatedDataP)[i] == i);
    }

    // Clear inflatable.
    inflatableClr(infP);
    CHECK_NULL(infP->inflatedDataP);

    // Delete inflatable.
    inflatableDel(&infP);
    CHECK_NULL(infP);
  }
}
